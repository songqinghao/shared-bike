#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "sqlconnection.h"
#include "user_service.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


UserEventHandler::UserEventHandler():iEventHandler("UserEventHandler")
{
	//将当前作为指针进行传递作为handler，后面会调用该类下的handle方法
	DispatchMsgService::getInstance()->subscribe(EEVENTID_GET_MOBILE_CODE_REQ,this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_LOGIN_REQ,this);
	//未来需要实现订阅事件的处理
	//初始化pm
	thread_mutex_create(&pm_);
}

UserEventHandler::~UserEventHandler()
{
	//实现退订事件hangdle的处理
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_GET_MOBILE_CODE_REQ,this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LOGIN_REQ,this);
	
	thread_mutex_destroy(&pm_);
}

iEvent* UserEventHandler::handle(const iEvent* ev)
{
	if (ev == NULL)
	{
		//LOG_ERROR("input ev is NULL");
		printf("input ev is NULL");
	}

	u32 eid = ev->get_eid();


	if (eid == EEVENTID_GET_MOBILE_CODE_REQ)
	{
		//返回手机验证码响应
		return handle_mobile_code_req((MobileCodeReqEv*)ev);
	}
	//登录请求事件来了
	else if (eid == EEVENTID_LOGIN_REQ)
	{
		return handle_login_req((LoginReqEv*) ev);
	}
	else if (eid == EEVENTID_RECHARGE_REQ)
	{
		//return handle_recharge_req((RechargeEv*) ev);
	}
	else if (eid == EEVENTID_GET_ACCOUNT_BALANCE_REQ)
	{
		//return handle_get_account_balance_req((GetAccountBalanceEv*) ev);
	}
	else if (eid == EEVENTID_LIST_ACCOUNT_RECORDS_REQ)
	{
		//return handle_list_account_records_req((ListAccountRecordsReqEv*) ev);
	}

	return NULL;
}

MobileCodeRspEv* UserEventHandler::handle_mobile_code_req(MobileCodeReqEv* ev)
{
	i32  icode = 0;
	std::string mobile_ = ev->get_mobile();
	//LOG_DEBUG("try to get moblie phone %s validate code .", mobile_.c_str());
	printf("try to get moblie phone %s validate code .\n", mobile_.c_str());
    //icode为验证码
	icode = code_gen();
	//修改之前进行上锁
    thread_mutex_lock(&pm_);
	m2c_[mobile_] = icode;
	//修改后进行解锁
	thread_mutex_unlock(&pm_);
	printf("mobile: %s, code: %d\n", mobile_.c_str(), icode);
	
	//返回验证码响应事件，以刚刚随机产生的验证码进行初始化
	return new MobileCodeRspEv(200, icode);
}

//产生验证码
i32 UserEventHandler::code_gen()
{
	i32 code = 0;
	srand((unsigned int)time(NULL));

	code = (unsigned int)(rand() % (999999 - 100000) + 100000);

	return code;
}

LoginResEv* UserEventHandler::handle_login_req(LoginReqEv* ev)
{
	LoginResEv* loginResEv = nullptr;
	//验证手机验证码是否正确
	std::string mobile = ev->get_mobile();//获取手机号
	i32 code = ev->get_icode();//获取验证码
	thread_mutex_lock(&pm_);
	auto iter = m2c_.find(mobile);
	//没找到手机号，或者验证码错误
	if (((iter != m2c_.end()) && (code != iter->second))
		|| (iter == m2c_.end()))
	{
		//printf("UserEventHandler::handle_login_req：不存在手机号！\n");
		loginResEv = new LoginResEv(ERRC_INVALID_DATA);
	}
	
	thread_mutex_unlock(&pm_);
	if(loginResEv) return loginResEv;
	
	//如果验证成功，则判断数据库中是否存在该手机号，如果不存在那么就插入
	//重新创建一个数据库连接对象
	std::shared_ptr<MysqlConnection>mysqlconn(new MysqlConnection);
	st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	    if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port,
		conf_args.db_user.c_str(), conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req Database init failed. exit!\n");
		//返回处理错误事件
		return new LoginResEv(ERRO_PROCCESS_FAILED);	
	}

	//创建用户服务类
	UserService userservice(mysqlconn);
	bool result = false;
	//如果不存在此手机号
	if(!userservice.exist(mobile))
	{
		result = userservice.insert(mobile);
		//插入失败
		if (!result)
		{
			LOG_ERROR("insert user(%s) to db failed.", mobile.c_str());
			return new LoginResEv(ERRO_PROCCESS_FAILED);
		}
	}
	return new LoginResEv(ERRC_SUCCESS);

}
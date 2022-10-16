#include "user_event_handler.h"
#include "DispatchMsgService.h"
// #include "sqlconnection.h"
// #include "iniconfig.h"
// #include "user_service.h"

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
	else if (eid == EEVENTID_LOGIN_REQ)
	{
		//return handle_login_req((LoginReqEv*) ev);
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



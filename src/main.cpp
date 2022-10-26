#include <iostream>
#include <unistd.h>
#include <memory>
#include "bike.pb.h"
#include "ievent.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include "iniconfig.h"
#include "Logger.h"
#include "sqlconnection.h"
#include "BusProcessor.h"

using namespace std;

int main(int argc,char** argv)
{
    if (argc != 3) {
		printf("Please input shbk <conifg file path> <log file config>!\n");
		return -1;
	}

    if (!Logger::instance()->init(std::string(argv[2])))
	{
		fprintf(stderr, "init log module failed.\n");
		return -2;
	}

    //创建单例
	Iniconfig *config = Iniconfig::getInstance();
	if (!config->loadfile(std::string(argv[1])))
	{
		LOG_ERROR("load %s failed.", argv[1]);
		Logger::instance()->GetHandle()->error("load %s failed.", argv[1]);
		return -3;
	}

    st_env_config conf_args = config->getconfig();
	LOG_INFO("[database] ip: %s port：%d user: %s  pwd: %s db: %s  [server] port: %d\n", conf_args.db_ip.c_str(), conf_args.db_port, \
		conf_args.db_user.c_str(), conf_args.db_pwd.c_str(), conf_args.db_name.c_str(), conf_args.svr_port);
        
    std::shared_ptr<MysqlConnection>mysqlconn(new MysqlConnection);
    //数据数据库初始化不成功
    if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port,
		conf_args.db_user.c_str(), conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("Database init failed. exit!\n");
		return -4;
	}
	//这里也会初始化用户处理事件对象（UserEventHandler）
    BusinessProcessor busPro(mysqlconn);
    //初始化表，看存不存在，不存在那么就创建
    busPro.init();


    DispatchMsgService*DMS = DispatchMsgService::getInstance();
    DMS->open();//单例模式open

    NetworkInterface* NTIF = new NetworkInterface();
    //设置端口号
    NTIF->start(conf_args.svr_port);
    while(true)
    {
        NTIF->network_dispatch();
        //1s钟执行一次
        sleep(1);
        LOG_DEBUG("network_event_dispatch ...\n");
    }
    //创建一个请求事件
    /*
    MobileCodeReqEv*pmcre = new MobileCodeReqEv("13559851513");
    //，投递进去
    DMS->enqueue(pmcre);
    */
    
    sleep(5);
    DMS->close();
    sleep(5);
    return 0;  
}




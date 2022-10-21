#ifndef BIKE_SHEARED_BUSPROCESSOR_H
#define BIKE_SHEARED_BUSPROCESSOR_H
#include "user_event_handler.h"
#include "sqlconnection.h"
//该类用于协调事务
class BusinessProcessor
{
public:
	BusinessProcessor(std::shared_ptr<MysqlConnection> conn);
	bool init();
	virtual ~BusinessProcessor();

private:
	std::shared_ptr<MysqlConnection> mysqlconn_;
	std::shared_ptr<UserEventHandler> ueh_;  //账户管理系统
};


#endif
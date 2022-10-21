#ifndef BIKE_SHREAD_USER_SERVICE_H
#define BIKE_SHREAD_USER_SERVICE_H
#include "glo_def.h"
#include "sqlconnection.h"
#include "events_def.h"

#include <memory>

//用户服务模块
class UserService
{
public:
    UserService(std::shared_ptr<MysqlConnection> sql_conn);
    //判断手机存不存在
    bool exist(const std::string& mobile);
    //插入手机号
    bool insert(const std::string& mobile);

private:
    std::shared_ptr<MysqlConnection> sql_conn_;
};

#endif
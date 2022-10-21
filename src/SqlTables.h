#ifndef BIKE_SHARE_SQLTABLES_H
#define BIKE_SHARE_SQLTABLES_H
#include <memory>
#include"sqlconnection.h"
#include"Logger.h"
#include"glo_def.h"
class SqlTables
{
    public:
        SqlTables(std::shared_ptr<MysqlConnection>sqlConn):sqlconn_(sqlConn)
        {
        }

        //创建用户表
        bool CreateUserInfo()
        {
            const char* pUserInfoTable = " \
                                    CREATE TABLE IF NOT EXISTS userinfo( \
                                    id			  int(16)		     NOT NULL primary key auto_increment, \
                                    mobile		  varchar(16)		 NOT NULL default '1300000000', \
                                    username     varchar(128)	     NOT NULL default '', \
                                    verify		  int(4)             NOT NULL default '0', \
                                    registertm	  timestamp          NOT NULL default CURRENT_TIMESTAMP, \
                                    money        int(4)             NOT NULL default 0, \
                                    INDEX        mobile_index(mobile) \
                                    )";


            if (!sqlconn_->Execute(pUserInfoTable))
            {
                LOG_ERROR("create table userinfoTable table failed. error msg:%s", sqlconn_->GetErrInfo());
                return false;
            }
            LOG_ERROR("create table userinfo success!!!");
            return true;
        }

        //创建自行车表
        bool CreateBikeTable()
        {
            const char* pBikeInfoTable = "\
                            CREATE TABLE IF NOT EXISTS bikeinfo( \
                            id		   int               NOT NULL primary key auto_increment, \
                            devno        int               NOT NULL, \
                            status       tinyint(1)        NOT NULL default 0, \
                            trouble      int               NOT NULL default 0, \
                            tmsg         varchar(256)      NOT NULL default '',\
                            latitude     double(10,6)      NOT NULL default 0, \
                            longitude    double(10,6)      NOT NULL default 0, \
                            unique(devno)  \
                            )";

            if (!sqlconn_->Execute(pBikeInfoTable))
            {
                LOG_ERROR("create table bikeinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
                return false;
            }
            
            LOG_ERROR("create table bikeTable success!!!");
            return true;

        }

    private:
        //创建共享指针
        std::shared_ptr<MysqlConnection>sqlconn_;

};


#endif
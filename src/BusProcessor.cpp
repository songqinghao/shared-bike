#include "BusProcessor.h"
#include "SqlTables.h"

BusinessProcessor::BusinessProcessor(std::shared_ptr<MysqlConnection> conn)
    :mysqlconn_(conn),ueh_(new UserEventHandler())
{

}

bool BusinessProcessor::init()
{
    SqlTables tables(mysqlconn_);
    tables.CreateUserInfo();
    tables.CreateBikeTable();
    
    return true;
}
BusinessProcessor::~BusinessProcessor()
{
    //释放共享指针
    ueh_.reset();
    
}
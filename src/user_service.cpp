#include "user_service.h"

UserService::UserService(std::shared_ptr<MysqlConnection> sql_conn) : sql_conn_(sql_conn)
{

}

bool UserService::exist(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
    //查询语句
	sprintf(sql_content, \
		"select * from userinfo where mobile = %s", \
		mobile.c_str());
    //创建数据库结果集
	SqlRecordSet record_set;
    //查询数据
	if (!sql_conn_->Execute(sql_content, record_set))
	{
        //如果查询失败
		return false;
	}

    //如果查询成功，那么就返回记录行数是不是为0
	return (record_set.GetRowCount() != 0);

}

//插入手机号码操作
bool UserService::insert(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, \
		"insert into userinfo (mobile) values (%s)", \
		mobile.c_str());
	return sql_conn_->Execute(sql_content);
}
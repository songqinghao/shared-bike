#include "sqlconnection.h"
#include <string.h>

MysqlConnection::MysqlConnection() : mysql_(NULL)
{
    mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
}

MysqlConnection::~MysqlConnection()
{
    if (mysql_ != NULL)
    {
        mysql_close(mysql_);
        free(mysql_);
        mysql_ = NULL;
    }
    return;
}

bool MysqlConnection::Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb)
{
    LOG_INFO("enter Init \n");
    //初始化mysql
    if ((mysql_ = mysql_init(mysql_)) == NULL)
    {
        LOG_ERROR("init mysql failed %s, %d", this->GetErrInfo(), errno);
        return false;
    }

    //设置选项，如果断开连接以后设置为自动重新连接
    char cAuto = 1;
    if (mysql_options(mysql_, MYSQL_OPT_RECONNECT, &cAuto)!=0)//成功返回0
    {
        LOG_ERROR("mysql_options MYSQL_OPT_RECONNECT failed.reason: %s\n", GetErrInfo());
    }

    //进行连接
    if (mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb, nPort, NULL, 0) == NULL)
    {
        LOG_ERROR("connect mysql failed %s", this->GetErrInfo());
        return false;
    }

    //应该移到mysql_real_connect之前

    return true;
}

bool MysqlConnection::Execute(const char* szSql)
{
    if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
    {
        if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
        {
            reconnect();
        }
        return false;
    }
    return true;
}
int MysqlConnection::EscapeString(const char* pSrc, int nSrcLen, char* pDest)
{
    if (!mysql_)
    {
        return 0;
    }
    return mysql_real_escape_string(mysql_, pDest, pSrc, nSrcLen);
}




//获取错误信息
const char* MysqlConnection::GetErrInfo()
{
    return mysql_error(mysql_);
}

//重新连接数据库
void MysqlConnection::reconnect()
{
    mysql_ping(mysql_);
}

//执行mysql操作并且返回结果集
bool MysqlConnection::Execute(const char* szSql, SqlRecordSet& recordSet)
{
    //进行操作
    if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
    {
        //如果发生错误==连接中断
        if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
        {
            reconnect();
        }

        return false;
    }
    //mysql_store_result获取结果集
    MYSQL_RES* pRes = mysql_store_result(mysql_);
    if (!pRes)
    {
        return NULL;
    }
    //设置结果集
    recordSet.SetResult(pRes);

    return true;
}

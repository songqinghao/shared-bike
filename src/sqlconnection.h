#ifndef SQL_CONNECTION_H
#define SQL_CONNECTION_H

#include <mysql/mysql.h>
#include <string>
#include <mysql/errmsg.h>
#include <assert.h>
#include "glo_def.h"
class SqlRecordSet
{
    public:
        SqlRecordSet():m_pRes(NULL)
        {

        }

        explicit SqlRecordSet(MYSQL_RES* pRes)
	    {
		    m_pRes = pRes;
	    }

        //取结果集
        MYSQL_RES* MysqlRes()
        {
            return m_pRes;
        }

        //析构
        ~SqlRecordSet()
        {
            if(m_pRes)
            {
                mysql_free_result(m_pRes);
            }
        }

        inline void SetResult(MYSQL_RES* pRes)
        {
            //如果此时已经保存了结果集了，那么应该让程序报错，防止内存泄露
            assert(m_pRes == NULL);		
            if (m_pRes)
            {
                LOG_WARN("the MYSQL_RES has already stored result, maybe will cause memory leak");
            }
            m_pRes = pRes;
        }

        inline MYSQL_RES* GetResult()
        {
            return m_pRes;
        }

        //获取结果集的行
        void FetchRow(MYSQL_ROW& row)
        {
            row = mysql_fetch_row(m_pRes);
        }

        //获取结果集的行数
        inline i32 GetRowCount()
        {
            return m_pRes->row_count;
        }
    private:
        MYSQL_RES* m_pRes;
};



class MysqlConnection
{
    public:
        MysqlConnection();
        ~MysqlConnection();
        //获取mysql句柄
        MYSQL* Mysql()
        {
            return mysql_;
        }
        //对数据库的链接进行初始化
        bool Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);
        //不需要返回值的操作数据库
        bool Execute(const char* szSql);
        //需要拿到结果的操作数据库
        //SqlRecordSet就是用于存储结果集
        bool Execute(const char* szSql, SqlRecordSet& recordSet);
        //获取错误信息
        const char* GetErrInfo();
        //重新连接
        void reconnect();
        //将特殊字符进行转义,pSrc为转义前的字符串，而pDest为转义后的字符串
        int EscapeString(const char* pSrc, int nSrcLen, char* pDest);
    private:
        //准备一个mysql句柄
        MYSQL* mysql_;
};

#endif
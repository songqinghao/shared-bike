#ifndef BRKS_COMMON_EVENT_TYPE_H_
#define BRKS_COMMON_EVENT_TYPE_H_
#include "glo_def.h"
typedef struct EEorReason
{
  i32 code;
  const char* reason;
}EErrorReason;

/*事件ID，后续用于区分*/
enum Event
{
  //获取手机验证码请求
  EEVENTID_GET_MOBILE_CODE_REQ  = 0X01,
  //验证码响应
  EEVENTID_GET_MOBILE_CODE_RSP  = 0X02,
  
  //登录请求
  EEVENTID_LOGIN_REQ            = 0X03,
  //登录响应
  EEVENTID_LOGIN_RSP            = 0X04,
  
  //充值请求
  EEVENTID_RECHARGE_REQ         = 0X05,
  //充值响应
  EEVENTID_RECHARGE_RSP         = 0X06,
  
  //余额查询请求
  EEVENTID_GET_ACCOUNT_BALANCE_REQ = 0X07,
  //余额查询响应
  EEVENTID_GET_ACCOUNT_BALANCE_RSP = 0X08,
  
  //查询充值记录请求
  EEVENTID_LIST_ACCOUNT_RECORDS_REQ= 0X09,
  //查明充值记录响应
  EEVENTID_ACCOUNT_RECORDS_RSP     = 0X10,
  
  //查询骑行记录请求
  EEVENTID_LIST_TRAVELS_REQ        = 0X11,
  //查询骑行记录响应
  EEVENTID_LIST_TRAVELS_RSP        = 0X12,
  //空的响应
  EEVENTID_EXIT_RSP = 0xFE,
  EEVENTID_UNKNOWN = 0xFF

};

//事件响应错误代号
enum EErrorCode
{
    //成功响应
    ERRC_SUCCESS = 200,
    //无效的消息
    ERRC_INVALID_MSG = 400,
    //无效的数据
    ERRC_INVALID_DATA = 404,
    //方法不允许
    ERRC_METHOD_NOT_ALLOWED = 405,
    ERRO_PROCCESS_FAILED = 406,
    ERRO_BIKE_IS_TOOK = 407,
    ERRO_BIKE_IS_RUNNING = 408,
    ERRO_BIKE_IS_DAMAGED = 409,
    ERR_NULL             = 0
};

//将code转成现实意义（查询）：200——OK,400——失败
const char* getReasonByErrorCode(i32 code);

#endif
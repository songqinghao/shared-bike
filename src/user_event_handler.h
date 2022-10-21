#ifndef BRKS_BUS_USERM_HANDLER_H_
#define BRKS_BUS_USERM_HANDLER_H_

#include "glo_def.h"
#include "iEventHandler.h"
#include "events_def.h"
#include "thread_pool.h"
#include "thread.h"
//具体用户处理事件的类，继承自事件处理类
class UserEventHandler : public iEventHandler
{
public:
    UserEventHandler();
    virtual ~UserEventHandler();
    //应用成所设置的事件都应该由这里处理
    virtual iEvent* handle(const iEvent* ev);

private:
    //处理手机验证码的请求，返回的是手机验证码请求事件
    MobileCodeRspEv* handle_mobile_code_req(MobileCodeReqEv* ev);
    LoginResEv* handle_login_req(LoginReqEv* ev);
    
    //产生验证码方法
    i32 code_gen();

private:
    //std::string mobile_;
    std::map<std::string, i32> m2c_; //保存手机号码对应的验证码（手机号码+验证码）
    pthread_mutex_t pm_;
};




#endif





#ifndef BRKS_BUS_USERM_HANDLER_H_
#define BRKS_BUS_USERM_HANDLER_H_

#include "glo_def.h"
#include "iEventHandler.h"
#include "events_def.h"


//具体用户处理事件的类，继承自时间处理类
class UserEventHandler : public iEventHandler
{
public:
    UserEventHandler();
    virtual ~UserEventHandler();
    virtual iEvent* handle(const iEvent* ev);

private:
    //处理手机验证码的请求
    MobileCodeRspEv* handle_mobile_code_req(MobileCodeReqEv* ev);
    //LoginResEv* handle_login_req(LoginReqEv* ev);
    
    //产生验证码
    i32 code_gen();

private:
    //std::string mobile_;
    std::map<std::string, i32> m2c_; //保存手机号码对于的验证码（手机号码+验证码）
    pthread_mutex_t pm_;
};




#endif





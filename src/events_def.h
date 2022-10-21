#ifndef BRKS_COMMON_EVENT_DEF_H_
#define BRKS_COMMON_EVENT_DEF_H_

#include "ievent.h"
#include "bike.pb.h"
#include <string>
//手机验证码请求
class MobileCodeReqEv:public iEvent
{
    public:
        MobileCodeReqEv(const std::string&mobile):iEvent(EEVENTID_GET_MOBILE_CODE_REQ, iEvent::generateSeqNo())
        {
            msg_.set_mobile(mobile);
        };

        const std::string& get_mobile(){return msg_.mobile();};
        //打印信息
        virtual std::ostream& dump(std::ostream& out) const;
        virtual i32 ByteSize() { return msg_.ByteSize(); };
        virtual bool SerializeToArray(char* buf,int len){ return msg_.SerializeToArray(buf,len);};
    private:
        tutorial::mobile_request msg_;


};

class MobileCodeRspEv : public iEvent
{
    public:
        MobileCodeRspEv(i32 code, i32 icode):iEvent(EEVENTID_GET_MOBILE_CODE_RSP, iEvent::generateSeqNo())
        {
            msg_.set_code(code);
            msg_.set_icode(icode);
            msg_.set_data(getReasonByErrorCode(code));
        }
        //返回响应代号
        const i32 get_code() { return msg_.code(); };
        //返回验证码
	    const i32 get_icode() { return msg_.icode(); };
        //返回代号表述
	    const std::string& get_data() { return msg_.data(); };

        //返回类的描述
        virtual std::ostream& dump(std::ostream& out) const;
	    //msg_ByteSize()就是msg序列化以后占的buffer空间
        virtual i32 ByteSize() { return msg_.ByteSize(); };
	    virtual bool SerializeToArray(char* buf,int len){ return msg_.SerializeToArray(buf,len);};

    private:
        tutorial::mobile_response msg_;  
};

class ExitRspEv:public iEvent
{
    public:
        ExitRspEv():iEvent(EEVENTID_EXIT_RSP,iEvent::generateSeqNo())
        {

        }
};

//登录请求
class LoginReqEv : public iEvent
{
public:
	LoginReqEv(const std::string& mobile, i32 icode) :
		iEvent(EEVENTID_LOGIN_REQ, iEvent::generateSeqNo()) {
		msg_.set_mobile(mobile);
		msg_.set_icode(icode);
	}

	const std::string& get_mobile() { return msg_.mobile(); };
	const i32 get_icode() { return msg_.icode(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 ByteSize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

private:
	tutorial::login_request msg_;
};

//登录响应
class LoginResEv : public iEvent
{
    public:
        LoginResEv(i32 code) :
            iEvent(EEVENTID_LOGIN_RSP, iEvent::generateSeqNo()) {
            msg_.set_code(code);
            msg_.set_desc(getReasonByErrorCode(code));
        }


        const i32 get_code() { return msg_.code(); };
        const std::string& get_desc() { return msg_.desc(); };

        virtual std::ostream& dump(std::ostream& out) const;
        virtual i32 ByteSize() { return msg_.ByteSize(); };
        virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

    private:
        tutorial::login_response msg_;
};
#endif
#ifndef BRKS_COMMON_EVENT_DEF_H_
#define BRKS_COMMON_EVENT_DEF_H_

#include "event.h"
#include "bike.pb.h"
#include <string>
class MobileCodeReqEv:public iEvent
{
    public:
        MobileCodeReqEv(const std::string&mobile):iEvent(EEVENTID_GET_MOBILE_CODE_REQ, iEvent::generateSeqNo())
        {
            msg_.set_mobile(mobile);
        };

        const std::string& get_mobile(){return msg_.mobile();};
        virtual std::ostream& dump(std::ostream& out) const;
        
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
	    //virtual i32 ByteSize() { return msg_.ByteSize(); };
	    //virtual bool SerializeToArray(char* buf, int len) { return 
    private:
        tutorial::mobile_response msg_;  
};


#endif
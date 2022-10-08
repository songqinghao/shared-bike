#ifndef NS_EVENT_H_
#define NS_EVENT_H_

#include "glo_def.h"
#include "eventtype.h"
#include <string>
#include <iostream>

class iEvent
{
    public:
        iEvent(u32 eid, u32 sn);
        virtual ~iEvent();
        virtual std::ostream& dump(std::ostream&out)const{return out;};
        u32 generateSeqNo();//产生事件号
        u32 get_eid() const {return eid_;};
        u32 get_sn() const {return sn_;};//获取序列号

        void set_eid(u32 eid){eid_=eid;};
    private:
        u32 eid_;
        u32 sn_;

};

#endif
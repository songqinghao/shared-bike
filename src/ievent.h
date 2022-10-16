#ifndef NS_EVENT_H_
#define NS_EVENT_H_

#include "glo_def.h"
#include "eventtype.h"
#include <string>
#include <iostream>

class iEvent
{
    public:
        /*为什么要有序列号：
        *当事件相同的时候，用序列号来区分
        */
        iEvent(u32 eid, u32 sn);
        virtual ~iEvent();
        //用于调试，输出一些信息
        virtual std::ostream& dump(std::ostream&out)const{return out;};
        //序列化到一个数组中
        virtual bool SerializeToArray(char* buf,int len){ return true;};
        virtual i32 ByteSize(){ return 0; };
        u32 generateSeqNo();    //产生序列号
        u32 get_eid() const {return eid_;};//获取事件ID
        u32 get_sn() const {return sn_;};//获取序列号

        void set_eid(u32 eid){eid_=eid;};

        void set_args(void* args){args_ = args;};
        void* get_args(){return args_;};
    private:
        u32 eid_;   //事件ID
        u32 sn_;    //事件的序列号
        void* args_;
}; 
#endif
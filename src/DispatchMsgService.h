/*********************************************
* 负责分发消息服务模块，其实就是把外部收到的消息，转化成内部事件，也就是data->msg->event的解码过程，
* 然后再把事件投递至线池的消息队列，由线程池调用其process 方法对事件进行处理，最终调用每个event的handler方法
* 来处理event，此时每个event handler需要subscribe该event后才会被调用到.
**********************************************/

#ifndef BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
#define BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_

#include <map>//用于保存事件和处理函数的关系（对应关系）
#include <vector>
#include <queue>
//#include "ievent.h"
#include "eventtype.h"
#include "iEventHandler.h"
#include "thread_pool.h"
#include "thread.h"
#include "ievent.h"
#include "bike.pb.h"
#include "glob.h"
#include "events_def.h"
#include "NetworkInterface.h"

class DispatchMsgService
{
    protected:
        //设置成protected是为了避免构造多个DispatchMsgService类
        DispatchMsgService();
    public:
        virtual ~DispatchMsgService();
        virtual BOOL open();//打开服务（线程池开启）
        virtual BOOL close();//关闭线程池
        //将事件和handler进行关联，后面也需要把关联关系给保存下来
        virtual void subscribe(u32 eid, iEventHandler* handler);
        //解除关联
        virtual void unsubscribe(u32 eid, iEventHandler* handler);
        //将事件投入到线程池中处理
        virtual i32 enqueue(iEvent*ev);

        //给线程池回调的函数（C回调C++的方法得用static）
        static void svc(void* argv);
        
        //对具体事件进行分发处理（将不同的事件方法给不一样的事件处理器）
        virtual iEvent* process(const iEvent*ev);

        //static保证只有一个
        static DispatchMsgService* getInstance();
        iEvent* parseEvent(const char* messages,u32 len, u32 eid); 

        // static void setNetworkInterface(NetworkInterface* nwif){NTIF_ = nwif;};
        
        //将队列中的所有响应元素进行发送加载
        void handleAllResponseEvent(NetworkInterface* interface);
    protected:
        thread_pool_t* tp;
        //单例
        static DispatchMsgService* DMS_;

        //事件和处理函数的关联
        typedef std::vector<iEventHandler*> T_EventHandlers;
        typedef std::map<u32, T_EventHandlers > T_EventHandlersMap;
        T_EventHandlersMap subscribers_;

        bool svr_exit_;//服务是否开启

        static std::queue<iEvent*>response_events;//存储event的响应队列
        static pthread_mutex_t queue_mutex;//用于锁队列的锁
   
};

#endif

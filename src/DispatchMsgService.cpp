#include "DispatchMsgService.h"
#include <algorithm>
//将全局静态成员置空
DispatchMsgService*DispatchMsgService::DMS_=nullptr;

DispatchMsgService::DispatchMsgService()
{
    //服务状态初始化
    svr_exit_ = false;
    //对线程池进行清空
    tp = NULL;
}

DispatchMsgService::~DispatchMsgService()
{

}

BOOL DispatchMsgService::open()
{
    
    tp = thread_pool_init();
    return tp ? true:false;
}

BOOL DispatchMsgService::close()
{
    svr_exit_ = false;
    thread_pool_destroy(tp);
    tp = NULL;
}

DispatchMsgService* DispatchMsgService::getInstance()
{
    if(DMS_ == nullptr)
    {
        DMS_=new DispatchMsgService();
    }
    return DMS_;
}

//事件回调方法
void DispatchMsgService::svc(void* argv)
{
    DispatchMsgService* dms = DispatchMsgService::getInstance();
    iEvent* ev = (iEvent*)argv;
    if (!dms->svr_exit_)
    {
        LOG_DEBUG("DispatchMsgService::svc ...\n");
        //iEvent* rsp = 
        dms->process(ev);
        //删除事件
        delete ev;

    }
}

i32 DispatchMsgService::enqueue(iEvent* ev)
{
    if (NULL == ev)
    {
        return -1;
    }

    //先分配一个任务
    thread_task_t* task = thread_task_alloc(0);

    //-------------------------------------------------------------------------
    //理解存在问题
    //理解清楚————2022-10-5
    task->handler = DispatchMsgService::svc;
    task->ctx = ev;//回调时带的参数
    //-------------------------------------------------------------------------

    //把任务投到线程池中
    return thread_task_post(tp, task);

    //return msg_queue_.enqueue(ev, 0);
}

//实现订阅
void DispatchMsgService::subscribe(u32 eid, iEventHandler* handler)
{
    LOG_DEBUG("DispatchMsgService::subscribe eid: %u\n", eid);
    //查找看看是不是已经进行过订阅了
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);
    //之前已经有注册过
    if (iter != subscribers_.end())
    {
        //看看订阅的方法是不是有handler存在
        T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
        if (hdl_iter == iter->second.end())
        {
            //如果不一样，则加到后面（一个事件可以由多个事件处理器进行处理）
            iter->second.push_back(handler);
        }
    }
    else
    {
        //如果没有绑定任何任务处理器
        subscribers_[eid].push_back(handler);
    }
}

//实现退订（就是订阅相反的过程）
void DispatchMsgService::unsubscribe(u32 eid, iEventHandler* handler)
{
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);
    if (iter != subscribers_.end())
    {
        T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
        if (hdl_iter != iter->second.end())
        {
            iter->second.erase(hdl_iter);
        }
    }
}

void DispatchMsgService::process(const iEvent* ev)
{
    LOG_DEBUG("DispatchMsgService::process -ev: %p\n", ev);
    if (NULL == ev)
    {
        return ;
    }
    
    u32 eid = ev->get_eid();

    LOG_DEBUG("DispatchMsgService::process - eid: %u\n", eid);

    if (eid == EEVENTID_UNKNOWN)
    {
        LOG_WARN("DispatchMsgService : unknow evend id %d", eid);
        return ;
    }

    //LOG_DEBUG("dispatch ev : %d\n", ev->get_eid());

    T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
    if (handlers == subscribers_.end())
    {
        LOG_WARN("DispatchMsgService : no any event handler subscribed %d", eid);
        return ;
    }

    //存在有事件处理器
    iEvent* rsp = NULL;
    for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
    {
        iEventHandler* handler = *iter;
        LOG_DEBUG("DispatchMsgService : get handler: %s\n", handler->get_name().c_str());

        rsp = handler->handle(ev);//推荐使用vector 或list 返回多个rsp 
    }

    //return rsp;
}


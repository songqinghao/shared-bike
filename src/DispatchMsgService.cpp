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
    thread_cond_destroy(tp);
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
        iEvent* rsp = dms->process(ev);
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

    return thread_task_post(tp, task);

    //return msg_queue_.enqueue(ev, 0);
}

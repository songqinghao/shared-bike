#include "DispatchMsgService.h"
#include <algorithm>

#include "NetworkInterface.h"

/*
如果你使用xxx.h来定义一个类，然后使用xxx.cc来实现一个类中的方法，然后再yyy.cc中引用xxx.h文件，
那么如果你在xxx.h的类中声明了一个静态成员变量**（该成员变量是一个指针或者隐含指针），
那么你应该将这个静态成员变量的初始化（定义）放在xxx.cc文件中
*/
//将单例成员进行清空
DispatchMsgService*DispatchMsgService::DMS_=nullptr;
std::queue<iEvent*>DispatchMsgService::response_events;//存储event的响应队列
pthread_mutex_t DispatchMsgService::queue_mutex;//用于锁队列的锁
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

    thread_mutex_create(&queue_mutex);//初始化队列锁
    //初始化线程池
    tp = thread_pool_init();
    return tp ? true:false;
}

BOOL DispatchMsgService::close()
{
    //服务状态关闭
    svr_exit_ = true;
    
    thread_pool_destroy(tp);
    thread_mutex_destroy(&queue_mutex);
    subscribers_.clear();
    
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
        iEvent* rsp = dms->process(ev);
        //将rsp放到响应队列中去
        printf("process success!!,eid:%u\n",ev->get_eid());
        if(rsp)
        {
            printf("DispatchMsgService::svc not empty rsp eid:%u\n",rsp->get_eid());
            //打印rsp的信息
            rsp->dump(std::cout);
            printf("DispatchMsgService::svc dump success!!\n");
            //将会话再次进行记录（args就是cs）
            rsp->set_args(ev->get_args()); 
        }
        else
        {
            printf("DispatchMsgService::svc empty! \n");
            LOG_WARN("DispatchMsgService::svc rsp is nullptr\n");
            //创建一个中止响应传到队列里面去
            rsp = new ExitRspEv();
            rsp->set_args(ev->get_args());
        }

        //将响应事件投递到队列中
        thread_mutex_lock(&queue_mutex);
        response_events.push(rsp);
        thread_mutex_unlock(&queue_mutex);

    }
}

//将请求事件投递到线程池中
i32 DispatchMsgService::enqueue(iEvent* ev)
{
    if (NULL == ev)
    {
        return -1;
    }

    //先分配一个任务，将可分配的空间指向下一个
    thread_task_t* task = thread_task_alloc(0);

    //-------------------------------------------------------------------------
    //理解存在问题
    //理解清楚————2022-10-5
    //设置回调函数为DispatchMsgService::svc，到时候执行回调函数的时候就调用
    task->handler = DispatchMsgService::svc;
    task->ctx = ev;//回调时带的参数
    //-------------------------------------------------------------------------

    //把任务投到线程池中，立即就会被线程池中的线程调度执行
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
            //如果没有存在，则加到后面（一个事件可以由多个事件处理器进行处理）
            iter->second.push_back(handler);
            printf("DispatchMsgService::subscribe eid:%u handle追加\n",eid);
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
        //如果该事件定义了handler
        if (hdl_iter != iter->second.end())
        {
            iter->second.erase(hdl_iter);
        }
    }
}

iEvent* DispatchMsgService::process(const iEvent* ev)
{
    LOG_DEBUG("DispatchMsgService::process -ev: %p\n", ev);
    if (NULL == ev)
    {
        return NULL;
    }
    
    u32 eid = ev->get_eid();

    LOG_DEBUG("DispatchMsgService::process - eid: %u\n", eid);

    if (eid == EEVENTID_UNKNOWN)
    {
        LOG_WARN("DispatchMsgService : unknow evend id %d", eid);
        return NULL;
    }
    //迭代器看看事件有没有被订阅
    T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
    if (handlers == subscribers_.end())
    {
        LOG_WARN("DispatchMsgService : no any event handler subscribed %d", eid);
        return NULL;
    }
    std::cout<<"eid: "<<eid<<"已经订阅了！！"<<std::endl;
    //存在有事件处理器
    iEvent* rsp = NULL;
    //将事件的所有处理函数全部都处理一遍
    for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
    {
        //所有的事件全部都是由iEventHandler继承来的，用它来接实现handle
        iEventHandler* handler = *iter;
        LOG_DEBUG("DispatchMsgService : get handler: %s\n", handler->get_name().c_str());
        //返回一个响应事件
        rsp = handler->handle(ev);//推荐使用vector 或list 返回多个rsp (在这里应该就是只有一个rsp)
    }
    std::cout<<"eid: "<<eid<<"处理完了！！"<<"rsp eid："<<rsp->get_eid()<<std::endl;
    //这里的代码有瑕疵，应该是返回一整个队列，这里只返回了最后一个回调函数的返回，只是在当前场景不会出现多个订阅者，所以不会出现泄漏
    //后期进行修改.......................
    return rsp;
}

//解析事件
iEvent* DispatchMsgService::parseEvent(const char* messages,u32 len, u32 eid)
{   
    //如果消息为空
    if(!messages)
    {
        LOG_ERROR("DispatchMsgService::parseEvent——messages is null eid = %d.\n",eid);
        return nullptr;
    }

    //如果eid为手机验证码请求
    if(eid == EEVENTID_GET_MOBILE_CODE_REQ)
    {
        //定义手机验证码的解析方法
        tutorial::mobile_request mr;
        //如果成功解析
        if(mr.ParseFromArray(messages,len))
        {
            //创建手机验证码请求事件
            MobileCodeReqEv* ev = new MobileCodeReqEv(mr.mobile());
            return ev;
        }
    }
    //如果是请求登录的eid
    else if(eid == EEVENTID_LOGIN_REQ)
    {
        printf("DispatchMsgService::parseEvent eid==EEVENTID_LOGIN_REQ\n");
        tutorial::login_request lr;
        if(lr.ParseFromArray(messages,len))
        {
            //创建登录请求事件
            LoginReqEv* ev = new LoginReqEv(lr.mobile(),lr.icode());
            //printf("DispatchMsgService::parseEvent Create LoginReqEv success mobile %s,icode %d\n",lr.mobile(),lr.icode());
            std::cout<<"DispatchMsgService::parseEvent Create LoginReqEv success mobile "<<lr.mobile()<<" ,icode "<<lr.icode()<<std::endl;
            return ev;
        }
    } 

    return nullptr;
}

//处理响应队列中所有的响应事件
void DispatchMsgService::handleAllResponseEvent(NetworkInterface* interface)
{
    bool done = false;
    while(!done)
    {
        iEvent* ev = nullptr;
        //拿数据还是要用互斥锁
        thread_mutex_lock(&queue_mutex);
        //看队列是否为空
        if(!response_events.empty())
        {
            ev = response_events.front();
            response_events.pop();
        }
        //如果为空
        else
        {
            done = true;
        }
        thread_mutex_unlock(&queue_mutex);

        if(!done)
        {
            //如果是手机验证码响应
            if(ev->get_eid() == EEVENTID_GET_MOBILE_CODE_RSP)
            {   
                MobileCodeRspEv* mcre = static_cast<MobileCodeRspEv*>(ev);
                LOG_DEBUG("DispatchMsgService::handleAllResponseEvent——id:EEVENTID_GET_MOBILE_CODE_RSP)\n");
                //拿到会话状态
                ConnectSession*cs = (ConnectSession*)ev->get_args();
                cs->response = ev;
                //数据序列化，以后占的大小
                cs->message_len = mcre->ByteSize();
                //初始化write_buf的大小为头部+消息长度
                cs->write_buf = new char[cs->message_len+MESSAGE_HEADER_LEN];

                 //组装头部
                memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
                                             
                *(u16*)(cs->write_buf + 4) = EEVENTID_GET_MOBILE_CODE_RSP;
                *(i32*)(cs->write_buf + 6) = cs->message_len;

                mcre->SerializeToArray(cs->write_buf+MESSAGE_HEADER_LEN, cs->message_len);
                interface->send_response_message(cs);
           
            }
            else if(ev->get_eid() == EEVENTID_EXIT_RSP)
            {
                ConnectSession*cs = (ConnectSession*)ev->get_args();
                cs->response = ev;
                interface->send_response_message(cs);
            }
            //如果是登录响应的eid
            else if (ev->get_eid() == EEVENTID_LOGIN_RSP)
            {
                printf("DispatchMsgService::handleAllResponseEvent: eid=EEVENTID_LOGIN_RSP\n");
                //LoginResEv* lgre = static_cast<LoginResEv*>(ev);
                LOG_DEBUG("DispatchMsgService::handleAllResponseEvent - id：EEVENTID_LOGIN_RSP\n");

                ConnectSession* cs = (ConnectSession*)ev->get_args();
                cs->response = ev;

                //系列化请求数据
                //cs->message_len = lgre->ByteSize();
                cs->message_len = ev->ByteSize();
                cs->write_buf = new char[cs->message_len + MESSAGE_HEADER_LEN];

                //组装头部
                memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
                *(u16*)(cs->write_buf + 4) = EEVENTID_LOGIN_RSP;
                *(i32*)(cs->write_buf + 6) = cs->message_len;

                ev->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->message_len);

                interface->send_response_message(cs);
            }
        }

    }

}


#include "NetworkInterface.h"
#include "DispatchMsgService.h"
//切记：ConnectSession 必须是C类型的成员变量（要不然别随便memset，比如string类型的如果置零结构会被打乱）
static ConnectSession* session_init(int fd, struct bufferevent* bev) {
    ConnectSession* temp = nullptr;
    temp = new ConnectSession();

    if (!temp) {
        fprintf(stderr, "malloc failed. reason: %m\n");
        return nullptr;
    }
    
    memset(temp, '\0', sizeof(ConnectSession));
    //缓冲事件
    temp->bev = bev;
    temp->fd = fd;

}
NetworkInterface::NetworkInterface()
{
    base_ = nullptr;
    listener_ = nullptr; 
}
void session_reset(ConnectSession* cs)
{
    if(cs)
    {
        if(cs->read_buf)
        {
            delete[] cs->read_buf;
            cs->read_buf = NULL;
        }
        if(cs->write_buf)
        {
            delete[] cs->write_buf;
            cs->write_buf = NULL;
        }
        cs->session_stat = SESSION_STATUS::SS_REQUEST;//会话状态改成继续接收请求
        cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;

        cs->message_len = 0;
        cs->read_message_len = 0;
        cs->read_header_len = 0;

    }

}
NetworkInterface::~NetworkInterface()
{
    close();
}

//启动
bool NetworkInterface::start(int port)
{
    //地址绑定
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    //创建base
    base_ = event_base_new();
    //启动监听设置回调（回调方法为listener_cb）
    listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, base_,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        512, (struct sockaddr*)&sin,
        sizeof(struct sockaddr_in)); 

}


void NetworkInterface::close()
{
    if(base_)
    {
        event_base_free(base_);
        base_=nullptr;
    }
    if(listener_)
    {
        evconnlistener_free(listener_);
        listener_=nullptr;
    }
    
}

void session_free(ConnectSession* cs)
{
    if (cs)
    {
        if (cs->read_buf)
        {
            delete[] cs->read_buf;
            cs->read_buf = nullptr;

        }

        if (cs->write_buf)
        {
            delete[] cs->write_buf;
            cs->write_buf = nullptr;
        }
        delete cs;
    }
}

//监听回调消息
void  NetworkInterface::listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
    struct sockaddr* sock, int socklen, void* arg)
{
    struct event_base* base = (struct event_base*)arg;
    LOG_DEBUG("accept a client %d\n", fd);

    //为这个客户端分配一个bufferevent  
    struct bufferevent* bev = bufferevent_socket_new(base, fd,
        BEV_OPT_CLOSE_ON_FREE);

    ConnectSession *cs = session_init(fd, bev);
    cs->session_stat = SESSION_STATUS::SS_REQUEST;//状态设置为请求（处理请求）
    cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;//请求状态设置为读取头部
    //将网络地址转换为字符地址
    strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));

    LOG_DEBUG("remote ip : %s\n", cs->remote_ip);
    //请求来了回调handle_request，发送成功了以后设置的回调，出错以后设置的回调
    bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
    //允许读且是持久型的读取
    bufferevent_enable(bev, EV_READ | EV_PERSIST);
    //超时值应设置在配置文件
    //超时时间为10s，如果连接10s还没有数据来，写数据10s还没动作都是回调handle_error
    bufferevent_settimeout(bev, 20, 20);//超时值应设置在配置文件中
}

void NetworkInterface::handle_request(struct bufferevent* bev,void* arg)
{
    ConnectSession *cs = (ConnectSession*)arg;
    if(cs->session_stat!=SESSION_STATUS::SS_REQUEST)
    {
        LOG_WARN("NetWorkInterface::handle_request - wrong session state[%d].\n",cs->session_stat);
        return;
    }
    //如果现在处于读头部的阶段
    if(cs->req_stat==MESSAGE_STATUS::MS_READ_HEADER)
    {
        i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN-cs->read_header_len);
        cs->read_header_len += len;
        //将头部的最后一个位置设置为结束符
        cs->header[cs->read_header_len] = '\0';
        LOG_DEBUG("recv from client<<< %s\n", cs->header);

        //如果读取的头部长度与消息头部长度相等
        if(cs->read_header_len == MESSAGE_HEADER_LEN)
        {
            //比较头部中的前四个字节看是不是和最初定义的一致FBEB
            if (strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)) == 0)
            {
                //对eid和message_len进行赋值
                cs->eid = *((u16*)(cs->header+4));
                cs->message_len = *((i32*)(cs->header+6));
                LOG_DEBUG("NetworkInterface::handle_request - read  %d bytes in header, message len: %d\n", cs->read_header_len, cs->message_len);

                //对消息长度进行判断
                if (cs->message_len<1 || cs->message_len > MAX_MESSAGE_LEN) {
                    LOG_ERROR("NetworkInterface::handle_request wrong message, len: %u\n", cs->message_len);
                    bufferevent_free(bev);
                    session_free(cs);
                    return;
                }

                //给存储消息的地方进行分配内存
                cs->read_buf = new char[cs->message_len];
                //将事件状态改变
                cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;
                cs->read_message_len = 0;


            }else{
                //说明头部不是咱们自己定义的头部类型，“非法”
                LOG_ERROR("NetworkInterface::handle_request - Invalid request from %s\n", cs->remote_ip);
                //直接关闭请求，不给予任何响应,防止客户端恶意试探
                bufferevent_free(bev);
                session_free(cs);
                return;
            }
        }

    }
    //读取消息部分,evbuffer_get_length(bufferevent_get_input(bev)) 这样就可以拿到缓冲里有多少数据没有读取，因为有可能他就传了一个头部进来（内容为空）
    if(cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0)
    {
        i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
        cs->read_message_len += len;
        LOG_DEBUG("NetworkInterface::handle_request - bufferevent_read: %d bytes, message len: %d read len: %d\n", len, cs->message_len, cs->read_message_len);
        //看已经读取的长度和内容长度是否相等
        //如果消息数据已经读取完
        if(cs->message_len == cs->read_message_len)
        {
            //会话状态改变
            cs->session_stat = SESSION_STATUS::SS_RESPONSE;
            //解析事件
            iEvent *ev=DispatchMsgService::getInstance()->parseEvent(cs->read_buf,cs->read_message_len,cs->eid);           
            delete[] cs->read_buf;
            cs->read_buf = nullptr;
            cs->read_message_len = 0;

            //如果返回的事件有效，则....
            if(ev)
            {
                //ev{set_args,get_args}，用于后续进行响应的处理带出cs，方便设置到响应事件中
                ev->set_args(cs);//将cs保存到event中
                DispatchMsgService::getInstance()->enqueue(ev);
            
            }
            else
            {
                LOG_ERROR("NetworkInterface::handle_request ev is nullptr. remote ip: %s，eid = %d\n",cs->remote_ip,cs->eid);
                //直接关闭请求，不给予任何响应,防止客户端恶意试探
                bufferevent_free(bev);
                session_free(cs);
                
            }

        }
    }

}

void NetworkInterface::handle_response(struct bufferevent* bev, void* arg)
{
    LOG_DEBUG("NetworkInterface::handle_response.......\n");
}

//超时+连接关闭+读写出错都是调用handle_error
void NetworkInterface::handle_error(struct bufferevent* bev, short event, void* arg)
{
    ConnectSession* cs = (ConnectSession*)arg;
    LOG_DEBUG("NetworkInterface::handle_error.......\n");
    //如果连接关闭
    if(event & BEV_EVENT_EOF)
    {
        LOG_DEBUG("connection close............\n");
    }
    //读超时
    else if((event & BEV_EVENT_TIMEOUT)&& (event & BEV_EVENT_READING))
    {
        LOG_WARN("NetworkInterface::reading timeout......\n");
    }
    //写超时（写数据出去，但是因为网络问题不能写出去或系统内部出问题）
    else if((event & BEV_EVENT_TIMEOUT)&& (event & BEV_EVENT_WRITING))
    {
        LOG_WARN("NetworkInterface::writing timeout......\n");
    }
    else if(event & BEV_EVENT_ERROR)
    {
        LOG_ERROR("NetworkInterface::Error .....\n");
    }

    bufferevent_free(bev);
    session_free(cs);
}
//网络事件派发
void NetworkInterface::network_dispatch()
{
    event_base_loop(base_,EVLOOP_NONBLOCK);
    //处理请求事件，回复响应信息
    DispatchMsgService::getInstance()->handleAllResponseEvent(this);
}

void NetworkInterface::send_response_message(ConnectSession* cs)
{
    //如果没有响应事件
    if(cs->response == nullptr)
    {
        //关闭连接
        bufferevent_free(cs->bev);
        if(cs->request)
        {
            delete cs->request;
        }

        session_free(cs);
    }
    //如果响应不为空
    else
    {
        bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
        //将cs重置
        session_reset(cs);
    }
}
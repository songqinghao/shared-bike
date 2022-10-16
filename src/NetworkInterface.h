//网络接口类
#ifndef BRK_INTERFACE_NETWORK_INTERFACE_H_
#define BRK_INTERFACE_NETWORK_INTERFACE_H_


#include <event.h>
#include <event2/event.h>
#include<event2/listener.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "glo_def.h"
#include "ievent.h"

//头部长度：4+2+4 = 10字节（4：FBEB共享单车的传输信息协议，2：事件ID，4：数据长度）
//消息传输格式
#define MESSAGE_HEADER_LEN  10
#define MESSAGE_HEADER_ID   "FBEB"
//会话状态（处理请求？处理响应？）
enum class SESSION_STATUS
{
	//接收客户端请求读数据（处理请求）
	SS_REQUEST,
	//读完请求后进行响应（处理响应）
	SS_RESPONSE
};

//消息状态
enum class MESSAGE_STATUS
{
	//读取头部状态
	MS_READ_HEADER = 0,
	//读消息内容状态
	MS_READ_MESSAGE = 1,
	MS_READ_DONE = 2,      //消息传输完毕
	MS_SENDING = 3         //消息传输中
};

typedef struct _ConnectSession {
	//保存连接的IP地址
	char remote_ip[32];
    //当前会话连接状态
	SESSION_STATUS  session_stat;
    //客户端发来的请求事件
	iEvent* request;
    
	/*请求事件的状态
	//读取头部状态
	MS_READ_HEADER = 0,
	//读消息内容状态
	MS_READ_MESSAGE = 1,
	MS_READ_DONE = 2,     
	MS_SENDING = 3         
	*/
	MESSAGE_STATUS  req_stat;

    //处理完请求，还有响应事件
	iEvent* response;
	//响应的状态
	MESSAGE_STATUS  res_stat;

	u16  eid;    //保存当前请求的事件id
	i32  fd;     //保存当前传送的文件句柄

	struct bufferevent* bev;

	char* read_buf;                  //保存读消息的缓冲区（不保存头部，只保存内容）
	u32  message_len;                //当前读写消息的长度
	u32  read_message_len;           //已经读取的消息长度
	
	char header[MESSAGE_HEADER_LEN + 1]; //保存头部，10字节+1字节
	u32 read_header_len;                 //已读取的头部长度

	char* write_buf;					 //要写的缓冲区
	u32  sent_len;                       //已经发送的长度
	
}ConnectSession;



class NetworkInterface
{
    public:
        NetworkInterface();
        ~NetworkInterface();
		//启动
        bool start(int port);
        void close();
        //C++要调用C的函数，设置成static，监听请求的回调
        static void listener_cb(struct evconnlistener*listener, evutil_socket_t fd, struct sockaddr* sock, int socklen, void*arg);
		//读请求回调
        static void handle_request(struct bufferevent* bev,void* arg);
		//发送响应回调
        static void handle_response(struct bufferevent* bev, void* arg);
		//出错处理回调
        static void handle_error(struct bufferevent* bev, short event, void* arg);

		//循环处理（非阻塞的方法）
        void network_dispatch();
		//参数为会话状态（服务器与客户端之间的）
        void send_response_message(ConnectSession* cs);

    private:
	    struct evconnlistener* listener_;
	    struct event_base* base_;
};

#endif
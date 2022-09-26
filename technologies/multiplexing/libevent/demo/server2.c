#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
 
#include <unistd.h>
#include <event.h>
#include <event2/event.h>
#include <assert.h> 
 
#define BUFLEN  1024

typedef struct _ConnectStat {
	//struct event*  ev;
	struct bufferevent* bev;
	char buf[BUFLEN];
}ConnectStat;

//echo 服务实现相关代码
ConnectStat * stat_init(int fd, struct bufferevent *bev);
 
 
void accept_connection(int fd, short events, void* arg);
void do_echo_request(struct bufferevent* bev, void* arg);
void event_cb(struct bufferevent *bev, short event, void *arg);
//void do_echo_response(int fd, short events, void *arg);
 
int tcp_server_init(int port, int listen_num);

struct event_base* base;

int main(int argc, char** argv)
{
 
    int listener = tcp_server_init(9999, 10);
    if( listener == -1 )
    {
        perror(" tcp_server_init error ");
        return -1;
    }
 
    base = event_base_new();
 
    //添加监听客户端请求连接事件
    struct event* ev_listen = event_new(base, listener, EV_READ | EV_PERSIST,
                                         accept_connection, base);
    event_add(ev_listen, NULL);
 
 
    event_base_dispatch(base);
 
    return 0;
}

ConnectStat * stat_init(int fd, struct bufferevent *bev) {
	ConnectStat * temp = NULL;
	temp = (ConnectStat *)malloc(sizeof(ConnectStat));

	if (!temp) {
		fprintf(stderr, "malloc failed. reason: %m\n");
		return NULL;
	}

	memset(temp, '\0', sizeof(ConnectStat));
	temp->bev = bev;

}
 
void  accept_connection(int fd, short events, void* arg)
{
    evutil_socket_t sockfd;
 
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
 
    sockfd = accept(fd, (struct sockaddr*)&client, &len );
    evutil_make_socket_nonblocking(sockfd);
 
    printf("accept a client %d\n", sockfd);
 
    struct event_base* base = (struct event_base*)arg;
	
 
	struct bufferevent* bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);
	ConnectStat *stat = stat_init(sockfd, bev);
	
    bufferevent_setcb(bev, do_echo_request, NULL, event_cb, stat);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);
	
}
 
 
void do_echo_request(struct bufferevent* bev, void* arg)
{
    
    ConnectStat *stat = (ConnectStat *)arg;
    char *msg = stat->buf;
    printf("do echo request ...\n");
	
	size_t len = bufferevent_read(bev, msg, BUFLEN);
 
    msg[len] = '\0';
    printf("recv from client<<<< %s\n", msg);
 
    bufferevent_write(bev, msg, strlen(msg));
	
   
}


void event_cb(struct bufferevent *bev, short event, void *arg)
{
	ConnectStat *stat = (ConnectStat *)arg;
 
    if (event & BEV_EVENT_EOF)
        printf("connection closed\n");
    else if (event & BEV_EVENT_ERROR)
        printf("some other error\n");
 
    //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);
    free(stat);
}
 
 
 
typedef struct sockaddr SA;
int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    evutil_socket_t listener;// int listener;
 
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if( listener == -1 )
        return -1;
 
    //允许多次绑定同一个地址。要用在socket和bind之间
	//等同于 int opt = 1;
	//setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
    evutil_make_listen_socket_reuseable(listener);
 
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);
 
    if( bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
        goto error;
 
    if( listen(listener, listen_num) < 0)
        goto error;
 
 
    //跨平台统一接口，将套接字设置为非阻塞状态
    evutil_make_socket_nonblocking(listener);
 
    return listener;
 
    error:
        errno_save = errno;
        evutil_closesocket(listener);
        errno = errno_save;
 
        return -1;
}

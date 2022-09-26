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
	struct event*  ev;
	char buf[BUFLEN];
}ConnectStat;

//echo 服务实现相关代码
ConnectStat * stat_init(int fd, struct event *ev);
 
 
void accept_connection(int fd, short events, void* arg);
void do_echo_request(int fd, short events, void *arg);
void do_echo_response(int fd, short events, void *arg);
 
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

ConnectStat * stat_init(int fd, struct event *ev) {
	ConnectStat * temp = NULL;
	temp = (ConnectStat *)malloc(sizeof(ConnectStat));

	if (!temp) {
		fprintf(stderr, "malloc failed. reason: %m\n");
		return NULL;
	}

	memset(temp, '\0', sizeof(ConnectStat));
	temp->ev = ev;

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
	
 
    //仅仅是为了动态创建一个event结构体
    struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
    ConnectStat *stat = stat_init(sockfd, ev);
    
    //将动态创建的结构体作为event的回调参数
    event_assign(ev, base, sockfd, EV_READ ,
                 do_echo_request, (void*)stat);
 
    event_add(ev, NULL);
}
 
 
void do_echo_request(int fd, short events, void *arg)
{
    
    ConnectStat *stat = (ConnectStat *)arg;
    struct event *ev = stat->ev;
    char *msg = stat->buf;
    printf("do echo request ...\n");
	
    int len = read(fd, msg, BUFLEN - 1);
	
    if( len <= 0 )
    {
        printf("connectin close or some error happen when read\n");
        event_free(ev);
        close(fd);
        free(stat);
        return ;
    }
 
    msg[len] = '\0';
    printf("recv from client<<<< %s\n", msg);
    
    event_set(ev,  fd, EV_WRITE, do_echo_response, (void*)stat);
    ev->ev_base = base;        
    event_add(ev, NULL);
    //write(fd, reply_msg, strlen(reply_msg) );
}

void do_echo_response(int fd, short events, void *arg) {
	ConnectStat * stat = (ConnectStat *)(arg);
	struct event *ev = NULL;
	
	assert(stat!=NULL);
	ev = stat->ev;
	
	int len = strlen(stat->buf);
        printf("write to client>>>> %s\n", stat->buf);
	int _s = write(fd, stat->buf, len);
    if (_s > 0)
    {
            printf("write successfully.\n");
	}else if (_s == 0)  //client:close
    {
        fprintf(stderr,"Remote connection[fd: %d] has been closed\n", fd);
		goto  ERR_EXIT;
    }
    else //err occurred.
    {
        fprintf(stderr,"read faield[fd: %d], reason:%s [%d]\n",fd , strerror(errno), _s);
		goto  ERR_EXIT;
    }
	
    event_set(ev,  fd, EV_READ, do_echo_request, (void*)stat);
    ev->ev_base = base;        
    event_add(ev, NULL);
    return ;
	
ERR_EXIT:
    event_free(ev);
    close(fd);
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

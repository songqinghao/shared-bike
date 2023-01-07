#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h> 
#include <unistd.h>
#include <event.h>
#include <event2/event.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

typedef struct _ConnectStat  ConnectStat;

#define BUFLEN  1024

struct _ConnectStat {
	struct event*  ev;
	int fd;
	char send_buf[BUFLEN];
	//PF *handler;//不同页面的处理函数	
};

//echo 服务实现相关代码
ConnectStat * stat_init(int fd);
//void accept_connection(int fd, void *data);
void accept_connection(int fd, short events, void* arg);

void do_welcome_handler(int fd, short events, void* arg);
//void do_echo_handler(int fd, void  *data);
void do_echo_handler(int fd, short events, void *arg);

void do_echo_response(int fd, short events, void *arg);

//创建事件集event_base
struct event_base * ev_base;


void usage(const char* argv)
{
	printf("%s:[ip][port]\n", argv);
}

void set_nonblock(int fd)
{
	int fl = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

int startup(char* _ip, int _port)  //创建一个套接字，绑定，检测服务器
{
	//sock
	//1.创建套接字
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("sock");
		exit(2);
	}

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//2.填充本地 sockaddr_in 结构体（设置本地的IP地址和端口）
	struct sockaddr_in local;
	local.sin_port = htons(_port);
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = inet_addr(_ip);

	//3.bind（）绑定
	if (bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
	{
		perror("bind");
		exit(3);
	}
	//4.listen（）监听 检测服务器
	if (listen(sock, 5) < 0)
	{
		perror("listen");
		exit(4);
	}
	//返回套接字
	return sock;    
}


ConnectStat * stat_init(int fd) {
	ConnectStat * temp = NULL;
	temp = (ConnectStat *)malloc(sizeof(ConnectStat));

	if (!temp) {
		fprintf(stderr, "malloc failed. reason: %m\n");
		return NULL;
	}

	memset(temp, '\0', sizeof(ConnectStat));
	temp->fd = fd;
	//temp->status = 0;
}

void do_welcome_handler(int fd, short events, void* arg){
	const char * WELCOME= "Welcome.\n";
	int wlen = strlen(WELCOME);
	int n ;
	ConnectStat * stat = (ConnectStat *)(arg);
	
	if( (n = write(fd, "Welcome.\n",wlen)) != wlen ){
		
		if(n<=0){
		    fprintf(stderr, "write failed[len:%d], reason: %s\n",n,strerror(errno));
		}else fprintf(stderr, "send %d bytes only ,need to send %d bytes.\n",n,wlen);
		
	}else {
		//时间改变设置
		event_set(stat->ev, fd, EV_READ, do_echo_handler, (void *)stat);
		stat->ev->ev_base = ev_base;//必须重置事件集合
		//重新添加到事件集中
		event_add(stat->ev, NULL);
		
	}
}


void do_echo_handler(int fd, short events, void *arg) {
	ConnectStat * stat = (ConnectStat *)(arg);
	char * p = NULL;
	
	assert(stat!=NULL);
	
	
	p = stat->send_buf;
	*p++ = '-';
	*p++ = '>';
	ssize_t _s = read(fd, p, BUFLEN-(p-stat->send_buf)-1); //2字节"->" +字符结束符.
    	if (_s > 0)
    	{
		
		*(p+_s) = '\0';
		printf("receive from client: %s\n", p);
		//_s--;
		//while( _s>=0 && ( stat->send_buf[_s]=='\r' || stat->send_buf[_s]=='\n' ) ) stat->send_buf[_s]='\0';
		
		if(!strncasecmp(p, "quit", 4)){//退出.
			//comm_close(fd);
			event_free(stat->ev);//自动解除监听事件，释放event 
			close(fd);
            		free(stat);
			return ;
		}
		//调整event继续加入到事件集
		event_set(stat->ev, fd, EV_WRITE, do_echo_response, (void *)stat);
		stat->ev->ev_base = ev_base;//必须重置事件集合
		event_add(stat->ev, NULL);
		
	}else if (_s == 0)  //client:close
    	{
       		fprintf(stderr,"Remote connection[fd: %d] has been closed\n", fd);
        	event_free(stat->ev);//自动解除监听事件，释放event 
		close(fd);//关闭socket
        	free(stat);
    	}
    	else //err occurred.  出错处理暂时忽略
    	{
        	fprintf(stderr,"read faield[fd: %d], reason:%s [%ld]\n",fd , strerror(errno), _s);
		
 	}
}

void do_echo_response(int fd, short events, void *arg) {
	ConnectStat * stat = (ConnectStat *)(arg);
	int len = strlen(stat->send_buf);
	int _s = write(fd, stat->send_buf, len);
	
	if(_s>0){
		event_set(stat->ev, fd, EV_READ, do_echo_handler, (void *)stat);
		stat->ev->ev_base = ev_base;//必须重置事件集合
		event_add(stat->ev, NULL);
		
	}else if(_s==0){
		fprintf(stderr,"Remote connection[fd: %d] has been closed\n", fd);
	        event_free(stat->ev);//自动解除监听事件，释放event 
		close(fd);
	        free(stat);
	}else {
		fprintf(stderr,"read faield[fd: %d], reason:%d [%s]\n",fd ,_s ,strerror(errno));
	}
}

//read()
//注册写事件
  
//写事件就绪
//write()

//void accept_connection(int fd, void *data){
void accept_connection(int fd, short events, void* arg){
	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);
	//得到客户端的fd	
	int new_fd = accept(fd, (struct sockaddr*)&peer, &len);

	if (new_fd > 0)
	{
		ConnectStat *stat = stat_init(new_fd);
		set_nonblock(new_fd);

		printf("new client: %s:%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
		
		struct event *ev = event_new(ev_base, new_fd, EV_WRITE, do_welcome_handler, (void *)stat);
		stat->ev = ev;//调整事件
		event_add(ev, NULL);
	}
}


int main(int argc,char **argv){

	if (argc != 3)//检测参数个数是否正确
	{
		usage(argv[0]);
		exit(1);
	}
	int listen_sock = startup(argv[1], atoi(argv[2]));      //创建一个绑定了本地 ip 和端口号的套接字描述符
	//初始化事件集
	ev_base = event_base_new();
	//进行stat的初始化
	ConnectStat * stat = stat_init(listen_sock);
	//创建事件（事件集,fd,监听事件,持续读(也可以),调用的函数,参数）
	struct event* ev_listen = event_new(ev_base, listen_sock, EV_READ|EV_PERSIST, accept_connection, NULL);
	event_add(ev_listen, NULL);

	//事件循环
	event_base_dispatch(ev_base);

	//comm_select_shutdown();
        return 0;
}

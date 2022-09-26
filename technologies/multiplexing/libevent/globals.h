#ifndef GLOBALS_H
#define GLOBALS_H

#include <sys/time.h>
#include <sys/resource.h>	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#define FD_DESC_SZ		64

#define COMM_OK		  (0)
#define COMM_ERROR	 (-1)
#define COMM_NOMESSAGE	 (-3)
#define COMM_TIMEOUT	 (-4)
#define COMM_SHUTDOWN	 (-5)
#define COMM_INPROGRESS  (-6)
#define COMM_ERR_CONNECT (-7)
#define COMM_ERR_DNS     (-8)
#define COMM_ERR_CLOSING (-9)


//调试相关
#define DEBUG_LEVEL  0
#define DEBUG_ONLY   8
#define debug(m, n)    if( m >= DEBUG_LEVEL && n <= DEBUG_ONLY  ) printf
	
#define safe_free(x)	if (x) { free(x); x = NULL; }

typedef void PF(int, void *);

//fd所对应的各种信息和参数
struct _fde {
    unsigned int type;
    u_short local_port;//本地端口
    u_short remote_port;//远程端口
    struct in_addr local_addr;//地址
 
    char ipaddr[16];		/* dotted decimal address of peer 点分制地址*/
   
   
    PF *read_handler;//读处理的函数指针
    void *read_data;
    PF *write_handler;
    void *write_data;
    PF *timeout_handler;
    time_t timeout;
    void *timeout_data;//timeout处理时所对应的数据
};

typedef struct _fde fde;

extern fde *fd_table;  //fd=1=》fd_table[1]用作下标		
extern int Biggest_FD; //初始化连接	

/*系统时间相关,设置成全局变量，供所有模块使用*/
extern struct timeval current_time;
extern double current_dtime;
extern time_t sys_curtime;


/* epoll 相关接口实现 */
/* epoll 封装 */
extern void do_epoll_init(int max_fd);
extern void do_epoll_shutdown();
extern void epollSetEvents(int fd, int need_read, int need_write);
extern int do_epoll_select(int msec);

/*框架外围接口*/
/*给用户调用*/
void comm_init(int max_fd)
extern int comm_select(int msec);
extern inline void comm_call_handlers(int fd, int read_event, int write_event);
void  commUpdateReadHandler(int fd, PF * handler, void *data);
void commUpdateWriteHandler(int fd, PF * handler, void *data);



extern const char *xstrerror(void);//扩展型的string error
int ignoreErrno(int ierrno);//忽略错误

#endif /* GLOBALS_H */

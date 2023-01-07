#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/wait.h>
#include <string.h>
typedef void (*spawn_proc_pt) (void *data);
static void worker_process_cycle(void *data);
static void start_worker_processes(int n);
pid_t spawn_process(spawn_proc_pt proc, void *data, char *name); 

int main(int argc,char **argv){
    //启动四个工作进程
    start_worker_processes(4);
    //管理子进程，等待子进程结束
    wait(NULL);
}

void start_worker_processes(int n){
    int i=0;
    for(i = n - 1; i >= 0; i--){
       //创建四个子进程（处理任务的过程worker_process_cycle，传入i的地址，名字是work_process）
       spawn_process(worker_process_cycle,(void *)(intptr_t) i, "work_process");
    }
}

pid_t spawn_process(spawn_proc_pt proc, void *data, char *name){

    pid_t pid;
    pid = fork();

    switch(pid){
    case -1:
        fprintf(stderr,"fork() failed while spawning \"%s\"\n",name);
        return -1;
    case 0://如果是子进程
          proc(data);
          return 0;
    default:
          break;
    }
    //pid = fork();会产生两个pid对于子进程来说pid为0（成功的情况下），而父进程则会返回刚刚创建的子进程的pid   
    printf("start %s %ld\n",name,(long int)pid);
    return pid;
}

//设置cpu亲缘关系，把进程绑定在一个cpu的核上面
static void worker_process_init(int worker){
    cpu_set_t cpu_affinity;//cpu亲缘
    //worker = 2;
	//多核高并发处理  4个core  0core-0进程 1core-1进程 2-2 3-3  
    CPU_ZERO(&cpu_affinity);//将结构体清零
    CPU_SET(worker % CPU_SETSIZE,&cpu_affinity);// 0 1 2 3，将核与掩码进行绑定（CPU_SETSIZE为1024） 
    //sched_setaffinity
    //0:绑定自己
    if(sched_setaffinity(0,sizeof(cpu_set_t),&cpu_affinity) == -1){
       fprintf(stderr,"sched_setaffinity() failed\n");
    }
}

void worker_process_cycle(void *data){
    int worker = (intptr_t) data;//worker就是3210
    //初始化，将进程分配给各个核
    worker_process_init(worker);

    //干活，子进程该干什么就写在这下面
    for(;;){
      sleep(10);
      printf("pid %ld ,doing ...\n",(long int)getpid());
    }
}
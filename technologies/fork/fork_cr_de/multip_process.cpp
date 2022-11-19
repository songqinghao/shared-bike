#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/types.h>
typedef void (*spawn_proc_pt) (void *data);
static void worker_process_cycle(void *data);
static void start_worker_processes(int n);
pid_t spawn_process(spawn_proc_pt proc, void *data, char *name); 

int main(int argc,char **argv){
start_worker_processes(4);//启动四个工作进程
    //管理子进程，等待子进程都挂了
    wait(NULL);
}

void start_worker_processes(int n){
    int i=0;
    for(i = n - 1; i >= 0; i--){
       spawn_process(worker_process_cycle,(void *)(intptr_t) i, "worker process");//生产四个子进程（处理任务的过程worker_process_cycle，传入i的地址，名字是work_process）
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
    printf("start %s %ld\n",name,(long int)pid);//打印父进程的pid
    return pid;
}

//设置cpu亲缘关系，把进程绑定在一个cpu的核上面
static void worker_process_init(int worker){
    cpu_set_t cpu_affinity;//cpu亲缘
    //worker = 2;
	//多核高并发处理  4core  0 - 0 core 1 - 1  2 -2 3 -3  
    CPU_ZERO(&cpu_affinity);//将结构体清零
    CPU_SET(worker % CPU_SETSIZE,&cpu_affinity);// 0 1 2 3，将核与掩码进行绑定 
//sched_setaffinity
//0:绑定自己
    if(sched_setaffinity(0,sizeof(cpu_set_t),&cpu_affinity) == -1){
       fprintf(stderr,"sched_setaffinity() failed\n");
    }
}

void worker_process_cycle(void *data){
     int worker = (intptr_t) data;//worker就是3210
    //初始化
     worker_process_init(worker);

    //干活，子进程该干什么就写在这下面
    for(;;){
      sleep(10);
      printf("pid %ld ,doing ...\n",(long int)getpid());
    }
}
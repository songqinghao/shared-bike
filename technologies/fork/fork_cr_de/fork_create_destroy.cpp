#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
int main()
{
    pid_t fpid;//fpid表示fork函数返回的值
    int count = 0;
    int status = 0;
    fpid=fork();
    if (fpid < 0)
        printf("error in fork!");
    else if (fpid == 0) {
        printf("i am the child process, my process id is %d\n",getpid());
        printf("I’m children\n");
        count +=2;
        exit(2);//销毁子进程
    }
    else {
        printf("parent process return fpid:%d\n",fpid);
        printf("i am the parent process, my process id is %d\n",getpid());
        printf("I’m parent.\n");
        count++;
    }
    printf("统计结果是: %d\n",count);
    wait(&status);//返回进程挂掉的一个结构体（子进程exit后返回的）
    printf("parent:status:%d\n",WEXITSTATUS(status));//如果要返回当时进程挂掉的值，需要加宏WEXITSTATUS
    return 0;
}
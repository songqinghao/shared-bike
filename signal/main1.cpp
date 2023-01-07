#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void myhandle(int sig) 
{
	printf("Catch a signal : %d\n", sig);
    //又变回默认操作了
    signal(SIGINT, SIG_DFL);
}

int main(void) 
{
    //ctrl+c会发送SIGINT，当SIGINT触发后不再是退出进程而是触发myhandle方法，所以得手动kill进程才可以结束进程
	signal(SIGINT, myhandle);
	while (1) {
            sleep(1);
	}
	return 0;
}
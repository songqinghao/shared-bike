#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int my_global;

void* my_thread_handle(void *arg) 
{
	int val;

	val = *((int*)arg);

	printf("new thread begin, arg=%d\n", val);
	my_global += val;

	sleep(3);
	//退出程序，并将my_global传出去（记住my_globle不可是该线程中的临时变量）
	pthread_exit(&my_global);

	//不会到这一步
	printf("new thread end\n");
}

int main(void)
{
	pthread_t  mythread;
	int arg;
	int ret;
	void *thread_return;

	arg = 100;
	my_global = 1000;

	printf("my_global=%d\n", my_global);
	printf("ready create thread...\n");
	
	//创建线程，线程标识符为mythread，线程执行函数为my_thread_handle，函数所带的参数是arg
	ret = pthread_create(&mythread, 0, my_thread_handle, &arg);
	if (ret != 0) {
		printf("create thread failed!\n");
		exit(1);
	}

	printf("wait thread finished...\n");
	//利用thread_return将mythread线程退出时带的数据接住
	ret = pthread_join(mythread, &thread_return);
	if (ret != 0) {
		printf("pthread_join failed!\n");
		exit(1);
	}
	printf("wait thread end, return value is %d\n", *((int*)thread_return));
	printf("my_global=%d\n", my_global);

	printf("create thread finished!\n");
}

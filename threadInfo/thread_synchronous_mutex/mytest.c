#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define BUFF_SIZE 80

char buff[BUFF_SIZE];
sem_t sem1;
sem_t sem2;
time_t now;         //实例化time_t结构    
struct tm *timenow; //实例化tm结构指针    
   
//线程1执行的函数
void thread1_function(void*args)
{
    printf("come to thread1_function!!\n");
    while(1) {
		//P(sem)
		if (sem_wait(&sem1) != 0) {
			printf("sem_wait sem1 failed!\n");
			exit(1);
		}
		
		printf("string is: %slen=%d\n", buff, strlen(buff));
        sem_post(&sem2);//对sem2进行v操作
		if (strncmp(buff, "exit", 4) == 0) {
			break;
		}  
	}
}

//线程2执行的函数
void thread2_function(void*args)
{
    printf("come to thread2_function!!\n");
       
    while(1)
    {
        
        //P(sem)
		if (sem_wait(&sem2) != 0) {
			printf("sem_wait sem2 failed!\n");
			exit(1);
		}
        time(&now);
        //localtime函数把从time取得的时间now换算成你电脑中的时间(就是你设置的地区) 
        timenow = localtime(&now);   
        //asctime函数把时间转换成字符，通过printf()函数输出 
        printf("Local time is %s\n",asctime(timenow));

        if (strncmp(buff, "exit", 4) == 0) {
			break;
		}  
    }
     
}

int main()
{
    //创建两个子线程
    pthread_t mythread1;
    pthread_t mythread2;
    int ret;
    void* returnP;
    
    //初始化信号量
    ret = sem_init(&sem1, 0, 0);
    if(ret != 0)
    {
        printf("init sem1 failed!!!\n");
        exit(1);
    }
    ret = sem_init(&sem2, 0, 0);
    if(ret != 0)
    {
        printf("init sem2 failed!!!\n");
        exit(1);
    }

    //创建线程
    ret = pthread_create(&mythread1,0,thread1_function,0);
    if(ret != 0)
    {
        printf("create thread 1 failed!!\n");
        exit(1);
    }
    ret = pthread_create(&mythread2,0,thread2_function,0);
    if(ret != 0)
    {
        printf("create thread 2 failed!!\n");
        exit(1);
    }

    printf("come to while!!\n");
    while(1)
    {
        fgets(buff,sizeof(buff),stdin);
        if(sem_post(&sem1)!=0)
        {
            printf("sem_post sem1 failed!!\n");
            exit(1);
        }

        if (strncmp(buff, "exit", 4) == 0) {
			break;
		}  
    }
    
    //单纯为了等待两个线程结束
    pthread_join(mythread1,&returnP);
    pthread_join(mythread2,&returnP);
    //销毁信号量
    if(sem_destroy(&sem1)||sem_destroy(&sem2))
    {
        printf("destroy sem failed!!!\n");
        exit(1);
    }
    printf("destroy sem success!!!\n");
    return 0;
}

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

struct Conn_stat
{
	int count;
	char ip[64];
};


int main()
{
	
	void *shm = NULL;
	
	int shmid = 0, i = 0;
	struct Conn_stat stat = {0,"127.0.0.1"};
	
	//创建共享内存
	
	shmid = shmget((key_t)1234, sizeof(struct Conn_stat), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(1);
	}
	//将共享内存连接到当前进程的地址空间
	shm = shmat(shmid, (void*)0, 0);
	if(shm == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(2);
	}
	printf("Memory attached at %p\n", shm);
	
	//设置共享内存
	struct Conn_stat *p = (struct Conn_stat*)shm;
	memcpy(p,&stat,sizeof(struct Conn_stat));
	
	while((i++) < 30)//修改共享内存中写数据
	{
		p->count++;
		sleep(1);
	}
	
	//把共享内存从当前进程中分离（并不是删除哦）
	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(3);
	}
	
	exit(0);
}

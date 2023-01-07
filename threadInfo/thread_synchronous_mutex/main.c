#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFF_SIZE 80

char buff[BUFF_SIZE];
sem_t sem;

static void* str_thread_handle(void *arg) 
{
	while(1) {
		//P(sem)
		if (sem_wait(&sem) != 0) {
			printf("sem_wait failed!\n");
			exit(1);
		}
		
		printf("string is: %slen=%d\n", buff, strlen(buff));
		if (strncmp(buff, "end", 3) == 0) {
			break;
		}
	}
}

int main(void)
{
	int ret;
	pthread_t  str_thread;
	void *thread_return;


	ret = sem_init(&sem, 0, 0);
	if (ret != 0) {
		printf("sem_init failed!\n");
		exit(1);
	}

	ret = pthread_create(&str_thread, 0, str_thread_handle, 0);
	if (ret != 0) {
		printf("pthread_create failed!\n");
		exit(1);
	}

	while (1) {
		fgets(buff, sizeof(buff), stdin);

		//V(sem)
		if (sem_post(&sem) != 0) {
			printf("sem_post failed!\n");
			exit(1);
		}
		
		if (strncmp(buff, "end", 3) == 0) {
			break;
		}
	}

	ret = pthread_join(str_thread, &thread_return);
	if (ret != 0) {
		printf("pthread_join failed!\n");
		exit(1);
	}

	ret = sem_destroy(&sem);
	if (ret != 0) {
		printf("sem_destroy failed!\n");
		exit(1);
	}

	return 0;
}
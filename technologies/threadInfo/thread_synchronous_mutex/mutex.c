#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFF_SIZE 80

int global_value = 1000;
pthread_mutex_t  lock;

static void* str_thread_handle(void *arg) 
{
	int i = 0;

	for (i=0; i<10; i++) {
		pthread_mutex_lock(&lock);

		if (global_value  > 0) {
			// work
			sleep(1);
			printf("soled ticket(%d) to ChildStation(%d)\n",
				global_value, i+1);
		}
		global_value--;
		
		pthread_mutex_unlock(&lock);
		sleep(1);
	}
}

int main(void)
{
	int ret;
	pthread_t  str_thread;
	void *thread_return;
	int i;

	

	ret = pthread_mutex_init(&lock, 0);
	if (ret != 0) {
		printf("pthread_mutex_init failed!\n");
		exit(1);
	}

	ret = pthread_create(&str_thread, 0, str_thread_handle, 0);
	if (ret != 0) {
		printf("pthread_create failed!\n");
		exit(1);
	}

	for (i=0; i<10; i++) {
		pthread_mutex_lock(&lock);
		
		if (global_value  > 0) {
			// work
			sleep(1);
			printf("soled ticket(%d) to MainStation(%d)\n",
				global_value, i+1);
		}
		global_value--;
		
		
		pthread_mutex_unlock(&lock);
		sleep(1);
	}

	ret = pthread_join(str_thread, &thread_return);
	if (ret != 0) {
		printf("pthread_join failed!\n");
		exit(1);
	}

	ret = pthread_mutex_destroy(&lock);
	if (ret != 0) {
		printf("pthread_mutex_destroy failed!\n");
		exit(1);
	}

	return 0;
}


#include "thread.h"


int
thread_mutex_create(pthread_mutex_t *mtx)
{
    int            err;
    pthread_mutexattr_t  attr;

    err = pthread_mutexattr_init(&attr);
    if (err != 0) {
        fprintf(stderr, "pthread_mutexattr_init() failed, reason: %s\n",strerror(errno));
        return ERROR;
    }
    /*设置属性，PTHREAD_MUTEX_ERRORCHECK是检错锁，
    如果同一个线程请求同一个锁则返回edeadlk，
    否则与pthread_mutex_timed_np类型动作相同，这样防止死锁
    ------------------------------
    pthread_mutex_timed_np是缺省锁，也就是普通锁，
    当一个线程加锁后其余请求锁的线程则组成一个等待队列
    并在解锁后按顺序优先级获得锁
    */
    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (err != 0) {
		fprintf(stderr, "pthread_mutexattr_settype(PTHREAD_MUTEX_ERRORCHECK) failed, reason: %s\n",strerror(errno));
        return ERROR;
    }
    
    //属性被带到mtx中，追加属性
    err = pthread_mutex_init(mtx, &attr);
    if (err != 0) {
        fprintf(stderr,"pthread_mutex_init() failed, reason: %s\n",strerror(errno));
        return ERROR;
    }

    err = pthread_mutexattr_destroy(&attr);
    if (err != 0) {
		fprintf(stderr,"pthread_mutexattr_destroy() failed, reason: %s\n",strerror(errno));
    }

    return OK;
}


int
thread_mutex_destroy(pthread_mutex_t *mtx)
{
    int  err;

    err = pthread_mutex_destroy(mtx);
    if (err != 0) {
        fprintf(stderr,"pthread_mutex_destroy() failed, reason: %s\n",strerror(errno));
        return ERROR;
    }

    return OK;
}



int
thread_mutex_lock(pthread_mutex_t *mtx)
{
    int  err;

    err = pthread_mutex_lock(mtx);
    if (err == 0) {
        return OK;
    }
	fprintf(stderr,"pthread_mutex_lock() failed, reason: %s\n",strerror(errno));

    return ERROR;
}



int
thread_mutex_unlock(pthread_mutex_t *mtx)
{
    int  err;

    err = pthread_mutex_unlock(mtx);

#if 0
    ngx_time_update();
#endif

    if (err == 0) {
        return OK;
    }
	
	fprintf(stderr,"pthread_mutex_unlock() failed, reason: %s\n",strerror(errno));
    return ERROR;
}

#include "thread_pool.h"


static void thread_pool_exit_handler(void *data);
static void *thread_pool_cycle(void *data);
static int_t thread_pool_init_default(thread_pool_t *tpp, char *name);



static uint_t       thread_pool_task_id;

static int debug = 0;

thread_pool_t* thread_pool_init()
{
    int             err;
    pthread_t       tid;
    uint_t          n;
    pthread_attr_t  attr;
	thread_pool_t   *tp=NULL;

	tp = calloc(1,sizeof(thread_pool_t));

	if(tp == NULL){
	    fprintf(stderr, "thread_pool_init: calloc failed!\n");
	}

	thread_pool_init_default(tp, NULL);

    thread_pool_queue_init(&tp->queue);

    if (thread_mutex_create(&tp->mtx) != OK) {
		free(tp);
        return NULL;
    }

    if (thread_cond_create(&tp->cond) != OK) {
        (void) thread_mutex_destroy(&tp->mtx);
		free(tp);
        return NULL;
    }

    err = pthread_attr_init(&attr);
    if (err) {
        fprintf(stderr, "pthread_attr_init() failed, reason: %s\n",strerror(errno));
		free(tp);
        return NULL;
    }
    //PTHREAD_CREATE_DETACHED:在线程创建时将其属性红色为分离状态（detached）,主线程使用pthread_join无法等待到结束的子线程
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err) {
        fprintf(stderr, "pthread_attr_setdetachstate() failed, reason: %s\n",strerror(errno));
		free(tp);
        return NULL;
    }

    //tp->threads线程数
    for (n = 0; n < tp->threads; n++) {
        //thread_pool_cycle四个进程启动后都会执行这个方法
        err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
        if (err) {
            fprintf(stderr, "pthread_create() failed, reason: %s\n",strerror(errno));
			free(tp);
            return NULL;
        }
    }

    (void) pthread_attr_destroy(&attr);

    return tp;
}


void thread_pool_destroy(thread_pool_t *tp)
{
    uint_t           n;
    thread_task_t    task;
    volatile uint_t  lock;

    memset(&task,'\0', sizeof(thread_task_t));

    task.handler = thread_pool_exit_handler;//给一个自杀任务
    task.ctx = (void *) &lock;

    for (n = 0; n < tp->threads; n++) {
        lock = 1;

        if (thread_task_post(tp, &task) != OK) {
            return;
        }

        //提高效率，先把资源让出来
        while (lock) {
            sched_yield();//让出cpu的执行权
        }

        //task.event.active = 0;
    }

    (void) thread_cond_destroy(&tp->cond);
    (void) thread_mutex_destroy(&tp->mtx);

	free(tp);
}


static void
thread_pool_exit_handler(void *data)
{
    uint_t *lock = data;

    *lock = 0;

    pthread_exit(0);
}


thread_task_t *
thread_task_alloc(size_t size)//调用任务函数传输的参数大小size=sizeof(struct args)
{
    thread_task_t  *task;

    task = calloc(1,sizeof(thread_task_t) + size);
    if (task == NULL) {
        return NULL;
    }

    task->ctx = task + 1;//偏移一个task，ctx指向到size的内存

    return task;
}


int_t
thread_task_post(thread_pool_t *tp, thread_task_t *task)
{
    if (thread_mutex_lock(&tp->mtx) != OK) {//线程池上锁
        return ERROR;
    }
    //正在等待的任务数量已经达到最大
    if (tp->waiting >= tp->max_queue) {
        (void) thread_mutex_unlock(&tp->mtx);//解锁

        fprintf(stderr,"thread pool \"%s\" queue overflow: %ld tasks waiting\n",
                      tp->name, tp->waiting);
        return ERROR;
    }

    //task->event.active = 1;
    
    task->id = thread_pool_task_id++;//static thread_pool_task_id
    task->next = NULL;

    //发送信号（用于同步互斥）
    if (thread_cond_signal(&tp->cond) != OK) {
        (void) thread_mutex_unlock(&tp->mtx);
        return ERROR;
    }

    *tp->queue.last = task;
    tp->queue.last = &task->next;

    tp->waiting++;//等待的任务数++

    (void) thread_mutex_unlock(&tp->mtx);

    if(debug)fprintf(stderr,"task #%lu added to thread pool \"%s\"\n",
                   task->id, tp->name);

    return OK;
}


static void *
thread_pool_cycle(void *data)
{
    thread_pool_t *tp = data;

    int                 err;
    thread_task_t       *task;


    if(debug)fprintf(stderr,"thread in pool \"%s\" started\n", tp->name);

   

    for ( ;; ) {
        if (thread_mutex_lock(&tp->mtx) != OK) {//上锁
            return NULL;
        }

        //拿到任务将waiting--
        tp->waiting--;

        while (tp->queue.first == NULL) {//没有任务可以做
            if (thread_cond_wait(&tp->cond, &tp->mtx)//挂起，等任务进来再唤醒
                != OK)
            {
                (void) thread_mutex_unlock(&tp->mtx);
                return NULL;
            }
        }

        task = tp->queue.first;
        tp->queue.first = task->next;

        if (tp->queue.first == NULL) {
            tp->queue.last = &tp->queue.first;
        }
		
        if (thread_mutex_unlock(&tp->mtx) != OK) {
            return NULL;
        }



        if(debug) fprintf(stderr,"run task #%lu in thread pool \"%s\"\n",
                       task->id, tp->name);
        //执行任务
        task->handler(task->ctx);

        if(debug) fprintf(stderr,"complete task #%lu in thread pool \"%s\"\n",task->id, tp->name);

        task->next = NULL;

        //释放task
        free(task);
        //notify 
    }
}




static int_t
thread_pool_init_default(thread_pool_t *tpp, char *name)
{
	if(tpp)
    {
        tpp->threads = DEFAULT_THREADS_NUM;
        tpp->max_queue = DEFAULT_QUEUE_NUM;
            
        
		tpp->name = strdup(name?name:"default");
        if(debug)fprintf(stderr,
                      "thread_pool_init, name: %s ,threads: %lu max_queue: %ld\n",
                      tpp->name, tpp->threads, tpp->max_queue);

        return OK;
    }

    return ERROR;
}





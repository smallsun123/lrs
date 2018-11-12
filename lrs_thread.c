#include "lrs_thread.h"

static int lrs_create_thread(pthread_t *tidp,const pthread_attr_t *attr, (void*)(*start_rtn)(void*), void *arg)
{
    int ret;
    if((ret = pthread_create((pthread_t*)tidp, attr, start_rtn, (void*)arg)) < 0){
        printf("pthread_create failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_join_thread(pthread_t tidp, void **retval)
{
    int ret;
    if((ret = pthread_join((pthread_t)tidp, retval)) < 0){
        printf("pthread_join failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static void* _entry(void* arg)
{
    struct LRS_THREAD *thread = (struct LRS_THREAD*)arg;
    
    if(thread->worker){
        thread->worker(thread);
    } else {
        printf("pthread has no work to do ... \n");
    }
    return NULL;
}

struct LRS_THREAD* lrs_thread_alloc()
{
    struct LRS_THREAD *thread = NULL;
    thread = (struct LRS_THREAD*)malloc(sizeof(struct LRS_THREAD));
    if(!thread){
        printf("lrs_thread_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(thread, 0, sizeof(struct LRS_THREAD));

    thread->start = lrs_thread_start;
    thread->stop = lrs_thread_stop;
    thread->set_worker = lrs_set_thread_worker;
    
    thread->worker = NULL;
    thread->worker_arg = NULL;

    return thread;
}

void lrs_thread_free(struct LRS_THREAD *thread)
{
    if(thread){

        thread->start = NULL;
        thread->stop = NULL;
        thread->set_worker = NULL;
        
        thread->worker = NULL;
        thread->worker_arg = NULL;

        free(thread);
        thread = NULL;
    }
    return;
}


static int lrs_thread_start(struct LRS_THREAD *thread)
{
    int ret;
    if((ret = lrs_create_thread(&thread->thread_id, NULL, _entry, thread)) < 0){
        printf("lrs_thread_start failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}


static int lrs_thread_stop(struct LRS_THREAD *thread)
{
    int ret;
    void *retval;
    thread->thread_stop = 1;
    if((ret = lrs_join_thread(thread->thread_id, &retval)) < 0){
        printf("lrs_thread_stop failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_set_thread_worker(struct LRS_THREAD *thread, lrs_worker worker, void *arg)
{
    thread->worker = worker;
    thread->worker_arg = arg;
    return 0;
}


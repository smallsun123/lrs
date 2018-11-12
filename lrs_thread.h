#ifndef __LRS_THREAD__H__
#define __LRS_THREAD__H__

#include <pthread.h>

struct LRS_THREAD;

typedef int (*lrs_start_thread)(struct LRS_THREAD*);
typedef int (*lrs_stop_thread)(struct LRS_THREAD*);
typedef void* (*lrs_worker)(void*);
typedef int (*lrs_set_thread_worker)(struct LRS_THREAD*, lrs_worker, void*);


typedef struct LRS_THREAD{
    pthread_t       thread_id;
    int             thread_stop;

    lrs_start_thread start;
    lrs_stop_thread stop;
    lrs_set_thread_worker set_worker;
    
    lrs_worker  worker;
    void *worker_arg;
}LRS_THREAD;

struct LRS_THREAD* lrs_thread_alloc();
void lrs_thread_free(struct LRS_THREAD *thread);

#endif

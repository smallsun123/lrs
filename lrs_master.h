#ifndef __LRS_MASTER__H__
#define __LRS_MASTER__H__

#include "lrs_thread.h"
#include "lrs_epoll.h"
#include "lrs_sock.h"

typedef void* (*master_worker)(void*);

typedef struct LRS_MASTER{

    struct LRS_SOCKET *sock;
    struct LRS_EPOOL *epoll;
    struct LRS_THREAD *thread;
       
}LRS_MASTER;


struct LRS_MASTER* lrs_master_alloc();
void lrs_master_free(struct LRS_MASTER* master);

#endif

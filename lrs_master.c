#include "lrs_master.h"

struct LRS_MASTER* lrs_master_alloc()
{
    struct LRS_MASTER *master = NULL;
    master = (struct LRS_MASTER*)malloc(sizeof(struct LRS_MASTER));
    if(!master){
        printf("lrs_master_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }

    memset(master, 0, sizeof(struct LRS_MASTER));

    master->epoll = lrs_epoll_alloc(1);
    if(!master->epoll){
        printf("lrs_master_alloc epoll failed! error msg : %s\n", strerror(errno));
        free(master);
        return NULL;
    }

    master->sock = lrs_socket_alloc();
    if(!master->sock){
        printf("lrs_master_alloc epoll failed! error msg : %s\n", strerror(errno));
        lrs_epoll_free(master->epoll);
        free(master);
        return NULL;
    }

    master->thread = lrs_thread_alloc();
    if(!master->sock){
        printf("lrs_master_alloc epoll failed! error msg : %s\n", strerror(errno));
        lrs_epoll_free(master->epoll);
        lrs_socket_free(master->sock);
        free(master);
        return NULL;
    }

    
    master->thread->set_worker(master->thread, lrs_master_workder, master);
    master->thread->start(master->thread);
    
    return master;
}

void lrs_master_free(struct LRS_MASTER* master)
{
    if(master){
        if(master->thread){
            if(!master->thread->thread_stop)
                master->thread->stop(master->thread);
            lrs_thread_free(master->thread);
        }
        if(master->epoll){
            lrs_epoll_free(master->epoll);
        }
        if(master->sock){
            lrs_epoll_free(master->sock);
        }
        free(master);
        master = NULL;
    }
    return;
}

static void* lrs_master_workder(void *arg)
{
    int ret, i, fd, len;
    struct sockaddr_in addr;
    struct LRS_SESSION *session = NULL;
    struct LRS_THREAD *thread = (struct LRS_THREAD*)arg;
    struct LRS_MASTER *master = (struct LRS_MASTER*)thread->worker_arg;

    master->sock->createandlisten(master->sock);
    master->epoll->add(master->epoll->fd, master->sock->fd, EPOLLIN);

    for(;;){
        ret = master->epoll->wait(master->epoll);
        for(i = 0; i < ret; ++i){
            if(master->epoll->events[i].data.fd == master->sock->fd &&
                master->epoll->events[i].events & EPOLLIN){
                //new connect
                memset(&addr, 0, sizeof(struct sockaddr_in));
                fd = master->sock->accept(master->sock, &addr, &len);
                session = lrs_session_alloc(&addr, fd);
            }
        }
    }

    return NULL;
}


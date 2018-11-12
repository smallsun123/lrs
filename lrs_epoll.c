#include "lrs_epoll.h"

static int lrs_epoll_create(int size)
{
    int ret;
     if((ret = epoll_create(size)) < 0){
        printf("epoll_create failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event){
    int ret;
    if((epoll_ctl(epfd, op, fd, event)) < 0)
        printf("lrs_epoll_ctl failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_wait(int epfd, struct epoll_event * events, int maxevents, int timeout){
    int ret;
    if((ret = epoll_wait(epfd, events, maxevents, timeout)) < 0){
        printf("lrs_epoll_wait failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_add_event(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    lrs_epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    return 0;
}

static int lrs_delete_event(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    lrs_epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    return 0;
}

static int lrs_modify_event(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    lrs_epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    return 0;
}


struct LRS_EPOOL* lrs_epoll_alloc(int maxevents)
{
    struct LRS_EPOOL* epoll = NULL;
    epoll = (struct LRS_EPOOL*)malloc(sizeof(struct LRS_EPOOL));
    if(!epoll){
        printf("epoll_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(epoll, 0, sizeof(struct LRS_EPOOL));

    epoll->events = malloc(sizeof(struct epoll_event)*maxevents);
    if(!epoll->events){
        printf("epoll_alloc events failed! error msg : %s\n", strerror(errno));
        free(epoll);
        return NULL;
    }
    memset(epoll->events, 0, sizeof(struct epoll_event)*maxevents);
    
    epoll->fd = lrs_epoll_create(maxevents);
    epoll->wait = lrs_epoll_wait;
    epoll->add = lrs_epoll_add_event;
    epoll->delelte = lrs_epoll_delete_event;
    epoll->modify = lrs_epoll_modify_event;
    return epoll;
}

void lrs_epoll_free(struct LRS_EPOOL* epoll)
{
    if(epoll){
        if(epoll->events){
            free(epoll->events);
            epoll->events = NULL;
        }

        if(epoll->fd)
            close(epoll->fd);
 
        epoll->wait = NULL;
        epoll->add = NULL;
        epoll->delelte = NULL;
        epoll->modify = NULL;
    
        free(epoll);
        epoll = NULL;
    }
    return;
}

static int lrs_epoll_wait(struct LRS_EPOOL *epoll)
{
    return lrs_wait(epoll->fd, epoll->events, epoll->maxevents, EPOLL_TIME_OUT);
}

static int lrs_epoll_add_event(struct LRS_EPOOL *epoll, int fd, uint32_t events)
{
    return lrs_add_event(epoll->fd, fd, events);
}

static int lrs_epoll_delete_event(struct LRS_EPOOL *epoll, int fd, uint32_t events)
{
    return lrs_delete_event(epoll->fd, fd, events);
}

static int lrs_epoll_modify_event(struct LRS_EPOOL *epoll, int fd, uint32_t events)
{
    return lrs_modify_event(epoll->fd, fd, events);
}


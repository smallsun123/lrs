#ifndef __LRS_EPOLL__H__
#define __LRS_EPOLL__H__

#define EPOLLIN        0x001            //epollin, 表示对应的文件描述符可以读（包括对端SOCKET正常关闭）
#define EPOLLPRI       0x002            //epollpri, 表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）
#define EPOLLOUT       0x004            //epollout, 表示对应的文件描述符可以写
#define EPOLLRDNORM    0x040            //epollrdnorm, 普通数据可读
#define EPOLLRDBAND    0x080            //epollrdband, 优先级数据可读
#define EPOLLWRNORM    0x100            //epollwrnorm, 普通数据可写
#define EPOLLWRBAND    0x200            //epollwrband, 优先级数据可写
#define EPOLLMSG       0x400            //epollmsg, 消息队列数据
#define EPOLLERR       0x008            //epollerr, 表示对应的文件描述符发生错误
#define EPOLLHUP       0x010            //epollhup, 表示对应的文件描述符被挂断

#define EPOLLRDHUP     0x2000           //epollrdhup, 连接远端已经关闭或半关闭

/*
    ET模式 仅当状态发生变化的时候才获得通知, 这里所谓的状态的变化并不包括缓冲区中还有未处理的数据,也就是说,如果要采用ET模式,需要一直read/write直到出错为止,
    LT模式 只要有数据没有处理就会一直通知下去的.
*/
#define EPOLLET        0x80000000       //epollet, 将EPOLL设为边缘触发(Edge Triggered)模式，默认水平触发(Level Triggered)
#define EPOLLONESHOT   0x40000000       //epolloneshot, 只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里

#define EPOLL_CTL_ADD  1
#define EPOLL_CTL_DEL  2
#define EPOLL_CTL_MOD  3

#define EPOLL_TIME_OUT 500

struct LRS_EPOOL;

typedef union epoll_data {
    void         *ptr;
    int           fd;
    uint32_t      u32;
    uint64_t      u64;
} epoll_data_t;

struct epoll_event {
    uint32_t      events;
    epoll_data_t  data;
};

typedef int (*lrs_epoll_wait)(struct LRS_EPOOL*);
typedef int (*lrs_add_event)(struct LRS_EPOOL*, int, uint32_t);
typedef int (*lrs_delete_event)(struct LRS_EPOOL*, int, uint32_t);
typedef int (*lrs_modify_event)(struct LRS_EPOOL*, int, uint32_t);

typedef struct LRS_EPOOL{
    int fd;
    int maxevents;
    struct epoll_event *events;
    
    lrs_epoll_wait wait;
    lrs_add_event add;
    lrs_delete_event delelte;
    lrs_modify_event modify;
}LRS_EPOOL;

struct LRS_EPOOL* lrs_epoll_alloc(int maxevents);
void lrs_epoll_free(struct LRS_EPOOL* epoll);

#endif
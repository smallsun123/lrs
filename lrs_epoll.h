#ifndef __LRS_EPOLL__H__
#define __LRS_EPOLL__H__

#define EPOLLIN        0x001            //epollin, ��ʾ��Ӧ���ļ����������Զ��������Զ�SOCKET�����رգ�
#define EPOLLPRI       0x002            //epollpri, ��ʾ��Ӧ���ļ��������н��������ݿɶ�������Ӧ�ñ�ʾ�д������ݵ�����
#define EPOLLOUT       0x004            //epollout, ��ʾ��Ӧ���ļ�����������д
#define EPOLLRDNORM    0x040            //epollrdnorm, ��ͨ���ݿɶ�
#define EPOLLRDBAND    0x080            //epollrdband, ���ȼ����ݿɶ�
#define EPOLLWRNORM    0x100            //epollwrnorm, ��ͨ���ݿ�д
#define EPOLLWRBAND    0x200            //epollwrband, ���ȼ����ݿ�д
#define EPOLLMSG       0x400            //epollmsg, ��Ϣ��������
#define EPOLLERR       0x008            //epollerr, ��ʾ��Ӧ���ļ���������������
#define EPOLLHUP       0x010            //epollhup, ��ʾ��Ӧ���ļ����������Ҷ�

#define EPOLLRDHUP     0x2000           //epollrdhup, ����Զ���Ѿ��رջ��ر�

/*
    ETģʽ ����״̬�����仯��ʱ��Ż��֪ͨ, ������ν��״̬�ı仯���������������л���δ���������,Ҳ����˵,���Ҫ����ETģʽ,��Ҫһֱread/writeֱ������Ϊֹ,
    LTģʽ ֻҪ������û�д���ͻ�һֱ֪ͨ��ȥ��.
*/
#define EPOLLET        0x80000000       //epollet, ��EPOLL��Ϊ��Ե����(Edge Triggered)ģʽ��Ĭ��ˮƽ����(Level Triggered)
#define EPOLLONESHOT   0x40000000       //epolloneshot, ֻ����һ���¼���������������¼�֮���������Ҫ�����������socket�Ļ�����Ҫ�ٴΰ����socket���뵽EPOLL������

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
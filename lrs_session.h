#ifndef __LRS_SESSION__H__
#define __LRS_SESSION__H__

#define LRS_RTMP_MAX_SESSION 1024

typedef void* (*session_worker)(void*);

typedef struct LRS_SESSION{
    struct lrs_rbtree_node_t node;

    int inchunksize;
    int windowacksize;
    int peerbandwidth;
    
    LRS_HAND_SHAKE *handshake;
    LRS_AMF *amf;
    LRS_BUFFER *buffer;
    LRS_SOCKET *sock;
    LRS_EPOOL *epoll;
    LRS_RTMP_CHUNK *chunk;
    LRS_RTMP_PACKET *pkt;
    LRS_THREAD *thread;

    struct list_head gop;
}LRS_SESSION;

struct LRS_SESSION* lrs_session_alloc(struct sockaddr_in *addr, int fd);
void lrs_session_free(struct LRS_SESSION *session);

#endif

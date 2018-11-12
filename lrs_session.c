#include "lrs_session.h"

struct LRS_SESSION* lrs_session_alloc(struct sockaddr_in *addr, int fd)
{
    struct LRS_SESSION *session = NULL;
    session = (struct LRS_SESSION*)malloc(sizeof(struct LRS_SESSION));
    if(!session){
        printf("lrs_session_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(session, 0, sizeof(struct LRS_SESSION));

    //LRS_BUFFER *buffer;
    //LRS_RTMP_CHUNK *chunk;
    //LRS_RTMP_PACKET *pkt;

    session->handshake = lrs_handshake_alloc();
    if(!session->handshake){
        printf("lrs_session_alloc handshake failed! error msg : %s\n", strerror(errno));
        free(session);
        return NULL;
    }

    session->amf = amf_alloc();
    if(!session->amf){
        printf("lrs_session_alloc amf failed! error msg : %s\n", strerror(errno));
        lrs_handshake_free(session->handshake);
        free(session);
        return NULL;
    }

    session->sock = lrs_socket_alloc();
    if(!session->sock){
        printf("lrs_session_alloc sock failed! error msg : %s\n", strerror(errno));
        lrs_handshake_free(session->handshake);
        amf_free(session->amf);
        free(session);
        return NULL;
    }

    session->sock->set(session->sock, addr, 0, fd);    

    session->epoll = lrs_epoll_alloc(LRS_RTMP_MAX_SESSION);
    if(!session->epoll){
        printf("lrs_session_alloc sock failed! error msg : %s\n", strerror(errno));
        lrs_handshake_free(session->handshake);
        amf_free(session->amf);
        lrs_socket_free(session->sock);
        free(session);
        return NULL;
    }

    session->thread = lrs_thread_alloc();
    if(!session->thread){
        printf("lrs_session_alloc sock failed! error msg : %s\n", strerror(errno));
        lrs_handshake_free(session->handshake);
        amf_free(session->amf);
        lrs_socket_free(session->sock);
        lrs_epoll_free(session->epoll);
        free(session);
        return NULL;
    }

    session->buffer = lrs_buffer_alloc();
    if(!session->buffer){
        printf("lrs_session_alloc sock failed! error msg : %s\n", strerror(errno));
        lrs_handshake_free(session->handshake);
        amf_free(session->amf);
        lrs_socket_free(session->sock);
        lrs_epoll_free(session->epoll);
        lrs_thread_free(session->thread);
        free(session);
        return NULL;
    }

    INIT_LIST_HEAD(&session->gop);

    session->thread->set_worker(session->thread, lrs_session_worker, session);
    session->thread->start(session->thread);

    return session;
}

void lrs_session_free(struct LRS_SESSION *session)
{
    if(session){
        if(session->thread){
            if(!session->thread->thread_stop)
                session->thread->stop(session->thread);
            lrs_thread_free(session->thread);
        }
        if(session->handshake){
            lrs_handshake_free(session->handshake);
        }
        if(session->amf){
            amf_free(session->amf);
        }
        if(session->sock){
            lrs_socket_free(session->sock);
        }
        if(session->epoll){
            lrs_epoll_free(session->epoll);
        }
        free(session);
        session = NULL;
    }
    return;
}

static int lrs_handshake_to_client(struct LRS_SESSION *session){

    int len;
    struct LRS_BUFFER *writebuf = lrs_buffer_alloc(RTMP_HANDSHAKE_C1S1C2S2_SIZE*2 + RTMP_HANDSHAKE_C0S0_SIZE);
    session->sock->read(session->sock, session->buffer, 1);
    session->handshake->lrs_C0C1_s2c(session->handshake, session->buffer->pread, 
        session->buffer->pwrite - session->buffer->pread, writebuf->pwrite, &len);
    writebuf->pwrite += len;
    session->sock->write(session->sock, writebuf);
    session->buffer->reset(session->buffer);
    session->sock->read(session->sock, session->buffer, 1);
    session->handshake->lrs_handshake_C2_s2c(session->handshake, session->buffer->pread, 
        session->buffer->pwrite - session->buffer->pread);
    lrs_buffer_free(writebuf);

    return 0;
}

static int lrs_parse_chunksize(struct LRS_SESSION *session, struct LRS_RTMP_PACKET *pkt){

    struct LRS_RTMP_CHUNK *chunk = NULL;
    struct list_head *pos, *npos;
    list_for_each_safe(pos, npos, &pkt->list){
        chunk = list_entry(pos, struct LRS_RTMP_CHUNK, node);
        
        list_del(pos);
        lrs_rtmp_chunk_free(chunk);
    }
        
    session->inchunksize = session->amf->dint32(pkt->);
}

static int lrs_receive_message(struct LRS_SESSION *session, struct LRS_RTMP_PACKET *pkt){

    switch(pkt->type){
    case LRS_RTMP_SET_CHUNK_SIZE: {
        
    }break;
    case LRS_RTMP_ABORT_MESSAGE: {

    }break;
    case LRS_RTMP_ACKNOWLEDGEMENT: {

    }break;
    case LRS_RTMP_CONTROL: {

    }break;
    case LRS_RTMP_SET_WINDOW_ACK_SIZE: {

    }break;
    case LRS_RTMP_SET_PEER_BW: {

    }break;
    case LRS_RTMP_AUDIO: {

    }break;
    case LRS_RTMP_VIDEO: {

    }break;
    case LRS_RTMP_FLEX_STREAM_SEND: {

    }break;
    case LRS_RTMP_FLEX_SHARED_OBJECT: {

    }break;
    case LRS_RTMP_FLEX_MESSAGE: {

    }break;
    case LRS_RTMP_INFO: {

    }break;
    case LRS_RTMP_SHARED_OBJECT: {

    }break;
    case LRS_RTMP_INVOKE: {

    }break;
    case LRS_RTMP_FLASH_VIDEO: {

    }break;
    default : break;
    }
}

static void* lrs_session_worker(void *arg)
{
    int ret, len, i;
    struct LRS_THREAD *thread = (struct LRS_THREAD*)arg;
    struct LRS_SESSION *session = (struct LRS_SESSION*)thread->worker_arg;

    //handshake
    lrs_handshake_to_client(session);

    session->epoll->add(session->epoll->fd, session->sock->fd, EPOLLIN);
    for(;;){
        ret = session->epoll->wait(session->epoll);
        for(i = 0; i < ret; ++i){
            if(session->epoll->events[i].events & EPOLLIN){
                session->sock->read(session->sock, session->buffer, 1);
                session->pkt->parser(session->pkt, session->buffer, session->sock);

                if(){

                }
                list_add_tail(&session->pkt->node, &session->gop);
            }
        }
    }
    
    //connect
    
}


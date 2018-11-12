#include "lrs_sock.h"

static int lrs_create_socket()
{
    int ret;
    if((ret = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("lrs_create_socket failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_close(int fd){
    int ret;
    if((ret = close(fd)) < 0){
        printf("lrs_close failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_socket_reuseaddr(int fd){
    int on = 1, ret;
    if((ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))) < 0){
        printf("lrs_socket_reuseaddr failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_socket_reuseport(int fd){
    int on = 1, ret;
    if((ret = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int))) < 0){
        printf("lrs_socket_reuseport failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_setnonblock(int fd){
    int ret, opts;
    if((opts = fcntl(fd, F_GETFL)) < 0){
        printf("O_NONBLOCK F_GETFL failed! ret = %d, error msg : %s\n", opts, strerror(errno));
        return -1;
    }
    opts = opts | O_NONBLOCK;
    if((ret = fcntl(st, F_SETFL, opts)) < 0){
        printf("O_NONBLOCK F_SETFL failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_setnodelay(int fd){
    int on = 1, ret;
    if((ret = setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &on, sizeof(int))) < 0){
        printf("lrs_setnodelay failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_setcloseexec(int fd){
    int ret, opts;
    if((opts = fcntl(fd, F_GETFL)) < 0){
        printf("FD_CLOEXEC fcntl F_GETFL failed! ret = %d, error msg : %s\n", opts, strerror(errno));
        return -1;
    }
    opts = opts | FD_CLOEXEC;
    if((ret = fcntl(st, F_SETFL, opts)) < 0){
        printf("FD_CLOEXEC fcntl F_SETFL failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_bind(int fd, struct sockaddr_in *addr){
    int ret;
    if((ret = bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in))) < 0){
        printf("bind failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_listen(int fd, int max){
    int ret;
    if((ret = listen(fd, max)) < 0){
        printf("listen failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_accept(int fd, struct sockaddr_in *addr, int *len){
    int ret;
    if((ret = accept(fd, (struct sockaddr *)addr, len)) < 0){
        printf("accept failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_connect(int fd, struct sockaddr_in *addr){
    int ret;
    if((ret = connect(fd, (struct sockaddr*)addr, sizeof(struct sockaddr_in))) < 0){
        printf("connect failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_read(int fd, uint8_t *buf, int size){
    int ret;
    if((ret = recv(fd, buf, size, 0)) < 0){
        printf("recv failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}

static int lrs_write(int fd, const uint8_t *buf, int size){
    int ret;
    if((ret = send(s->fd, buf, size, MSG_NOSIGNAL)) < 0){
        printf("send failed! ret = %d, error msg : %s\n", ret, strerror(errno));
    }
    return ret;
}


struct LRS_SOCKET* lrs_socket_alloc()
{
    struct LRS_SOCKET *sock = NULL;
    sock = (struct LRS_SOCKET*)malloc(sizeof(struct LRS_SOCKET));
    if(!sock){
        printf("alloc_socket failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(sock, 0, sizeof(struct LRS_SOCKET));
    
    sock->accept = lrs_sockaccept;
    sock->createandlisten = lrs_create_socket_and_listen;
    sock->read = lrs_sockread;
    sock->write = lrs_sockwrite;
    sock->set = lrs_sockset;
    return sock;
}

void lrs_socket_free(struct LRS_SOCKET* sock)
{
    if(sock){
        if(sock->fd)
            lrs_close(sock->fd);
        
        sock->accept = NULL;
        sock->createandlisten = NULL;
        sock->read = NULL;
        sock->write = NULL;
        sock->set = NULL;

        free(sock);
        sock = NULL;
    }
    return;
}


static int lrs_create_socket_and_listen(struct LRS_SOCKET* sock)
{
    int ret;
    if((ret = sock->fd = lrs_create_socket()) < 0){
        return -1;
    }
    if((ret = lrs_socket_reuseaddr(sock->fd)) < 0){
        return -1;
    }
    if((ret = lrs_socket_reuseport(sock->fd) < 0){
        return -1;
    }

    memset(&sock->addr,0,sizeof(sock->addr));
    sock->addr.sin_family = AF_INET;
    sock->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock->addr.sin_port = htons(LRS_SERVER_PORT);

    if((ret = lrs_bind(sock->fd, &sock->addr)) < 0){
        return -1;
    }
    if((ret = lrs_setnonblock(sock->fd)) < 0){
        return -1;
    }
    if((ret = lrs_setnodelay(sock->fd)) < 0){
        return -1;
    }
    if((ret = lrs_setcloseexec(sock->fd)) < 0){
        return -1;
    }
    if((ret = lrs_listen(sock->fd, LRS_MAX_LISTEN)) < 0){
        return -1;
    }

    return ret;
}

static int lrs_sockread_asyc(struct LRS_SOCKET* sock, struct LRS_BUFFER *buffer, int increase)
{
    
}

static int lrs_sockread(struct LRS_SOCKET* sock, struct LRS_BUFFER *buffer, int increase)
{
    int ret=0, len=0, len2=0;

reread:
    if(buffer->pfree <= buffer->pwrite){
        len = buffer->end - buffer->pwrite;
        if(buffer->pfree > buffer->start){
            len2 = buffer->pfree - buffer->start;
        }
    } else {
        len = buffer->pfree - buffer->pwrite;
    }

    if((ret += lrs_read(sock->fd, buffer->pwrite, len)) < 0){
        printf("lrs_sockread failed! code : %d, msg : %s\n", ret, strerror(errno));
        return ret;
    }

    buffer->pwrite += ret;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    if(ret == len){

        if(increase){
            buffer->realloc(buffer);

            goto reread;
        }
        if((ret += lrs_read(sock->fd, buffer->pwrite, len2)) < 0){
            printf("lrs_sockread failed! code : %d, msg : %s\n", ret, strerror(errno));
            return ret;
        }
    }

    return ret;
}

static int lrs_sockwrite(struct LRS_SOCKET* sock, struct LRS_BUFFER *buffer)
{
    int ret;
    if(buffer->pread == buffer->pwrite){
        return 0;
    }
    if(buffer->pread < buffer->pwrite){
        ret = lrs_write(sock->fd, buffer->pread, buffer->pwrite - buffer->pread);
    } else {
        ret = lrs_write(sock->fd, buffer->pread, buffer->end - buffer->pread);
        ret += lrs_write(sock->fd, buffer->start, buffer->pwrite- buffer->start);
    }
    return ret;
}

static int lrs_sockaccept(struct LRS_SOCKET* sock, struct sockaddr_in *addr, int *len)
{
    return lrs_accept(sock->fd, addr, len);
}

static int lrs_sockset(struct LRS_SOCKET* sock, struct sockaddr_in *addr, int port, int fd)
{
    memcpy(&sock->addr, addr, sizeof(struct sockaddr_in));
    sock->port = port;
    sock->fd = fd;
    return 0;
}


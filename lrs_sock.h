#ifndef __LRS_SOCK__H__
#define __LRS_SOCK__H__

#include <sys/types.h>
#include <sys/socket.h>

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define LRS_SERVER_IP "0.0.0.0"
#define LRS_SERVER_PORT 1935
#define LRS_MAX_LISTEN 100

struct LRS_SOCKET;

typedef int (*sockaccept)(struct LRS_SOCKET*, struct sockaddr_in*, int*);
typedef int (*sockread)(struct LRS_SOCKET*, struct LRS_BUFFER*, int);
typedef int (*sockwrite)(struct LRS_SOCKET*, struct LRS_BUFFER*);
typedef int (*createlisten)(struct LRS_SOCKET*);
typedef int (*setsock)(struct LRS_SOCKET* sock, struct sockaddr_in *addr, int port, int fd);

typedef struct LRS_SOCKET{
    
    struct sockaddr_in addr;
    int port;
    int fd;

    sockaccept accept;
    sockread read;
    sockwrite write;
    createlisten createandlisten;
    setsock set;
}LRS_SOCKET;

struct LRS_SOCKET* lrs_socket_alloc();
void lrs_socket_free(struct LRS_SOCKET* sock);

#endif

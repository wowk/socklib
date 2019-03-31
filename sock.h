#ifndef SOCK_LIB_H_
#define SOCK_LIB_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


union sock_addr_u {
        struct sockaddr_storage ss;
        struct sockaddr_un su;
        struct sockaddr_in si;
        struct sockaddr_in6 si6;
};

struct sock_t {
    int fd;
    int af;
    int domain;
    int proto;
    union sock_addr_u saddr;
    union sock_addr_u daddr;
    socklen_t socklen;
};


extern int socket_addr(union sock_addr_u* sau, int af, const char* ip, int port, socklen_t* socklen);

extern struct sock_t* create_socket(int af, int domain, int proto);
extern struct sock_t* accept_socket(struct sock_t* sock);

extern int bind_socket(struct sock_t* sock, const char* ip, int port);
extern int listen_socket(struct sock_t* sock, int backlog);
extern int connect_socket(struct sock_t* sock, const char* ip, int port);

extern int send_socket(struct sock_t* sock, int flags, size_t iov_cnt, ...);
extern int recv_socket(struct sock_t* sock, int flags, size_t iov_cnt, ...);
extern int printf_socket(struct sock_t* sock, const char* fmt, ...);

extern void close_socket(struct sock_t* s);

#endif

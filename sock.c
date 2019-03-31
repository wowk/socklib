#include <debug.h>
#include <sock.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>


int socket_addr(union sock_addr_u* sau, int af, const char* ip, int port, socklen_t* socklen)
{
    memset(sau, 0, sizeof(*sau));

    if(af == AF_LOCAL){
        const char* path = ip;
        sau->su.sun_family = AF_LOCAL;
        strcpy(sau->su.sun_path, path);
        *socklen = sizeof(sau->su);
    }else if(af == AF_INET){
        sau->si.sin_family = AF_INET;
        sau->si.sin_port   = htons(port);
        if(1 != inet_pton(AF_INET, ip, &sau->si.sin_addr)){
            errno = EINVAL;
            error(0, errno, "failed to convert ipv4 addr");
            goto return_error;
        }
        *socklen = sizeof(sau->si);
    }else if(af == AF_INET6){
        sau->si6.sin6_family = af;
        sau->si6.sin6_port   = htons(port);
        if(1 != inet_pton(AF_INET6, ip, &sau->si6.sin6_addr)){
            errno = EINVAL;
            error(0, errno, "failed to convert ipv6 addr");
            goto return_error;
        }
        *socklen = sizeof(sau->si6);
    }else{
        errno = EPFNOSUPPORT;
        error(0, errno, "not support this protocol yet");
        goto return_error;
    }

    return 0;

return_error:
    return -errno;
}

struct sock_t* create_socket(int af, int domain, int proto)
{
    int fd = -1;
    struct sock_t* sock = NULL;

    fd = socket(af, domain, proto);
    if(fd < 0){
        error(0, errno, "failed to create socket(%d, %d, %d)", af, domain, proto);
        return NULL;
    }

    sock = (struct sock_t*)malloc(sizeof(struct sock_t));
    if(!sock){
        error(1, errno, "failed to create socket object");
        goto return_error;
    }

    sock->fd     = fd;
    sock->af     = af;
    sock->domain = domain;
    sock->proto  = proto;
    
    return sock;

return_error:
    if(fd >= 0)
        close(fd);
    if(sock)
        free(sock);
    return NULL;
}

int bind_socket(struct sock_t* sock, const char* ip, int port)
{
    if(0 > socket_addr(&sock->saddr, sock->af, ip, port, &sock->socklen)){
        error(0, errno, "failed to parse socket type");
        return -errno;
    }

    if(0 > bind(sock->fd, (struct sockaddr*)&(sock->saddr), sock->socklen)){
        error(0, errno, "failed bind socket");
        return -errno;
    }
    
    return 0;
}

int listen_socket(struct sock_t* sock, int backlog)
{
    int ret = listen(sock->fd, backlog);
    if(ret < 0){
        error(0, errno, "failed to listen on socket");
    }

    return ret;
}

struct sock_t* accept_socket(struct sock_t* sock)
{
    struct sock_t* cli_sock = (struct sock_t*)malloc(sizeof(struct sock_t));
    if(!cli_sock){
        error(0, errno, "failed to accept new connection");
        return NULL;
    }
   
    socklen_t socklen;
    cli_sock->fd = accept(sock->fd, (struct sockaddr*)&cli_sock->daddr, &socklen);
    if(cli_sock->fd < 0){
        error(0, errno, "failed to accept new connection");
        free(cli_sock);
        return NULL;
    }

    cli_sock->af     = sock->af;
    cli_sock->proto  = sock->proto;
    cli_sock->domain = sock->domain;
    cli_sock->socklen= sock->socklen;
    memcpy(&cli_sock->saddr, &sock->saddr, sock->socklen);

    return cli_sock;
}

int connect_socket(struct sock_t* sock, const char* ip, int port)
{
    if(0 > socket_addr(&sock->daddr, sock->af, ip, port, &sock->socklen)){
        error(0, errno, "failed to parse socket type");
        return -errno;
    }

    if(0 > connect(sock->fd, (struct sockaddr*)&(sock->daddr), sock->socklen)){
        error(0, errno, "failed bind socket");
        return -errno;
    }

    return 0;
}

int send_socket(struct sock_t* sock, int flags, size_t iov_cnt, ...)
{
    struct msghdr msghdr;
    struct iovec iovec[iov_cnt];
    va_list val;

    va_start(val, iov_cnt);
    for(size_t i = 0 ; i < iov_cnt ; i ++){
        iovec[i].iov_base  = va_arg(val, void*);
        iovec[i].iov_len   = va_arg(val, size_t);
    }
    va_end(val);

    memset(&msghdr, 0, sizeof(msghdr));
    msghdr.msg_iov    = iovec;
    msghdr.msg_iovlen = iov_cnt;
    if(sock->domain != SOCK_STREAM){
        msghdr.msg_name   = &sock->daddr;
        msghdr.msg_namelen= sock->socklen;
    }

    return sendmsg(sock->fd, &msghdr, flags);
}

int recv_socket(struct sock_t* sock, int flags, size_t iov_cnt, ...)
{
    struct msghdr msghdr;
    struct iovec iovec[iov_cnt];
    va_list val;

    va_start(val, iov_cnt);
    for(size_t i = 0 ; i < iov_cnt ; i ++){
        iovec[i].iov_base  = va_arg(val, void*);
        iovec[i].iov_len   = va_arg(val, size_t);
    }
    va_end(val);

    memset(&msghdr, 0, sizeof(msghdr));
    msghdr.msg_iov    = iovec;
    msghdr.msg_iovlen = iov_cnt;
    if(sock->domain != SOCK_STREAM){
        msghdr.msg_name   = &sock->daddr;
        msghdr.msg_namelen= sock->socklen;
    }

    return recvmsg(sock->fd, &msghdr, flags);
}

int printf_socket(struct sock_t* sock, const char* fmt, ...)
{
    va_list vl;
    char buffer[4096] = "";
    
    va_start(vl, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, vl);
    va_end(vl);

    return send_socket(sock, 0, 1, buffer, strlen(buffer));
}

void close_socket(struct sock_t* s)
{
    if(s){
        close(s->fd);
        free(s);
    }
}

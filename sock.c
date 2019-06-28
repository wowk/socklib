#include <debug.h>
#include <sock.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

/**************************************************************
 * AF_LOCAL   : <path>
 * AF_INET    : <ip> <port>
 * AF_INET6   : <ip> <port>
 * AF_NETLINK : <pid> <multicast groups>
 * AF_PACKET  : 1. <ifindex> <pkttype>
 *              2. <ifindex> <pkttype> <hatype> <halen> <haddr>
 * ***********************************************************/
ssize_t socket_addr(union sock_addr_u* sau, int af, int count, ...)
{
    va_list vl;
    ssize_t socklen;

    memset(sau, 0, sizeof(*sau));
    va_start(vl, count);

    if(af == AF_LOCAL){
        const char* path = va_arg(vl, const char*);
        if(count != 1){
            goto return_error;
        }
        sau->su.sun_family = AF_LOCAL;
        strcpy(sau->su.sun_path, path);
        socklen = sizeof(sau->su);
    }else if(af == AF_INET){
        const char* ip = va_arg(vl, const char*);
        uint16_t  port = va_arg(vl, uint32_t);
        if(count != 2){
            goto return_error;
        }
        sau->si.sin_family = AF_INET;
        sau->si.sin_port   = htons(port);
        if(1 != inet_pton(AF_INET, ip, &sau->si.sin_addr)){
            errno = EINVAL;
            error(0, errno, "failed to convert ipv4 addr");
            goto return_error;
        }
        socklen = sizeof(sau->si);
    }else if(af == AF_INET6){
        const char* ip = va_arg(vl, const char*);
        uint16_t  port = (uint16_t)va_arg(vl, int);
        if(count != 2){
            goto return_error;
        }
        sau->si6.sin6_family = af;
        sau->si6.sin6_port   = htons(port);
        if(1 != inet_pton(AF_INET6, ip, &sau->si6.sin6_addr)){
            errno = EINVAL;
            error(0, errno, "failed to convert ipv6 addr");
            goto return_error;
        }
        socklen = sizeof(sau->si6);
    }else if(af == AF_NETLINK){
        sau->snl.nl_family = AF_NETLINK;
        sau->snl.nl_pid = 0;
        sau->snl.nl_pid = va_arg(vl, pid_t);
        sau->snl.nl_groups = va_arg(vl, uint32_t);
        socklen = sizeof(sau->snl);
    }else if(af == AF_PACKET){
        if(count == 2){
            sau->sll.sll_ifindex = va_arg(vl, int);
            uint16_t proto = (uint16_t)va_arg(vl, int);
            debug(1, "=========proto: 0x%.4x==========", proto);
            sau->sll.sll_protocol= htons(proto);
            sau->sll.sll_halen   = va_arg(vl, int);
        }else if(count == 3){
            sau->sll.sll_ifindex = va_arg(vl, int);
            sau->sll.sll_protocol= htons(ETH_P_ARP);
            sau->sll.sll_hatype  = va_arg(vl, int);
            sau->sll.sll_halen   = va_arg(vl, int);
            sau->sll.sll_pkttype = va_arg(vl, int);
            memcpy(sau->sll.sll_addr, va_arg(vl, uint8_t*), sau->sll.sll_halen);
        }else{
            goto return_error;
        }
        sau->sll.sll_family = AF_PACKET;
        socklen = sizeof(sau->sll);
    }else{
        errno = EPFNOSUPPORT;
        error(0, errno, "not support this protocol yet");
        goto return_error;
    }

    return 0;

return_error:
    return -errno;
}

struct sock_t* socket_create(int af, int domain, int proto)
{
    int fd = -1;
    struct sock_t* sock = NULL;

    if(af == AF_PACKET){
        proto = htons(proto);
    }
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

int socket_bind(struct sock_t* sock, union sock_addr_u* addr, socklen_t socklen)
{
    if(0 > bind(sock->fd, (struct sockaddr*)addr, socklen)){
        error(0, errno, "failed bind socket");
        return -errno;
    }
    sock->socklen = socklen;
    if((&sock->addr) != addr){
        memcpy(&sock->addr, addr, socklen);
    }

    return 0;
}

int socket_listen(struct sock_t* sock, int backlog)
{
    int ret = listen(sock->fd, backlog);
    if(ret < 0){
        error(0, errno, "failed to listen on socket");
    }

    return ret;
}

struct sock_t* socket_accept(struct sock_t* sock)
{
    struct sock_t* cli_sock = (struct sock_t*)malloc(sizeof(struct sock_t));
    if(!cli_sock){
        error(0, errno, "failed to accept new connection");
        return NULL;
    }
   
    socklen_t socklen;
    cli_sock->fd = accept(sock->fd, (struct sockaddr*)&cli_sock->raddr, &socklen);
    if(cli_sock->fd < 0){
        error(0, errno, "failed to accept new connection");
        free(cli_sock);
        return NULL;
    }

    cli_sock->af     = sock->af;
    cli_sock->proto  = sock->proto;
    cli_sock->domain = sock->domain;
    cli_sock->socklen= sock->socklen;
    memcpy(&cli_sock->addr, &sock->addr, sock->socklen);

    return cli_sock;
}

int socket_connect(struct sock_t* sock, const char* ip, int port)
{
    ssize_t ret = socket_addr(&sock->raddr, sock->af, 2, ip, port);
    if(0 > ret){
        error(0, errno, "failed to parse socket type");
        return -errno;
    }
    sock->socklen = (socklen_t)ret;

    if(0 > connect(sock->fd, (struct sockaddr*)&(sock->raddr), sock->socklen)){
        error(0, errno, "failed bind socket");
        return -errno;
    }

    return 0;
}

int socket_send(struct sock_t* sock, int flags, size_t iov_cnt, ...)
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
        msghdr.msg_name   = &sock->raddr;
        msghdr.msg_namelen= sock->socklen;
    }

    return sendmsg(sock->fd, &msghdr, flags);
}

int socket_recv(struct sock_t* sock, int flags, size_t iov_cnt, ...)
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
        msghdr.msg_name   = &sock->raddr;
        msghdr.msg_namelen= sock->socklen;
    }

    return recvmsg(sock->fd, &msghdr, flags);
}

int socket_printf(struct sock_t* sock, const char* fmt, ...)
{
    va_list vl;
    char buffer[4096] = "";
    
    va_start(vl, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, vl);
    va_end(vl);

    return socket_send(sock, 0, 1, buffer, strlen(buffer));
}

void socket_close(struct sock_t* s)
{
    if(s){
        close(s->fd);
        free(s);
    }
}

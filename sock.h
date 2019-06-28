#ifndef SOCK_LIB_H_
#define SOCK_LIB_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h> 
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <linux/netlink.h>


union sock_addr_u {
        struct sockaddr_storage ss;
        struct sockaddr_un su;
        struct sockaddr_in si;
        struct sockaddr_in6 si6;
        struct sockaddr_ll sll;
        struct sockaddr_nl snl;
};

struct sock_t {
    int fd;
    int af, domain, proto;
    union sock_addr_u addr;
    union sock_addr_u raddr;
    socklen_t socklen;
};

#ifdef __cplusplus
extern "C" {
#endif

 /**************************************************************
 * AF_LOCAL   : <path>
 * AF_INET    : <ip> <port[uint16_t]>
 * AF_INET6   : <ip> <port[uint16_t]>
 * AF_NETLINK : <pid> <multicast groups>
 * AF_PACKET  : 1. <ifindex> <pkttype>
 *              2. <ifindex> <pkttype> <hatype> <halen> <haddr[uint8_t[8]]>
 * ***********************************************************/ 
ssize_t socket_addr(union sock_addr_u* sau, int af, int count, ...);

#define socket_addr_in(sau, af, ip, port) \
            socket_addr(sau, af, 2, (const char*)(ip), (uint32_t)(port))
#define socket_addr_local(sau, af, ip, port) \
            socket_addr(sau, AF_LOCAL, 2, (const char*)(ip), (uint32_t)(port))
#define socket_addr_ll(sau, ifindex, protocol, halen) \
            socket_addr(sau, AF_PACKET, 2, (int)(ifindex), (int)protocol, (int)halen)
#define socket_addr_arp(sau, ifindex, pkttype, hatype, halen, haddr) \
            socket_addr(sau, AF_PACKET, 5, (int)(ifindex), (int)(pkttype), (int)(hatype), (int)(halen), (int)(haddr))
#define socket_addr_nl(sau, pid, mc_groups) \
            socket_addr(sau, AF_NETLINK, 2, (pid_t)pid, (uint32_t)mc_groups)

struct sock_t* socket_create(int af, int domain, int proto);
struct sock_t* socket_accept(struct sock_t* sock);

int socket_bind(struct sock_t* sock, union sock_addr_u* addr, socklen_t socklen);
int socket_listen(struct sock_t* sock, int backlog);
int socket_connect(struct sock_t* sock, const char* ip, int port);

int socket_recv(struct sock_t* sock, int flags, size_t iov_cnt, ...);
int socket_send(struct sock_t* sock, int flags, size_t iov_cnt, ...);
int socket_printf(struct sock_t* sock, const char* fmt, ...);

void socket_close(struct sock_t* s);

#ifdef __cplusplus
}
#endif

#endif

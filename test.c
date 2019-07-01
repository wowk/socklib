#include "debug.h"
#include "sock.h"
#include <net/if.h>
#include <netinet/ether.h>
#include <linux/rtnetlink.h>


int main(int argc, char* argv[])
{
    struct sock_t* sock;
    struct ethhdr ethhdr;
    struct ether_arp arphdr;

    sock = socket_create(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if(!sock){
        error(1, errno, "failed to create socket");
    }
    
    sock->socklen = socket_addr_nl(&sock->addr, 0, RTMGRP_LINK|RTMGRP_IPV4_ROUTE);
    if(sock->socklen < 0){
        error(1, errno, "failed to get sockaddr_nl");
    }
    
    if(0 > socket_bind(sock, &sock->addr, sock->socklen)){
        error(1, errno, "failed to bind socket");
    }

    while(1) {
        ssize_t ret = socket_recv(sock, MSG_TRUNC, 2, &ethhdr, sizeof(ethhdr), &arphdr, sizeof(arphdr));
        if(ret < 60){
            continue;
        }
        
        info("get nl message");
    } 
    socket_close(sock);

    return 0;
}

#include "debug.h"
#include "sock.h"
#include <net/if.h>


int main(int argc, char* argv[])
{
    struct sock_t* sock;

    sock = socket_create(AF_PACKET, SOCK_RAW, ETH_P_ALL);
    if(!sock){
        error(1, errno, "failed to create socket");
    }
    
    int ifindex = if_nametoindex("enp3s0");
    if(ifindex < 0){
        error(1, errno, "failed to get ifindex");
    }

    sock->socklen = socket_addr_ll(&sock->addr, ifindex, ETH_P_ARP, ETH_ALEN);

    if(0 > socket_bind(sock, &sock->addr, sock->socklen)){
        error(1, errno, "failed to bind socket");
    }

    char buffer[1500];
    while(1) {
        
    }

    socket_close(sock);

    return 0;
}

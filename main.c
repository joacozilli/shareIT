#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "include/str.h"
#include "src/data_structures/array/array.h"
#include "src/data_structures/avl_tree/avl_tree.h"

#include "include/events.h"
#include "include/network.h"

// node structure would be something like this
// struct _Peer {
//     char* name;
//     char ip[INET_ADDDRSTRLEN];
//     int port;
//     int tolerance;
// };


int main() {

    struct ifaddrs* ifap;

    int ret = getifaddrs(&ifap);
    if (ret < 0)
        printf("error with getifaddrs\n");
    
    for (struct ifaddrs* temp = ifap; temp != NULL; temp = temp->ifa_next) {
        struct sockaddr_in* sa = (struct sockaddr_in *) temp->ifa_addr;
        char* addr = inet_ntoa(sa->sin_addr);
        printf("Interface: %s\tAddress: %s\n", temp->ifa_name, addr);
    }

    freeifaddrs(ifap);



    int bc_port = 12345;
    int srv_port = 60000;
    char* pc_ip = "192.168.1.77";
    char* broadcast = "255.255.255.255"; // now it works??

    char* notebook_ip = "192.168.1.45";
    int notebook_port = 50000;

    int tcp_sock = create_tcp_listener_socket(srv_port, pc_ip, 10);
    int udp_sock = create_broadcast_udp_socket(bc_port, NULL);

    int clientfd = create_tcp_client_socket(notebook_port, notebook_ip);

    char* msg = "MESSAGE SENT FROM PC TO NOTEBOOK VIA TCP\n";

    int nbytes = send_tcp_message(clientfd, (void*) msg, strlen(msg));
    printf("pc sent %d bytes to notebook\n", nbytes);

    char buff[120];
    nbytes = recv_udp_message(udp_sock, buff, 120);
    buff[nbytes] = '\0';
    printf("i received datagram: %s", buff);

    close(tcp_sock);
    close(udp_sock);

    return 0;
}
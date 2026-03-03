#include "server.h"

#include <stdio.h>
#include <sys/epoll.h>

#include "network.h"
#include "events.h"


handler_status_t main_handler(fd_info fd) {
    switch (fd->type) {
    case SOCKET_TCP_CLIENT:
        /* client request */
        break;
    case SOCKET_UDP:
        /* hello messages. Must discriminate and ignore my own hello's */

        break;
    case SEND_HELLO_TIMEOUT:
        /* hello timeout has ended. Must broadcast hello again */

        break;
    case CLEANUP_TIMEOUT:
        /* clean timeout has ended */
    
        break;

    default:
        break;
    }
}


int name_request(char* buff, int udpSock, int srvSock, int epfd) {

    buff = "id";

    struct epoll_event eventsQueue[100];
    int eventsReady = epoll_wait(epfd, eventsQueue, 100, -1);

    for (int i = 0; i < eventsReady; i++) {
        continue;
    }

}


int start_node(int srv_port, const char* ip, int broadcast_port) {
    int srvSocket = create_tcp_listener_socket(srv_port, ip, 1000);
    int udpSocket = create_broadcast_udp_socket(broadcast_port, NULL);

    int epfd = create_srv_epoll(srvSocket, udpSocket);

    // char id[ID_LENGTH];
    // while (name_request(id, udpSocket, srvSocket, epfd));

    // start working threads


}
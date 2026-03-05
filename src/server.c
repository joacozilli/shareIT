#include "server.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>

#include "network.h"
#include "events.h"
#include "utils.h"


handler_status_t main_handler(fd_info fd, server_info srv_info) {
    switch (fd->type) {
    case SOCKET_TCP_CLIENT:
        /* client request */
        uint16_t msg_len;
        int nbytes = recv_tcp_message(fd->fd_data->integer, (void*) &msg_len, HEADER_LENGTH);

        break;

    case SOCKET_UDP:
        /* hello messages. Must discriminate and ignore my own hello's */

        break;

    case FILE_TRANSFER:
        /* transfer a chunk */

        /* if finished, return CLIENT_CLOSE_CONNECTION, otherwise DOWNLOAD_IN_PROGRESS  */

        break;

    case SEND_HELLO_TIMEOUT:
        /* hello timeout has ended. Must broadcast hello again */
        u_int64_t buff;
        if (read(fd->fd_data->integer, (void*) &buff, 8) < 0) {
            errnoprintf("read in %s", __func__);
            return ERROR;
        }

        int attempts = 0;
        send_hello_msg:
        int ret = send_udp_mesage(srv_info->udp_socket,
                                 (void*) srv_info->hello_msg,
                                 strlen(srv_info->hello_msg),
                                 srv_info->broadcast_port,
                                 srv_info->broadcast_ip);
        attempts++;
        if (ret < 0) {
            if (attempts < MAX_HELLO_ATTEMPTS)
                goto send_hello_msg;
            else
                return ERROR;
        }
        return TIMEOUT_DONE;
        break;
    case CLEANUP_TIMEOUT:
        /* clean timeout has ended */
    
        break;

    default:
        break;
    }
}


int start_node(int srv_port, const char* ip, int broadcast_port, const char* broadcast_ip, char* srv_name) {
    int srvSocket = create_tcp_listener_socket(srv_port, ip, 1000);
    int udpSocket = create_broadcast_udp_socket(broadcast_port, NULL);

    int epfd = create_srv_epoll(srvSocket, udpSocket);

    server_info srv_info = malloc(sizeof(struct _server_info));
    srv_info->srv_name = srv_name;
    srv_info->srv_socket = srvSocket;
    srv_info->udp_socket = udpSocket;
    srv_info->srv_port = srv_port;
    srv_info->srv_ip = ip;
    srv_info->broadcast_port = broadcast_port;
    srv_info->broadcast_ip = broadcast_ip;

    char* hello_msg = malloc(sizeof(char) * 1024);
    snprintf(hello_msg, 1024, "HELLO %s %d %s", srv_info->srv_name, srv_info->srv_port, srv_info->srv_ip);
    hello_msg = realloc(hello_msg, sizeof(char) * (strlen(hello_msg) + 1));
    srv_info->hello_msg = hello_msg;

    // start working threads

}
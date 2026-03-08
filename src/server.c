#include "server.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <string.h>

#include "network.h"
#include "utils.h"
#include "array.h"
#include "str.h"
#include "peer.h"
#include "avl_concurrent.h"


handler_status_t main_handler(fd_info fd, uint32_t events , server_info srv_info) {
    switch (fd->type) {
    case SOCKET_TCP_CLIENT:
        /* client request */
        uint16_t msg_len;
        int nbytes = recv_tcp_message(fd->fd_data->integer, (void*) &msg_len, HEADER_LENGTH);

        /* if download request and the file exists, do this */
        transfer_info trans = malloc(sizeof (struct _transfer_info));
        trans->client_fd = fd->fd_data->integer;
        trans->file_fd; // = open(FILEPATH,...)
        trans->chunk_amount_sent = 0;
        trans->chunk_len = FILE_TRANSFER_CHUNK_SIZE;
        trans->transfer_completed = 0;
        fd->fd_data = malloc(sizeof(union _fd_data));
        fd->fd_data->trans_info = trans;
        fd->type = FILE_TRANSFER;
        return DOWNLOAD_REQUEST;

        break;

    case SOCKET_UDP:
        char buffer[255];
        nbytes = recv_udp_message(fd->fd_data->integer, buffer, 255);

        printf("udp msg received: %s\n", buffer);
        Array arr = parse_input(buffer, " ");
        if (arr == NULL)
            return TIMEOUT_OR_BROADCAST;

        /* hello messages have the form HELLO [NAME] [IP] [PORT]*/
        if (array_size(arr) == 4 && !strcmp(array_idx(arr, 0), "HELLO")) {
            char* name = array_idx(arr, 1);
            char* ip = array_idx(arr, 2);
            int port = atoi(array_idx(arr, 3));

            if (!strcmp(ip, srv_info->srv_ip) && port == srv_info->srv_port) {
                array_destroy(arr);
                return TIMEOUT_OR_BROADCAST;
            }
            struct _peer p;
            p.name = name;
            p.ip = ip;
            p.port = port;
            p.tolerance = 0;
            concurrent_avl_insert(srv_info->peers, (void*) &p);
            printf("I found a new peer!!\n");
            concurrent_avl_print(srv_info->peers);
        }
        array_destroy(arr);
        return TIMEOUT_OR_BROADCAST;
        break;

    case FILE_TRANSFER:
        /* transfer a chunk */
        if (events & EPOLLOUT) {

        }
        /* client sent something, probably an error during transfer on his behalf */
        else if (events & EPOLLIN) {

        }
        /* if finished, return CLIENT_CLOSE_CONNECTION, otherwise DOWNLOAD_IN_PROGRESS  */
        
        break;

    case SEND_HELLO_TIMEOUT:
        printf("sending hello event...\n");
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
        return TIMEOUT_OR_BROADCAST;
        break;
    case CLEANUP_TIMEOUT:
        printf("cleanup event...\n");
        buff;
        read(fd->fd_data->integer, (void*) &buff, 8);
        return TIMEOUT_OR_BROADCAST;   
        break;

    default:
        break;
    }
}


int start_node(int srv_port, char* ip, int broadcast_port, char* broadcast_ip, char* srv_name) {
    int srvSocket = create_tcp_listener_socket(srv_port, ip, 1000);
    int udpSocket = create_broadcast_udp_socket(broadcast_port, NULL);

    int epfd = create_srv_epoll(srvSocket, udpSocket);

    printf("srvSocket: %d, udpSocket: %d, epfd: %d\n", srvSocket, udpSocket, epfd);

    server_info srv_info = malloc(sizeof(struct _server_info));
    srv_info->srv_name = srv_name;
    srv_info->srv_socket = srvSocket;
    srv_info->udp_socket = udpSocket;
    srv_info->srv_port = srv_port;
    srv_info->srv_ip = ip;
    srv_info->broadcast_port = broadcast_port;
    srv_info->broadcast_ip = broadcast_ip;

    srv_info->peers = concurrent_avl_create(peer_copy, peer_compare, peer_delete, peer_print);

    char* hello_msg = malloc(sizeof(char) * 1024);
    snprintf(hello_msg, 1024, "HELLO %s %s %d", srv_info->srv_name, srv_info->srv_ip, srv_info->srv_port);
    hello_msg = realloc(hello_msg, sizeof(char) * (strlen(hello_msg) + 1));
    srv_info->hello_msg = hello_msg;

    create_hello_timeout(epfd);
    create_cleanup_timeout(epfd);
    // start working threads

    wait_epoll_events(epfd, srv_info, main_handler);

    close(srvSocket);
    close(udpSocket);
    close(epfd);

    return 0;

}
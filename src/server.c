#include "server.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include "network.h"
#include "utils.h"
#include "array.h"
#include "str.h"
#include "peer.h"
#include "avl_concurrent.h"


handler_status_t main_handler(fd_info fd, uint32_t events , server_info srv_info) {
    int nbytes;
    int buffer_len = 1024;
    char buffer[buffer_len];
    uint16_t msg_len;
    uint16_t msg_len_network_order;
    Array arr;
    switch (fd->type) {
    case SOCKET_TCP_CLIENT:
        nbytes = recv_tcp_message(fd->fd_data->integer, (char*) &msg_len_network_order, HEADER_LENGTH);
        msg_len = ntohs(msg_len_network_order);

        if (nbytes <= 0 || msg_len <= 0)
            return CLIENT_CLOSE_CONNECTION;

        if (msg_len > buffer_len) {
            /* message longer than it should, ignore it */
            char tempbuff[nbytes];
            recv_tcp_message(fd->fd_data->integer, (void*) tempbuff, nbytes);
            return CLIENT_CONTINUE_CONNECTION;
        }
        nbytes = recv_tcp_message(fd->fd_data->integer, buffer, nbytes);
        if (nbytes <= 0)
            return CLIENT_CLOSE_CONNECTION;

        arr = parse_input(buffer, " ");
        if (arr == NULL)
            return CLIENT_CONTINUE_CONNECTION;
        
        
        if (array_size(arr) == 2 && !strcmp(array_idx(arr, 0), "DOWNLOAD_REQUEST")) {
            char* filename = array_idx(arr, 1);
            if (0 /* file doesn't exist */) {
                char msg[1024];
                sprintf(msg, "NOT_FOUND %s", filename);
                msg_len_network_order = htons(strlen(msg));
                /* send header first */
                send_tcp_message(fd->fd_data->integer, (char*) &msg_len_network_order, HEADER_LENGTH);
                send_tcp_message(fd->fd_data->integer, msg, strlen(msg));
                return CLIENT_CONTINUE_CONNECTION;
            }
        }

        /* if download request and the file exists, do this */
        transfer_info trans = malloc(sizeof (struct _transfer_info));
        trans->client_fd = fd->fd_data->integer;
        trans->file_fd; // = open(FILEPATH,...)
        trans->chunk_amount_sent = 0;
        trans->chunk_len = FILE_TRANSFER_CHUNK_SIZE;
        fd->fd_data = malloc(sizeof(union _fd_data));
        fd->fd_data->trans_info = trans;
        fd->type = FILE_TRANSFER;
        return DOWNLOAD_REQUEST;

        break;

    case SOCKET_UDP:
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
            // whole chunk has been already sent, load next
            if (fd->fd_data->trans_info->chunk_amount_sent == fd->fd_data->trans_info->chunk_len) {
                nbytes = read(fd->fd_data->trans_info->file_fd,
                              fd->fd_data->trans_info->chunk_buffer,
                              fd->fd_data->trans_info->chunk_len);

                fd->fd_data->trans_info->chunk_amount_sent = 0;
                if (nbytes == 0) {
                    /*
                    transfer completed. Rearm fd as type SOCKET_TCP_CLIENT.
                    */
                    close(fd->fd_data->trans_info->file_fd);
                    fd->type = SOCKET_TCP_CLIENT;
                    int temp = fd->fd_data->trans_info->client_fd;
                    free(fd->fd_data->trans_info);
                    fd->fd_data->integer = temp;
                    return CLIENT_CONTINUE_CONNECTION;
                }
            }
            nbytes = send(fd->fd_data->trans_info->client_fd, 
                          fd->fd_data->trans_info->chunk_buffer + fd->fd_data->trans_info->chunk_amount_sent,
                          fd->fd_data->trans_info->chunk_len - fd->fd_data->trans_info->chunk_amount_sent, 0);
            
            if (nbytes < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return DOWNLOAD_REQUEST;
                else {
                    errnoprintf("send in main_handler (error when sending chunk of file): %s", __func__);
                    return ERROR;
                }
            }
            fd->fd_data->trans_info->chunk_amount_sent += nbytes;
            return DOWNLOAD_REQUEST;
        }

        /* client sent something, probably an error during transfer on his behalf */
        else if (events & EPOLLIN) {

        }
        
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
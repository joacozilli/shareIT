#include "server/server.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "server/protocol.h"
#include "server/downloads.h"
#include "server/peeks.h"
#include "network.h"
#include "utils.h"
#include "log.h"
#include "array.h"
#include "str.h"
#include "peer.h"
#include "avl_concurrent.h"
#include "files.h"
#include "directories.h"
#include "cli.h"



void* substract_tolerance(void* _peer, void* context) {
    peer p = (peer) _peer;
    Array forget = (Array) context;

    p->tolerance--;
    if (p->tolerance == 0)
        array_add(forget, p->name);

    return _peer;
}

void* forget_peers(void* _peer_name, void* context) {
    conc_AVL peers = (conc_AVL) context;
    struct _peer p;
    p.name = (char*) _peer_name;
    peer ret = concurrent_avl_search_by(peers, &p, peer_compare_names);
    if (ret != NULL) {
        log_info("forgot peer %s (no hello message received for a while)", p.name);
        concurrent_avl_delete(peers, ret);
    }
    return _peer_name;
}



handler_status_t main_handler(fd_info fd, uint32_t events , server_info srv_info) {
    int nbytes;
    int buffer_len = 1024;
    char buffer[buffer_len];
    uint16_t msg_len;
    uint16_t msg_len_network_order;
    Array arr;
    switch (fd->type) {
    case SOCKET_TCP_CLIENT:

        if (events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
            log_info("hang up detected, closing connection.");
            return CLIENT_CLOSE_CONNECTION;
        }

        nbytes = recv_tcp_message(fd->fd_data->integer, (char*) &msg_len_network_order, HEADER_LENGTH);
        msg_len = ntohs(msg_len_network_order);

        if (nbytes <= 0 || msg_len <= 0) {
            log_error("unable to receive tcp message header");
            return CLIENT_CLOSE_CONNECTION;
        }

        if (msg_len > buffer_len) {
            /* message longer than it should, ignore it */
            char tempbuff[nbytes];
            recv_tcp_message(fd->fd_data->integer, (void*) tempbuff, nbytes);
            return CLIENT_CONTINUE_CONNECTION;
        }
        nbytes = recv_tcp_message(fd->fd_data->integer, buffer, msg_len);
        if (nbytes <= 0) {
            log_error("unable to receive tcp message body");
            return CLIENT_CLOSE_CONNECTION;
        }
        buffer[msg_len] = '\0';

        arr = parse_input(buffer, " ");
        if (arr == NULL)
            return CLIENT_CONTINUE_CONNECTION;
        
        
        if (array_size(arr) == 2 && !strcmp(array_idx(arr, 0), DOWNLOAD_REQUEST_MSG)) {
            log_info("download request received");
            handler_status_t ret = download_request(fd, srv_info->files, array_idx(arr,1));
            array_destroy(arr);
            return ret;
        }

        else if (array_size(arr) == 1 && !strcmp(array_idx(arr, 0), PEEK_REQUEST_MSG)) {
            log_info("peek request received");
            handler_status_t ret = see_files_request(fd, srv_info->files);
            array_destroy(arr);
            return ret;
        }

        break;

    case SOCKET_UDP:
        nbytes = recv_udp_message(fd->fd_data->integer, buffer, buffer_len);
        buffer[nbytes] = '\0';
        
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
            p.tolerance = MAX_TOLERANCE;
            log_info("received hello from %s %s %d", name, ip, port);
            concurrent_avl_insert(srv_info->peers, (void*) &p);
        }
        array_destroy(arr);
        return TIMEOUT_OR_BROADCAST;
        break;

    case FILE_TRANSFER:
        return file_transfer(fd); 
        break;

    case SEND_HELLO_TIMEOUT:
        u_int64_t buff;
        if (read(fd->fd_data->integer, (void*) &buff, sizeof buff) < 0) {
            log_errno("error in read");
            return ERROR;
        }
        send_udp_mesage(srv_info->udp_socket,
                        srv_info->hello_msg,
                        strlen(srv_info->hello_msg),
                        srv_info->broadcast_port,
                        srv_info->broadcast_ip);
        return TIMEOUT_OR_BROADCAST;
        break;

    case CLEANUP_TIMEOUT:
        if (read(fd->fd_data->integer, (void*) &buff, sizeof buff) < 0) {
            log_errno("error in read");
            return ERROR;
        }
        Array forget = array_create(10, str_copy, str_delete, NULL);
        concurrent_avl_map(srv_info->peers, substract_tolerance, (void*) forget);
        array_map(forget, forget_peers, (void*) srv_info->peers);
        array_destroy(forget);
        return TIMEOUT_OR_BROADCAST;   
        break;

    case UPDATE_SHARED_FILES_TIMEOUT:
    if (read(fd->fd_data->integer, (void*) &buff, sizeof buff) < 0) {
        log_errno("error in read");
        return ERROR;
    }
        update_shared_files(srv_info->files);
        return TIMEOUT_OR_BROADCAST;
        break;
    
    default:
        break;
    }
    return ERROR;
}

void* wait_events(void* _arg) {
    wait_for_events_thread_arg arg = _arg;
    wait_epoll_events(arg->epfd, arg->srv_info, main_handler);
    return NULL;
}

int start_node(int srv_port, char* ip, int broadcast_port, char* broadcast_ip, char* srv_name, Array dirs) {
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

    srv_info->peers = concurrent_avl_create(peer_copy, peer_compare, peer_delete, peer_print);


    shared_files sf = get_shared_files(dirs);
    srv_info->files = sf;

    char* hello_msg = malloc(sizeof(char) * 1024);
    snprintf(hello_msg, 1024, "HELLO %s %s %d", srv_info->srv_name, srv_info->srv_ip, srv_info->srv_port);
    hello_msg = realloc(hello_msg, sizeof(char) * (strlen(hello_msg) + 1));
    srv_info->hello_msg = hello_msg;

    int ret;
    ret = create_hello_timeout(epfd);
    if (ret < 0) {
        log_error("unable to create hello timeout");
        return -1;
    }
    ret = create_cleanup_timeout(epfd);
    if (ret < 0) {
        log_error("unable to create cleanup timeout");
        return -1;
    }

    ret = create_update_shared_files_timeout(epfd);
    if (ret < 0) {
        log_error("unable to create update shared files timeout");
        return -1;
    }

    cli_args cli_args = malloc(sizeof(struct _cli_args));
    cli_args->peers = srv_info->peers;
    cli_args->files = srv_info->files;
    pthread_t cli_thread;

    pthread_t wait_events_threads[NUM_THREADS];

    struct _wait_for_events_thread_arg arg;
    arg.epfd = epfd;
    arg.srv_info = srv_info;

    log_info("starting working threads");
    for (int i = 0; i < NUM_THREADS; i++) {
        ret = pthread_create(&wait_events_threads[i], NULL, wait_events, &arg);
        if (ret != 0) {
            log_errno("error with pthread_create");
            return -1;
        }
    }

    log_info("created %d working threads", NUM_THREADS);

    pthread_create(&cli_thread, NULL, start_cli, cli_args);
    pthread_detach(cli_thread);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(wait_events_threads[i], NULL);
    }


    close(srvSocket);
    close(udpSocket);
    close(epfd);

    return 0;

}
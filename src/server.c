#include "server.h"

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

#include "network.h"
#include "utils.h"
#include "log.h"
#include "array.h"
#include "str.h"
#include "peer.h"
#include "avl_concurrent.h"
#include "files.h"
#include "cli.h"


handler_status_t file_transfer(fd_info fd) {
    int nbytes;
    if (fd->fd_data->trans_info->chunk_amount_sent == fd->fd_data->trans_info->chunk_len) {
        nbytes = read(fd->fd_data->trans_info->file_fd,
                      fd->fd_data->trans_info->chunk_buffer,
                      fd->fd_data->trans_info->chunk_len);

        fd->fd_data->trans_info->chunk_amount_sent = FILE_TRANSFER_CHUNK_SIZE - nbytes;
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

    log_info("Sent %d bytes through file transfer when a shoulf have sent", nbytes);
    
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return DOWNLOAD_REQUEST;
        else {
            log_errno("error with send (error when sending chunk of a file)");
            return ERROR;
        }
    }
    fd->fd_data->trans_info->chunk_amount_sent += nbytes;
    return DOWNLOAD_REQUEST;
}

/**
 * Function to be used in see_file_request for mapping the avl tree of files.
 */
void* send_file_name(void* value, void* context) {
    file_info file = (file_info) value;
    fd_info fd = (fd_info) context;

    char msg[1024];
    snprintf(msg, 1024, "%s %zu", file->name, file->size);

    u_int16_t msg_len = strlen(msg);
    u_int16_t msg_len_network_order = htons(msg_len);
    send_tcp_message(fd->fd_data->integer, (char*) &msg_len_network_order, HEADER_LENGTH);
    send_tcp_message(fd->fd_data->integer, msg, msg_len);
    return value;
}

handler_status_t see_files_request(fd_info fd, conc_AVL files) {
    concurrent_avl_map(files, send_file_name, (void*) fd);
    u_int16_t end = END_OF_REQUEST;
    send_tcp_message(fd->fd_data->integer, (char*) &end, HEADER_LENGTH);
    return CLIENT_CLOSE_CONNECTION;
}



handler_status_t download_request(fd_info fd, conc_AVL files, char* filename) {
    struct _file_info temp;
    temp.name = filename;
    file_info file = concurrent_avl_search(files, &temp);
    if (file == NULL) {
        uint8_t not_found = FILE_NOT_FOUND_CODE;
        send_tcp_message(fd->fd_data->integer, (char*) &not_found, sizeof not_found);
        return CLIENT_CONTINUE_CONNECTION;
    }

    // we inform such file exists and its size, so the client knows how much to read.
    
    uint8_t found = FILE_FOUND_CODE;
    send_tcp_message(fd->fd_data->integer, (char*) &found, sizeof found);

    // the client knows the next 32 bits are the size of the file, so no need for header.
    uint32_t size_network_order = htonl(file->size);
    send_tcp_message(fd->fd_data->integer, (char*) &size_network_order, sizeof size_network_order);

    char path[1024];
    snprintf(path, 1024, "%s", file->path);
    int file_fd = open(path, O_RDONLY);

    transfer_info trans = malloc(sizeof (struct _transfer_info));
    trans->client_fd = fd->fd_data->integer;

    trans->file_fd = file_fd;
    trans->chunk_amount_sent = 0;
    trans->chunk_len = FILE_TRANSFER_CHUNK_SIZE;
    fd->fd_data = malloc(sizeof(union _fd_data));
    fd->fd_data->trans_info = trans;
    fd->type = FILE_TRANSFER;
    return DOWNLOAD_REQUEST;  
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
        nbytes = recv_tcp_message(fd->fd_data->integer, buffer, msg_len);
        if (nbytes <= 0)
            return CLIENT_CLOSE_CONNECTION;
        buffer[msg_len] = '\0';

        arr = parse_input(buffer, " ");
        if (arr == NULL)
            return CLIENT_CONTINUE_CONNECTION;
        
        
        if (array_size(arr) == 2 && !strcmp(array_idx(arr, 0), "DOWNLOAD_REQUEST")) {
            log_info("download request received");
            handler_status_t ret = download_request(fd, srv_info->files, array_idx(arr,1));
            array_destroy(arr);
            return ret;
        }

        else if (array_size(arr) == 1 && !strcoll(array_idx(arr, 0), PEEK_REQUEST_MSG)) {
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
            p.tolerance = 0;
            log_info("received hello from %s %s %d", name, ip, port);
            concurrent_avl_insert(srv_info->peers, (void*) &p);
        }
        array_destroy(arr);
        return TIMEOUT_OR_BROADCAST;
        break;

    case FILE_TRANSFER:
        /* transfer a chunk */
        if (events & EPOLLOUT)
           return file_transfer(fd);

        /* client sent something, probably an error during transfer on his behalf */
        else if (events & EPOLLIN) {

        }
        
        break;

    case SEND_HELLO_TIMEOUT:
        u_int64_t buff;
        if (read(fd->fd_data->integer, (void*) &buff, 8) < 0) {
            log_errno("error in read");
            return ERROR;
        }

        int attempts = 0;
        send_hello_msg:
        int ret = send_udp_mesage(srv_info->udp_socket,
                                  srv_info->hello_msg,
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
        read(fd->fd_data->integer, (void*) &buff, 8);
        return TIMEOUT_OR_BROADCAST;   
        break;

    default:
        break;
    }
    return ERROR;
}


int start_node(int srv_port, char* ip, int broadcast_port, char* broadcast_ip, char* srv_name, char* share_dir) {
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

    srv_info->files = get_files(share_dir);
    if (!srv_info->files)
        return -1;

    printf("share folder read. Here are all the shareds files:\n");
    concurrent_avl_print(srv_info->files);

    char* hello_msg = malloc(sizeof(char) * 1024);
    snprintf(hello_msg, 1024, "HELLO %s %s %d", srv_info->srv_name, srv_info->srv_ip, srv_info->srv_port);
    hello_msg = realloc(hello_msg, sizeof(char) * (strlen(hello_msg) + 1));
    srv_info->hello_msg = hello_msg;

    create_hello_timeout(epfd);
    create_cleanup_timeout(epfd);


    cli_args cli_args = malloc(sizeof(struct _cli_args));
    cli_args->peers = srv_info->peers;
    cli_args->files = srv_info->files;
    pthread_t cli_thread;
    pthread_create(&cli_thread, NULL, start_cli, cli_args);

    wait_epoll_events(epfd, srv_info, main_handler);
    close(srvSocket);
    close(udpSocket);
    close(epfd);

    return 0;

}
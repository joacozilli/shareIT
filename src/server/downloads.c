#include "server/downloads.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server/protocol.h"
#include "events.h"
#include "files.h"
#include "network.h"
#include "log.h"


handler_status_t file_transfer(fd_info fd) {
    int nbytes;
    if (fd->fd_data->trans_info->chunk_amount_sent == fd->fd_data->trans_info->chunk_len) {
        nbytes = read(fd->fd_data->trans_info->file_fd,
                      fd->fd_data->trans_info->chunk_buffer,
                      FILE_TRANSFER_CHUNK_SIZE);

        fd->fd_data->trans_info->chunk_amount_sent = 0;
        fd->fd_data->trans_info->chunk_len = nbytes;
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
            log_errno("error with send (error when sending chunk of a file)");
            return ERROR;
        }
    }
    fd->fd_data->trans_info->chunk_amount_sent += nbytes;
    return DOWNLOAD_REQUEST;
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
    trans->chunk_len = 0;
    fd->fd_data = malloc(sizeof(union _fd_data));
    fd->fd_data->trans_info = trans;
    fd->type = FILE_TRANSFER;
    return DOWNLOAD_REQUEST;  
}
#include "server/peeks.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>

#include "server/protocol.h"
#include "events.h"
#include "files.h"
#include "network.h"
#include "log.h"

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

    /**
     * Internally, client asks to peek a specific directory. The server sends info of such
     * directory's files and the name of the subdirectories.
     * When client cd to a subdirectory, repeat.
     */

    concurrent_avl_map(files, send_file_name, (void*) fd);
    u_int16_t end = END_OF_REQUEST;
    send_tcp_message(fd->fd_data->integer, (char*) &end, HEADER_LENGTH);
    return CLIENT_CLOSE_CONNECTION;
}
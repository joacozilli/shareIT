#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <stdint.h>
#include "avl_concurrent.h"

#define EPOLL_WAIT_MAX_EVENTS 1000  // max events returned by epoll_wait
#define SEND_HELLO_TIMEOUT_SEC 2    // timeout for broadcasting hello message
#define CLEANUP_TIMEOUT_SEC 2       // timeout for removing tolerance of all peers

#define MAX_TOLERANCE 3

#define FILE_TRANSFER_CHUNK_SIZE 1024 // files are transfered by chunks of this size of bytes

/* Type returned by handler function passed to wait_epoll_events. Describes what the handler has done
and what should be done with the file descriptor after handling the event. */
typedef enum {
    CLIENT_CONTINUE_CONNECTION,   // the event was from a client, it was completed and the client will continue the connection.
    CLIENT_CLOSE_CONNECTION,      // the event was from a client, it was completed and the client closed connection.
    DOWNLOAD_REQUEST,             // the event was a download request (either new or in progress already). Rearm file descriptor into epoll appropriately.
    TIMEOUT_OR_BROADCAST,         // the event was a timeout or udp broadcast. Rearm socket normally.
    ERROR,                        // critical error when handling event, close and remove file descriptor.
} handler_status_t;


/* describes what type is a file descriptor. */
typedef enum {
    SOCKET_TCP_LISTENER, 
    SOCKET_TCP_CLIENT,
    FILE_TRANSFER,
    SOCKET_UDP,
    SEND_HELLO_TIMEOUT,
    CLEANUP_TIMEOUT
} fd_type;


struct _transfer_info {
    int client_fd;
    int file_fd;

    char chunk_buffer[FILE_TRANSFER_CHUNK_SIZE];
    int chunk_len;
    int chunk_amount_sent;
};
typedef struct _transfer_info* transfer_info;


union _fd_data {
    int integer;
    transfer_info trans_info;
};
typedef union _fd_data* fd_data;


struct _fd_info {
    fd_type type;
    fd_data fd_data;
};
/* Relevant information of a file descriptor monitored by epoll.
If the type is FILE_TRANSFER, fd_data will be a transfer_info type.
In other case, fd_data is simply integer. */
typedef struct _fd_info* fd_info;


struct _server_info {
    char* srv_name;
    int srv_socket;
    int udp_socket;
    char* srv_ip;
    int srv_port;
    char* broadcast_ip;
    int broadcast_port;
    char* hello_msg;
    conc_AVL peers;
};

typedef struct _server_info* server_info;

/**
 * create epoll instance and add the server tcp socket and udp socket to it. Return the epoll
 * file descriptor on success, otherwise return -1 and inform error.
 */
int create_srv_epoll(int srvSock, int udpSock);

/**
 * accept the new connection and add the client to the epoll instance. Return 0 on success,
 * otherwise return -1 and inform error.
 */
int accept_client_connection(int epfd, int srvSock);

/**
 * wait for epoll events. If the ready file descriptor is the server, a new connection is accepted;
 * otherwise, it is passed to the handler function.
 */
int wait_epoll_events(int epfd, server_info srv_info, handler_status_t (*handler)(fd_info fd, uint32_t events, server_info srv_info));

/**
 * Create file descriptor for hello timeout with timerfd_create and add it to the epoll instance.
 * Return 0 on success, otherwise -1 and inform error.
 */
int create_hello_timeout(int epfd);

/**
 * Create file descriptor for cleanup timeout with timerfd_create and add it to the epoll instance.
 * Return 0 on success, otherwise -1 and inform error.
 */
int create_cleanup_timeout(int epfd);

#endif /* __EVENTS_H__ */
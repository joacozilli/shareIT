#ifndef __EVENTS_H__
#define __EVENTS_H__



#define EPOLL_WAIT_MAX_EVENTS 1000 // max events returned by epoll_wait
#define RECV_TIMEOUT_SEC 3 // timeout for calling recv over a client socket (in seconds).


/**
 * describes if a client closes connection or continues after handling its request.
 */
typedef enum {
    CONTINUE,
    CLOSE
} connection_status;

/* describes what type is a file descriptor */
typedef enum {
    SOCKET_TCP_LISTENER, 
    SOCKET_TCP_CLIENT,
    SOCKET_UDP,
    SEND_HELLO_TIMEOUT,
    CLEANUP_TIMEOUT
} fd_type;


struct _fd_info {
    int fd;
    fd_type type
};
typedef struct _fd_info* fd_info;

/**
 * create epoll instance and add the server socket to it. Return the epoll file descriptor
 * on success, otherwise return -1 and inform error.
 */
int create_srv_epoll(int srvSock);

/**
 * accept the new connection and add the client to the epoll instance. Return 0 on success,
 * otherwise return -1 and inform error.
 */
int accept_client_connection(int epfd, int srvSock);

/**
 * wait for epoll events. If the ready file descriptor is the server, a new connection is accepted;
 * if it is a client, it is passed to the handler function to handle its request.
 */
int wait_epoll_events(int epfd, connection_status (*handler)(int clientFd));

#endif /* __EVENTS_H__ */
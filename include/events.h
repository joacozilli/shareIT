#ifndef __EVENTS_H__
#define __EVENTS_H__



#define EPOLL_WAIT_MAX_EVENTS 1000  // max events returned by epoll_wait
#define RECV_TIMEOUT_SEC 3          // timeout for calling recv over a client socket (in seconds).
#define SEND_HELLO_TIMEOUT_SEC 2    // timeout for broadcasting hello message
#define CLEANUP_TIMEOUT_SEC 2       // timeout for removing tolerance of all peers

#define MAX_TOLERANCE 3


/* Type returned by handler function passed to wait_epoll_events. Describes what has the handler done
and what should be done with the file descriptor after handling the event. */
typedef enum {
    CLIENT_CONTINUE_CONNECTION, // the event was from a client and it will continue the connection
    CLIENT_CLOSE_CONNECTION,    // the event was from a client and it closed connection
    TIMEOUT_DONE,               // the event was a timeout
    ERROR                       // critical error when handling event, close and remove file descriptor
} handler_status_t;

/* describes what type is a file descriptor. */
typedef enum {
    SOCKET_TCP_LISTENER, 
    SOCKET_TCP_CLIENT,
    SOCKET_UDP,
    SEND_HELLO_TIMEOUT,
    CLEANUP_TIMEOUT
} fd_type;


struct _fd_info {
    int integer;
    fd_type type;
};
typedef struct _fd_info* fd_info;

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
int wait_epoll_events(int epfd, handler_status_t (*handler)(fd_info fd));

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
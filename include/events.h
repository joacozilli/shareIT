#ifndef __EVENTS_H__
#define __EVENTS_H__


/**
 * max events returned by epoll_wait
 */
#define EPOLL_WAIT_MAX_EVENTS 1000


/**
 * timeout for calling recv over a client socket (in seconds).
 */
#define RECV_TIMEOUT_SEC 3


/**
 * describes if client closes connection or continues after handling its request.
 */
typedef enum {
    CONTINUE,
    CLOSE
} connection_status;


typedef enum {
    SOCKET_TCP_LISTENER, 
    SOCKET_TCP_CLIENT,
    SOCKET_UDP
} socket_type;


struct _SocketInfo {
    int fd;
    socket_type type;
};
/*
socket informaton:
    - file descriptor
    - type
*/
typedef struct _SocketInfo* SocketInfo;

/**
 * create epoll instance and add the server socket to it. Return the epoll file descriptor
 * on success, otherwise return -1.
 */
int create_srv_epoll(int srvSock);

/**
 * accept the new connection and add the client to the epoll instance. Return 0 on success,
 * otherwise return -1.
 */
int accept_client_connection(int epfd, int srvSock);

/**
 * wait for epoll events. If the ready file descriptor is the server, a new connection is accepted;
 * if it is a client, it is passed to the handler function to handle its request.
 */
void wait_epoll_events(int epfd, connection_status (*handler)(int clientFd));

#endif /* __EVENTS_H__ */
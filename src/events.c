#include "events.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>




int create_srv_epoll(int srvSock) {
    /**
     * the listening socket (server socket) must be added with EPOLLIN event (a file descriptor
     * with EPOLLIN event means epoll_wait will notify when it is ready for read operations).
     * When a client tries to connect, epoll_wait will notify it and thus the listener can
     * accept and add the client's file descriptor to the epoll instance.
     *
     */

    int epfd = epoll_create1(0);
    if (epfd < 0)
    {
        errnoprintf("epoll_create1 in %s", __func__);
        return -1;
    }

    fd_info server = malloc(sizeof(struct _fd_info));
    server->fd = srvSock;
    server->type = SOCKET_TCP_LISTENER;
    struct epoll_event srvEvent;
    srvEvent.data.ptr = server;
    srvEvent.events = EPOLLIN | EPOLLONESHOT; // prevents several threads accept the same connection

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, srvSock, &srvEvent) < 0)
    {
        close(epfd);
        errnoprintf("epoll_ctl in %s", __func__);
        return -1;
    }

    return epfd;
}

int accept_client_connection(int epfd, int srvSock) {
    int clientfd = accept(srvSock, NULL, NULL);
    if (clientfd < 0){
        errnoprintf("accept in %s", __func__);
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT_SEC;
    tv.tv_usec = 0;

    if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) < 0) {
        errnoprintf("setsockopt in %s", __func__);
        close(clientfd);
        return -1;
    }

    fd_info sockInf = malloc(sizeof(struct _fd_info));
    sockInf->fd = clientfd;
    sockInf->type = SOCKET_TCP_CLIENT;
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLONESHOT;
    event.data.ptr = sockInf;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event) < 0) {
        free(sockInf);
        close(clientfd);
        errnoprintf("epoll_ctl in %s (unable to add client to epoll)", __func__);
        return -1;
    }
    return 0;
}

int wait_epoll_events(int epfd, connection_status (*handler)(int clientFd)) {
    struct epoll_event eventsQueue[EPOLL_WAIT_MAX_EVENTS];
    int eventsReady = epoll_wait(epfd, eventsQueue, EPOLL_WAIT_MAX_EVENTS, -1);
    if (eventsReady < 0)
    {
        errnoprintf("epoll_wait in %s", __func__);
        return -1;
    }
    for (int i = 0; i < eventsReady; i++)
    {
        fd_info socket = eventsQueue[i].data.ptr;

        if (socket->type == SOCKET_TCP_LISTENER)
        {
            accept_client_connection(epfd, socket->type);
            struct epoll_event srvEvent;
            srvEvent.data.ptr = socket;
            srvEvent.events = EPOLLIN | EPOLLONESHOT;
            epoll_ctl(epfd, EPOLL_CTL_MOD, socket->fd, &srvEvent);
            continue;
        }

        /**
         * handle client and rearm to epoll.
         */
        connection_status status = handler(socket->fd);

        if (status == CONTINUE)
        {
            struct epoll_event cliEvent;
            cliEvent.data.ptr = socket;
            cliEvent.events = EPOLLIN | EPOLLONESHOT;

            /* if unable to add again to epoll, close connection (very rare though) */
            if (epoll_ctl(epfd, EPOLL_CTL_MOD, socket->fd, &cliEvent) < 0)
            {
                close(socket->fd);
                free(socket);
                errnoprintf("epoll_ctl in %s", __func__);
            }
        }
        else if (status == CLOSE)
            epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, NULL);
        close(socket->fd);
        free(socket);
    }

    return 0;
}
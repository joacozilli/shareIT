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
    if (epfd < 0) {
        errnoprintf("epoll_create1 in %s", __func__);
        return -1;
    }

    fd_info server = malloc(sizeof(struct _fd_info));
    server->integer = srvSock;
    server->type = SOCKET_TCP_LISTENER;
    struct epoll_event srvEvent;
    srvEvent.data.ptr = server;
    srvEvent.events = EPOLLIN | EPOLLONESHOT; // prevents several threads accept the same connection

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, srvSock, &srvEvent) < 0) {
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
    sockInf->integer = clientfd;
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

int wait_epoll_events(int epfd, handler_status_t (*client_handler)(fd_info fd)) {
    struct epoll_event eventsQueue[EPOLL_WAIT_MAX_EVENTS];
    int eventsReady = epoll_wait(epfd, eventsQueue, EPOLL_WAIT_MAX_EVENTS, -1);
    if (eventsReady < 0) {
        errnoprintf("epoll_wait in %s", __func__);
        return -1;
    }
    for (int i = 0; i < eventsReady; i++) {
        fd_info fd = eventsQueue[i].data.ptr;

        if (fd->type == SOCKET_TCP_LISTENER) {
            if (accept_client_connection(epfd, fd->integer) < 0)
                eprintf("unable to accept client connection in %s", __func__);
            struct epoll_event srvEvent;
            srvEvent.data.ptr = fd;
            srvEvent.events = EPOLLIN | EPOLLONESHOT;
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd->integer, &srvEvent);
            continue;
        }

        handler_status_t status = client_handler(fd);

        if (status == CLIENT_CONTINUE_CONNECTION) {
            struct epoll_event cliEvent;
            cliEvent.data.ptr = fd;
            cliEvent.events = EPOLLIN | EPOLLONESHOT;

            /* if unable to add again to epoll, close connection (very rare though) */
            if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd->integer, &cliEvent) < 0) {
                close(fd->integer);
                free(fd);
                errnoprintf("epoll_ctl in %s", __func__);
            }
        }
        else if (status == CLIENT_CLOSE_CONNECTION) {
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd->integer, NULL);
            close(fd->integer);
            free(fd);
        }      
        else if (status == TIMEOUT_DONE) {
            struct epoll_event timeoutEvent;
            timeoutEvent.data.ptr = fd;
            timeoutEvent.events = EPOLLIN | EPOLLONESHOT;
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd->integer, &timeoutEvent);
        }
        else if (status == ERROR) {
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd->integer, NULL);
            close(fd->integer);
            free(fd);
        }
    }

    return 0;
}
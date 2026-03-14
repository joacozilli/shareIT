#define _POSIX_C_SOURCE 199309L
#include "events.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/fcntl.h>



int create_srv_epoll(int srvSock, int udpSock) {
    /**
     * the listening socket (server socket) must be added with EPOLLIN event (a file descriptor
     * with EPOLLIN event means epoll_wait will notify when it is ready for read operations).
     * When a client tries to connect, epoll_wait will notify it and thus the listener can
     * accept and add the client's file descriptor to the epoll instance.
     *
     */

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        log_errno("error in epoll_create1");
        return -1;
    }

    fd_info server = malloc(sizeof(struct _fd_info));
    server->fd_data = malloc(sizeof(union _fd_data));
    server->fd_data->integer = srvSock;
    server->type = SOCKET_TCP_LISTENER;
    struct epoll_event srvEvent;
    srvEvent.data.ptr = server;
    srvEvent.events = EPOLLIN | EPOLLONESHOT; // prevents several threads accept the same connection

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, srvSock, &srvEvent) < 0) {
        close(epfd);
        log_errno("error in epoll_ctl");
        return -1;
    }

    fd_info udp = malloc(sizeof(struct _fd_info));
    udp->fd_data = malloc(sizeof(union _fd_data));
    udp->fd_data->integer = udpSock;
    udp->type = SOCKET_UDP;
    struct epoll_event udpEvent;
    udpEvent.data.ptr = udp;
    udpEvent.events = EPOLLIN | EPOLLONESHOT;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, udpSock, &udpEvent) < 0) {
        close(epfd);
        log_errno("error in epoll_ctl");
        return -1;
    }

    return epfd;
}


int accept_client_connection(int epfd, int srvSock) {
    int clientfd = accept(srvSock, NULL, NULL);
    if (clientfd < 0){
        log_errno("error in accept");
        return -1;
    }
    
    int flags = fcntl(clientfd, F_GETFL, 0);
    fcntl(clientfd, F_SETFL, flags | O_NONBLOCK);

    fd_info sockInf = malloc(sizeof(struct _fd_info));
    sockInf->fd_data = malloc(sizeof(union _fd_data));
    sockInf->fd_data->integer = clientfd;
    sockInf->type = SOCKET_TCP_CLIENT;
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLONESHOT;
    event.data.ptr = sockInf;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event) < 0) {
        free(sockInf);
        close(clientfd);
        log_errno("error in epoll_ctl");
        return -1;
    }
    return 0;
}


int wait_epoll_events(int epfd, server_info srv_info, handler_status_t (*handler)(fd_info fd, uint32_t events, server_info srv_info)) {
    while (1) {
        struct epoll_event eventsQueue[EPOLL_WAIT_MAX_EVENTS];
        int eventsReady = epoll_wait(epfd, eventsQueue, EPOLL_WAIT_MAX_EVENTS, -1);
        if (eventsReady < 0) {
            log_errno("error in epoll_wait");
            return -1;
        }
        for (int i = 0; i < eventsReady; i++) {
            fd_info fd = eventsQueue[i].data.ptr;
            
            if (fd->type == SOCKET_TCP_LISTENER) {
                if (accept_client_connection(epfd, fd->fd_data->integer) < 0)
                    log_error("unable to accept client connection");
                struct epoll_event srvEvent;
                srvEvent.data.ptr = fd;
                srvEvent.events = EPOLLIN | EPOLLONESHOT;
                epoll_ctl(epfd, EPOLL_CTL_MOD, fd->fd_data->integer, &srvEvent);
                log_info("new client connection accepted");
                continue;
            }

            handler_status_t status = handler(fd, eventsQueue[i].events, srv_info);

            switch (status) {
            case  CLIENT_CONTINUE_CONNECTION:
                struct epoll_event cliEvent;
                cliEvent.data.ptr = fd;
                cliEvent.events = EPOLLIN | EPOLLONESHOT;

                /* if unable to add again to epoll, close connection (very rare though) */
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd->fd_data->integer, &cliEvent) < 0) {
                    close(fd->fd_data->integer);
                    free(fd);
                    log_errno("error in epoll_ctl");
                }
                break;
            
            case CLIENT_CLOSE_CONNECTION:
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd->fd_data->integer, NULL);
                close(fd->fd_data->integer);
                free(fd);   
                break;
            
            case DOWNLOAD_REQUEST:
                struct epoll_event downloadEvent;
                downloadEvent.data.ptr = fd;
                downloadEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd->fd_data->trans_info->client_fd, &downloadEvent) < 0) {
                    close(fd->fd_data->trans_info->client_fd);
                    close(fd->fd_data->trans_info->file_fd);
                    free(fd->fd_data->trans_info);
                    free(fd);
                    log_errno("error in epoll_ctl");
                }
                break;
            
            case TIMEOUT_OR_BROADCAST:
                struct epoll_event event;
                event.data.ptr = fd;
                event.events = EPOLLIN | EPOLLONESHOT;
                epoll_ctl(epfd, EPOLL_CTL_MOD, fd->fd_data->integer, &event);
                break;
            
            default:
                break;
            }
    
        }
    }

    return 0;
}


int create_hello_timeout(int epfd) {
    if (epfd < 0) {
        log_error("file descriptor %d is non-positive", epfd);
        return -1;
    }

    int hellofd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (hellofd < 0) {
        log_errno("error in timerfd_create");
        return -1;
    }

    struct itimerspec tv;
    tv.it_interval.tv_sec = SEND_HELLO_TIMEOUT_SEC;
    tv.it_interval.tv_nsec = 0;
    tv.it_value.tv_sec = SEND_HELLO_TIMEOUT_SEC;
    tv.it_value.tv_nsec = 0;

    if (timerfd_settime(hellofd, 0, &tv, NULL) < 0) {
        log_errno("error in timerfd_settime");
        close(hellofd);
        return -1;
    }

    fd_info hello = malloc(sizeof (struct _fd_info));
    hello->fd_data = malloc(sizeof(union _fd_data));
    hello->fd_data->integer = hellofd;
    hello->type = SEND_HELLO_TIMEOUT;

    struct epoll_event helloEvent;
    helloEvent.data.ptr = hello;
    helloEvent.events = EPOLLIN | EPOLLONESHOT;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, hellofd, &helloEvent) < 0) {
        log_errno("error in epoll_ctl");
        close(hellofd);
        free(hello);
        return -1;
    }

    return 0;
}

int create_cleanup_timeout(int epfd) {
    if (epfd < 0) {
        log_error("file descriptor %d is non-positive", epfd);
        return -1;
    }

    int cleanupfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (cleanupfd < 0) {
        log_errno("error in timerfd_create");
        return -1;
    }

    struct itimerspec tv;
    tv.it_interval.tv_sec = CLEANUP_TIMEOUT_SEC;
    tv.it_interval.tv_nsec = 0;
    tv.it_value.tv_sec = CLEANUP_TIMEOUT_SEC;
    tv.it_value.tv_nsec = 0;

    if (timerfd_settime(cleanupfd, 0, &tv, NULL) < 0) {
        log_errno("error in timerfd_settime");
        close(cleanupfd);
        return -1;
    }

    fd_info cleanup = malloc(sizeof (struct _fd_info));
    cleanup->fd_data = malloc(sizeof(union _fd_data));
    cleanup->fd_data->integer = cleanupfd;
    cleanup->type = CLEANUP_TIMEOUT;

    struct epoll_event cleanupEvent;
    cleanupEvent.data.ptr = cleanup;
    cleanupEvent.events = EPOLLIN | EPOLLONESHOT;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, cleanupfd, &cleanupEvent) < 0) {
        log_errno("error in epoll_ctl");
        close(cleanupfd);
        free(cleanup);
        return -1;
    }

    return 0;
}
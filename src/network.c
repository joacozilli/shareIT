#include "network.h"
#include "log.h"

#include <stdio.h> /* perror */
#include <stdlib.h>
#include <unistd.h> /* close */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* htons */
#include <string.h> /* memset */
#include <errno.h>


int create_tcp_listener_socket(int port, const char *ip, unsigned int connectionLimit) {
    if (connectionLimit < 1) {
        log_error("connection limit %d is non-positive", connectionLimit);
        return -1;
    }
    if (port < 1 || port > 65535) {
        log_error("invalid port %d", port);
        return -1;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        log_errno("error with socket creation");
        return -1;
    }

    /**
     * force to use the specified address (except if there is already an active socket listening
     * in such address).
     */
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0) {
        close(fd);
        log_errno("error in setsockopt");
        return -1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (ip == NULL)
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
    else {
        int ret = inet_pton(AF_INET, ip, &sa.sin_addr);
        if (ret != 1) {
            close(fd);
            if (ret == 0)
                log_error("error in inet_piton: invalid ip %s", ip);
            if (ret == -1)
                log_errno("error in inet_piton");
            return -1;
        }
    }

    if (bind(fd, (struct sockaddr *)&sa, sizeof sa) < 0) {
        close(fd);
        log_errno("error in bind");
        return -1;
    }

    if (listen(fd, connectionLimit) < 0) {
        close(fd);
        log_errno("error in listen");
        return -1;
    }
    log_info("created tcp listener socket at port %d, ip %s", port, ip);
    return fd;
}


int create_tcp_client_socket(int port, const char* ip) {
 if (port < 1 || port > 65535) {
        log_error("invalid port %d", port);
        return -1;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        log_errno("error with socket creation");
        return -1;
    }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof srv);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    int ret = inet_pton(AF_INET, ip, &srv.sin_addr);
    if (ret != 1) {
        close(fd);
        if (ret == 0)
            log_error("error in inet_piton: invalid ip %s", ip);
        if (ret == -1)
            log_errno("error in inet_piton");
        return -1;
    }

    if (connect(fd, (struct sockaddr*) &srv, sizeof srv) < 0) {
        close(fd);
        log_errno("error in connect");
        return -1;
    }
    log_info("created tcp socket connected at port %d, ip %s", port, ip);
    return fd;
}


int create_broadcast_udp_socket(int port, const char *ip) {
    if (port < 1 || port > 65535) {
        log_error("invalid port %d", port);
        return -1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        log_errno("error with socket creation");
        return -1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (ip == NULL)
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
    else {
        int ret = inet_pton(AF_INET, ip, &sa.sin_addr);
        if (ret != 1) {
            close(fd);
            if (ret == 0)
                log_error("error in inet_piton: invalid ip %s", ip);
            else if (ret == -1)
                log_errno("error in inet_piton");
            return -1;
        }
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0) {
        close(fd);
        log_errno("error in setsockopt");
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof opt) < 0) {
        close(fd);
        log_errno("error in setsockopt");
        return -1;
    }

    if (bind(fd, (struct sockaddr*) &sa, sizeof sa) < 0) {
        close(fd);
        log_errno("error in bind");
        return -1;
    }
    log_info("created udp socket for broadcast");
    return fd;
}


int send_tcp_message(int fd, const void* msg, size_t size) {
    if (fd < 0) {
        log_error("file descriptor %d is non-positive");
        return -1;
    }

    size_t total = 0;
    while (total < size) {
        // msg_dontwait so it doesn't block
        int nbytes = send(fd, msg+total, size-total, MSG_DONTWAIT);
        if (nbytes < 0) {
            log_errno("error in send");
            return -1;
        }
        if(nbytes == 0)
            break;
        total += nbytes;
    }
    return total;
}


int recv_tcp_message(int fd, void* buffer, size_t len) {
    if (fd < 0) {
        log_error("file descriptor %d is non-positive");
        return -1;
    }

    size_t total = 0;
    while (total < len) {
        int nbytes = recv(fd, buffer+total, len - total, 0);
        if (nbytes < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                continue;
            log_errno("error in recv");
            return -1;
        }
        if (nbytes == 0) // peer closed connection orderly
            break;
        total += nbytes;
    }
    return total;
}


int send_udp_mesage(int fd, const void* msg, size_t size, int port, const char* ip) {
    if (fd < 0) {
        log_error("file descriptor %d is non-positive");
        return -1;
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest.sin_addr);

    if (sendto(fd, msg, size, 0, (struct sockaddr*) &dest, sizeof dest) < 0) {
        log_errno("error in sendto");
        return -1;
    }

    return 0;
}

int recv_udp_message(int fd, void* buffer, size_t len) {
    if (fd < 0) {
        log_error("file descriptor %d is non-positive");
        return -1;
    }

    int nbytes = recvfrom(fd, buffer, len, MSG_TRUNC, NULL, NULL);
    if (nbytes < 0) {
        log_errno("error in recvfrom");
        return -1;
    }
    return nbytes;
}

#include "network.h"
#include "utils.h"

#include <stdio.h> /* perror */
#include <stdlib.h>
#include <unistd.h> /* close */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* htons */
#include <string.h> /* memset */


int create_tcp_listener_socket(int port, const char *ip, unsigned int connectionLimit) {
    if (connectionLimit < 1) {
        eprintf("%s: connection limit %d is non-positive\n", __func__, connectionLimit);
        return -1;
    }
    if (port < 1 || port > 65535) {
        eprintf("%s: invalid port %d\n", __func__, port);
        return -1;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        errnoprintf("socket in %s", __func__);
        return -1;
    }

    /**
     * force to use the specified address (except if there is already an active socket listening
     * in such address).
     */
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0) {
        close(fd);
        errnoprintf("setsockopt in %s", __func__);
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
                eprintf("inet_pton in %s: invalid ip %s\n", __func__, ip);
            else if (ret == -1)
                errnoprintf("inet_pton in %s", __func__);
            return -1;
        }
    }

    if (bind(fd, (struct sockaddr *)&sa, sizeof sa) < 0) {
        close(fd);
        errnoprintf("bind in %s", __func__);
        return -1;
    }

    if (listen(fd, connectionLimit) < 0) {
        close(fd);
        errnoprintf("listen in %s", __func__);
        return -1;
    }
    return fd;
}


int create_tcp_client_socket(int port, const char* ip) {
 if (port < 1 || port > 65535) {
        eprintf("%s: invalid port %d\n", __func__, port);
        return -1;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        errnoprintf("socket in %s", __func__);
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
            eprintf("inet_pton in %s: invalid ip %s\n", __func__, ip);
        else if (ret == -1)
            errnoprintf("inet_pton in %s", __func__);
        return -1;
    }

    if (connect(fd, (struct sockaddr*) &srv, sizeof srv) < 0) {
        close(fd);
        errnoprintf("connect in %s", __func__);
        return -1;
    }

}


int create_broadcast_udp_socket(int port, const char *ip) {
    if (port < 1 || port > 65535) {
        eprintf("%s: invalid port %d,\n", __func__, port);
        return -1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        errnoprintf("socket in %s", __func__);
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
                eprintf("inet_pton in %s: invalid ip %s\n", __func__, ip);
            else if (ret == -1)
                errnoprintf("inet_pton in %s", __func__);
            return -1;
        }
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0) {
        close(fd);
        errnoprintf("setsockopt in %s", __func__);
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof opt) < 0) {
        close(fd);
        errnoprintf("setsockopt in %s", __func__);
        return -1;
    }

    if (bind(fd, (struct sockaddr*) &sa, sizeof sa) < 0) {
        close(fd);
        errnoprintf("bind in %s", __func__);
        return -1;
    }

    return fd;
}


int send_tcp_message(int fd, const void* msg, size_t size) {
    int total = 0;
    while (total < size) {
        // blocks if receiver isn't reading (must fix!!!)
        int nbytes = send(fd, msg+total, size-total, 0);
        if (nbytes < 0) {
            errnoprintf("send in %s", __func__);
            return -1;
        }
        if (nbytes == 0) {
            // receiver closed connection gracefully.
        }
        total += nbytes;
    }
    return 0;
}



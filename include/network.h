#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stddef.h>

/**
 * Create new tcp socket listening on the specified port and ip. if ip is NULL, bind
 * to all interfaces. Return the file descriptor of the socket on success, otherwise 
 * return -1 and inform the error.
 */
int create_tcp_listener_socket(int port, const char* ip, unsigned int connectionLimit);

/**
 * Create new tcp socket and connect to specified port and ip as a client. Return the file descriptor
 * of the socket on success, otherwise return -1 and inform error.
 */
int create_tcp_client_socket(int port, const char* ip);

/**
 * Create new udp socket bound to the specified port and ip with broadcast permit. If ip is NULL,
 * bind to all interfaces. Return the file descriptor of the socket on success, otherwise 
 * return -1 and inform the error.
 */
int create_broadcast_udp_socket(int port, const char* ip);

/**
 * Send message through connected tcp socket. Return 0 if message was delivered successfully,
 * otherwise return -1 and inform error.
 */
int send_tcp_message(int fd, const void* msg, size_t size);


/**
 * Send message through udp socket to specified port and ip. Return 0 if message was delivered
 * successfully, otherwise return -1 and inform error.
 */
int send_udp_mesage(int fd, const void* msg, size_t size, int port, const char* ip);

#endif /* __NETWORK_H__ */
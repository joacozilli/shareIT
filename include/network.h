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
 * Send message through connected tcp socket. Return number of bytes sent if message was delivered successfully,
 * otherwise return -1 and inform error.
 */
int send_tcp_message(int fd, const void* msg, size_t size);

/**
 * Receive len bytes from tcp socket and store it in the passed buffer. The buffer must be of at least size len.
 * Return number of bytes received or -1 if there was an error and inform it.
 */
int recv_tcp_message(int fd, void* buffer, size_t len); 

/**
 * Send datagram through udp socket to specified port and ip. Return 0 if datagram was delivered
 * successfully, otherwise return -1 and inform error.
 */
int send_udp_mesage(int fd, const void* msg, size_t size, int port, const char* ip);

/**
 * Receive datagram through udp socket and store it in the passed buffer. Return length of datagram
 * or -1 if there was an error and inform it. If datagram is longer than buffer, it will be truncated.
 * To check if it was truncated, compare return value with buffer length.
 */
int recv_udp_message(int fd, void* buffer, size_t len);

#endif /* __NETWORK_H__ */
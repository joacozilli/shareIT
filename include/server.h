#ifndef __SERVER_H__
#define __SERVER_H__

#define ID_LENGTH 16

/**
 * All messages sent via tcp start with a header of this length where the length of the actual
 * message is stored. That way the receiver knows how much to read.
 */
#define HEADE_LENGTH 16



/**
 * Main handler of node. It handles clients's requests, hello messages and timeouts.
 */
handler_status_t main_handler(fd_info fd);


/**
 * Wait for events function to be executed on a working thread.
 */
void* wait_events(void* arg);


/**
 * Generate new id and broadcast a name request.
 */
int name_request(char* buff, int udpSock, int srvSock, int epfd);


/**
 * Start the execution of the node.
 */
int start_node(int srv_port, const char* srv_ip, int broadcast_port);




#endif /*  __SERVER_H__ */
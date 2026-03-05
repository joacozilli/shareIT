#ifndef __SERVER_H__
#define __SERVER_H__

/**
 * All messages sent via tcp start with a header of this length where the length of the actual
 * message is stored. That way the receiver knows how much to read.
 */
#define HEADER_LENGTH 2

/**
 * If node has an error when broadcasting a hello message, it shall try again this amount at most.
 */
#define MAX_HELLO_ATTEMPTS 3

/**
 * Main handler of node. It handles clients's requests, hello messages and timeouts.
 * broadcast hello, reading other's hello's and adding them to neiborhood, making cleanup
 * and managing search requests are done inside the handler (they are short tasks).
 * For download requests, the handler responds the client saying if the file exists or not.
 * If it does, returns an enum indicating that a new download request has started and the fd must be added
 * with the event EPOLLOUT and a data structure file_transfer_info (EPOLLIN event is kept).
 * 
 * When the client fd is ready for write, a chunk is transfered. This is done inside the handler.
 */
handler_status_t main_handler(fd_info fd, server_info srv_info);


/**
 * Wait for events function to be executed on a working thread.
 */
void* wait_events(void* arg);


/**
 * Start the execution of the node.
 */
int start_node(int srv_port, const char* srv_ip, int broadcast_port, const char* broadcast_ip, char* srv_name);




#endif /*  __SERVER_H__ */
#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>
#include "events.h"

/**
 * All messages sent via tcp that are not part of the protocol start with a header of this length
 * where the length of the actual message is stored. That way the receiver knows how much to read.
 */
#define HEADER_LENGTH 2

// if header stores this value, the receiver knows the request has been completed.
#define END_OF_REQUEST 0xffff

// this code means the file exist (used to respond client in download handling)
#define FILE_FOUND_CODE 0xff

// this code means the file doesn't exist (used to respond client in download handling)
#define FILE_NOT_FOUND_CODE 0x00

/**
 * If node has an error when broadcasting a hello message, it shall try again this amount at most.
 */
#define MAX_HELLO_ATTEMPTS 3


#define PEEK_REQUEST_MSG "PEEK_REQUEST"

/**
 * function to handle a file transfer. Called inside main_handler whenever the event is a file transfer.
 */
handler_status_t file_transfer(fd_info fd);

/**
 * function to handle see files requests. Called inside main_handler.
 */
handler_status_t see_files_request(fd_info fd, conc_AVL files);

/**
 * function to handle download requests. Called inside main_handler.
 */
handler_status_t download_request(fd_info fd, conc_AVL files, char* filename);


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
handler_status_t main_handler(fd_info fd, uint32_t events, server_info srv_info);


/**
 * Wait for events function to be executed on a working thread.
 */
void* wait_events(void* arg);


/**
 * Start the execution of the node.
 */
int start_node(int srv_port, char* srv_ip, int broadcast_port, char* broadcast_ip, char* srv_name, char* share_dir);




#endif /*  __SERVER_H__ */
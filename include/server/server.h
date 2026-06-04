#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>
#include "events.h"
#include "array.h"

#define NUM_THREADS sysconf(_SC_NPROCESSORS_ONLN)


/**
 * Main handler of node. It handles clients's requests, hello messages and timeouts.
 */
handler_status_t main_handler(fd_info fd, uint32_t events, server_info srv_info);



struct _wait_for_events_thread_arg {
    int epfd;
    server_info srv_info;
};
typedef struct _wait_for_events_thread_arg* wait_for_events_thread_arg;

/**
 * Wait for events function to be executed on a working thread.
 */
void* wait_events(void* _arg);


/**
 * Start the execution of the node.
 */
int start_node(int srv_port, char* srv_ip, int broadcast_port, char* broadcast_ip, char* srv_name, Array dirs);




#endif /*  __SERVER_H__ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/str.h"
#include "src/data_structures/array/array.h"
#include "src/data_structures/avl_tree/avl_tree.h"

#include "include/events.h"
#include "include/network.h"

// node structure would be something like this
// struct _NodeP2P {
//     char* name;
//     char ip[INET_ADDDRSTRLEN];
//     int port;
//     int counter;
// };


int main() {
    int bc_port = 12345;
    int srv_port = 60000;
    const char* pc_ip = "192.168.1.77";
    const char* broadcast = "192.168.1.255";

    int tcp_sock = create_tcp_listener_socket(6500, pc_ip, 10);
    int udp_sock = create_broadcast_udp_socket(bc_port, pc_ip);


    close(tcp_sock);
    close(udp_sock);

    return 0;
}
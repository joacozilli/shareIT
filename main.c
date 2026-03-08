#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "include/str.h"
#include "src/data_structures/array/array.h"
#include "src/data_structures/avl_tree/avl_tree.h"
#include "src/data_structures/avl_concurrent/avl_concurrent.h"

#include "events.h"
#include "network.h"
#include "server.h"


int main() {

    struct ifaddrs* ifap;

    int ret = getifaddrs(&ifap);
    if (ret < 0)
        printf("error with getifaddrs\n");
    
    for (struct ifaddrs* temp = ifap; temp != NULL; temp = temp->ifa_next) {
        struct sockaddr_in* sa = (struct sockaddr_in *) temp->ifa_addr;
        char* addr = inet_ntoa(sa->sin_addr);
        printf("Interface: %s\tAddress: %s\n", temp->ifa_name, addr);
    }

    freeifaddrs(ifap);

    int srv_port = 60000;
    char* srv_ip = "192.168.1.77";

    int broadcast_port = 12345;
    char* broadcast_ip = "192.168.1.255";

    char* srv_name = "DESK-PC";

    start_node(srv_port, srv_ip, broadcast_port, broadcast_ip, srv_name);


    // conc_AVL t = concurrent_avl_create(str_copy, str_compare, str_delete, str_print);
    // for (int i = 0; i < 100000; i++) {
    //     char buffer[250];
    //     sprintf(buffer, "str%d", i);
    //     concurrent_avl_insert(t, buffer);
    // }ds
    // concurrent_avl_print(t);
    // concurrent_avl_destroy(t);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <dirent.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "include/str.h"
#include "src/data_structures/array/array.h"
#include "src/data_structures/avl_tree/avl_tree.h"
#include "src/data_structures/avl_concurrent/avl_concurrent.h"

#include "utils.h"
#include "log.h"
#include "events.h"
#include "network.h"
#include "server.h"
#include "files.h"


int main() {
    log_file = fopen("shareit.log", "w");
    if (!log_file) {
        perror("fopen");
        return 1;
    }

    int srv_port = 60000;
    char* srv_ip = "192.168.1.77";
    int broadcast_port = 12345;
    char* broadcast_ip = "192.168.1.255";
    char* srv_name = "DESK-PC";
    start_node(srv_port, srv_ip, broadcast_port, broadcast_ip, srv_name, "./share");
    
    return 0;
}
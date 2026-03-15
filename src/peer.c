#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void* peer_copy(void* p) {
    peer pp = (peer) p;
    peer new = malloc(sizeof(struct _peer));
    new->name = malloc(sizeof(char) * (strlen(pp->name) + 1));
    strcpy(new->name, pp->name);
    new->ip = malloc(sizeof(char) * (strlen(pp->ip) + 1));
    strcpy(new->ip, pp->ip);
    new->port = pp->port;
    new->tolerance = pp->tolerance;
    return new;
}


int peer_compare(void* p1, void* p2) {
    peer pp1 = (peer) p1;
    peer pp2 = (peer) p2;
    int r = strcmp(pp1->ip, pp2->ip);
    if (r == 0) {
        if (pp1->port == pp2->port)
            return 0;
        if (pp1->port < pp2->port)
            return -1;
        return 1;
    }
    if (r < 0)
        return -1;
    return 1;
}

int peer_compare_names(void* p1, void* p2) {
    peer pp1 = (peer) p1;
    peer pp2 = (peer) p2;
    return strcmp(pp1->name, pp2->name);
}

void peer_delete(void* p) {
    peer pp = (peer) p;
    free(pp->ip);
    free(pp->name);
    free(pp);
}

void peer_print(void* p) {
    peer pp = (peer) p;
    char name_buff[NAME_SPACE];
    char ip_buff[IP_SPACE];
    char port_buff[PORT_SPACE];

    int name_len = strlen(pp->name);
    strncpy(name_buff, pp->name, name_len);
    for (int i = name_len; i < NAME_SPACE-1; name_buff[i] = ' ', i++);
    name_buff[NAME_SPACE-1] = '\0';

    int ip_len = strlen(pp->ip);
    strncpy(ip_buff, pp->ip, ip_len);
    for (int i = ip_len; i < IP_SPACE-1; ip_buff[i] = ' ', i++);
    ip_buff[IP_SPACE-1] = '\0';

    snprintf(port_buff, PORT_SPACE, "%d", pp->port);
    int port_len = strlen(port_buff);
    for (int i = port_len; i < PORT_SPACE-1; port_buff[i] = ' ', i++);
    port_buff[PORT_SPACE-1] = '\0';

    printf("name: %s | pi: %s | port: %s\n", name_buff, ip_buff, port_buff);
}
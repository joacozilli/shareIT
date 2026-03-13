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
    printf("PEER %s\n", pp->name);
    printf(" - ip: %s, port: %d\n", pp->ip, pp->port);
}
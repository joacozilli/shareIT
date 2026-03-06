#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


peer peer_copy(peer p) {
    peer new = malloc(sizeof(struct _peer));
    new->name = p->name;
    strncpy(new->ip, p->ip, INET_ADDRSTRLEN);
    new->port = p->port;
    new->tolerance = p->tolerance;
    return new;
}


int peer_compare(peer p1, peer p2) {
    int r = strcmp(p1->ip, p2->ip);
    if (r == 0) {
        if (p1->port == p2->port)
            return 0;
        if (p1->port < p2->port)
            return -1;
        return 1;
    }
    if (r < 0)
        return -1;
    return 1;
}

void peer_delete(peer p) {
    free(p);
}
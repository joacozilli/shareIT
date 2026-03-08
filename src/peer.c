#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


peer peer_copy(peer p) {
    peer new = malloc(sizeof(struct _peer));
    new->name = malloc(sizeof(char) * (strlen(p->name) + 1));
    strcpy(p->name, new->name);
    new->ip = malloc(sizeof(char) * (strlen(p->ip) + 1));
    strcpy(p->ip, new->ip);
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
    free(p->ip);
    free(p->name);
    free(p);
}

void peer_print(peer p) {
    printf("PEER %s\n", p->name);
    printf(" - ip: %s, port: %d\n", p->ip, p->name);
}
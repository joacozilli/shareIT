#ifndef __PEER_H__
#define __PEER_H__



struct _peer {
    char* name;
     char* ip;
     int port;
     int tolerance;
};
typedef struct _peer* peer;

/**
 * Return deep copy of peer.
 */
peer peer_copy(peer p);

/**
 * Return 0 if equals, < 0 if p1 < p2 and > 0 if p1 > p2.
 */
int peer_compare(peer p1, peer p2);

/**
 * delete peer from memory.
 */
void peer_delete(peer p);


#endif /* __PEER_H__ */
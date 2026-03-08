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
void* peer_copy(void* p);

/**
 * Return 0 if equals, < 0 if p1 < p2 and > 0 if p1 > p2.
 */
int peer_compare(void* p1, void* p2);

/**
 * delete peer from memory.
 */
void peer_delete(void* p);

/**
 * print peer in stdout.
 */
void peer_print(void* p);


#endif /* __PEER_H__ */
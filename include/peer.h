#ifndef __PEER_H__
#define __PEER_H__


// this is just for printing more nicely
#define NAME_SPACE 30
#define IP_SPACE 30
#define PORT_SPACE 10

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
 * The same as peer_compare, but it only compares the names.
 */
int peer_compare_names(void* p1, void* p2);

/**
 * delete peer from memory.
 */
void peer_delete(void* p);

/**
 * print peer in stdout.
 */
void peer_print(void* p);


#endif /* __PEER_H__ */
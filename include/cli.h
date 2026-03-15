#ifndef __CLI_H__
#define __CLI_H__
#include "events.h"

struct _cli_args {
    conc_AVL peers;
    conc_AVL files;
};
typedef struct _cli_args* cli_args;

typedef enum {
    HELP,           // show help message to use cli
    NEIGHBORS,      // list all known peers
    PEEK,           // see all files shared by a specific peer
    SEARCH,         // look for a specific file/pattern in all known peers
    DOWNLOAD,       // download one or more files from a specific peer
    UNDEFINED
} CMD;



void* start_cli(void* arg);


#endif
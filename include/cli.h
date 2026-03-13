#ifndef __CLI_H__
#define __CLI_H__
#include "events.h"

struct _cli_args {
    conc_AVL peers;
    conc_AVL files;
};
typedef struct _cli_args* cli_args;

typedef enum {
    HELP,       // show help message to use cli
    PEEK,       // see all files shared by a specific node
    SEARCH,     // look for a specific file/pattern in all known nodes
    DOWNLOAD,   // download one or more files from a specific node
    UNDEFINED
} CMD;



void* start_cli(void* arg);


#endif
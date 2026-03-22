#ifndef __CLI_H__
#define __CLI_H__
#include "events.h"

#define FILE_NAME_SPACE 50
#define FILE_SIZE_SPACE 30

struct _cli_args {
    conc_AVL peers;
    conc_AVL files;
};
typedef struct _cli_args* cli_args;

typedef enum {
    HELP,           // show help message to use cli
    NEIGHBORS,      // list all known peers
    PEEK,           // see all files shared by a specific peer
    SEARCH,         // search a specific file/pattern in all known peers
    DOWNLOAD,       // download one or more files from a specific peer
    UNDEFINED
} CMD;


struct _download_file_context {
    int fd;
    conc_AVL files;
};
typedef struct _download_file_context* download_file_context;

void* start_cli(void* arg);


#endif
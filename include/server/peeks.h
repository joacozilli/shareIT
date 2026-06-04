#ifndef __PEEKS_H__
#define __PEEKS_H__

#include <stdint.h>
#include "events.h"

/**
 * function to handle see files requests. Called inside main_handler.
 */
handler_status_t see_files_request(fd_info fd, conc_AVL files);


#endif
#ifndef __DOWNLOADS_H__
#define __DOWNLOADS_H__

#include <stdint.h>
#include "events.h"


/**
 * function to handle a file transfer. Called inside main_handler whenever the event is a file transfer.
 */
handler_status_t file_transfer(fd_info fd);

/**
 * function to handle download requests. Called inside main_handler.
 */
handler_status_t download_request(fd_info fd, conc_AVL files, char* filename);

#endif
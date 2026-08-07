#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


#define HEADER_LENGTH 2

// if header stores this value, the receiver knows the request has been completed.
#define END_OF_REQUEST 0xffff


#define DOWNLOAD_REQUEST_MSG "DOWNLOAD_REQUEST"

#define PEEK_REQUEST_MSG "PEEK_REQUEST"

// this code means the file exist (used to respond client in download handling)
#define FILE_FOUND_CODE 0xff

// this code means the file doesn't exist (used to respond client in download handling)
#define FILE_NOT_FOUND_CODE 0x00



#endif
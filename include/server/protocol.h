#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/**
 * All messages sent via tcp that are not part of the protocol start with a header of this length
 * where the length of the actual message is stored. That way the receiver knows how much to read.
 */
#define HEADER_LENGTH 2

// if header stores this value, the receiver knows the request has been completed.
#define END_OF_REQUEST 0xffff


#define PEEK_REQUEST_MSG "PEEK_REQUEST"

// this code means the file exist (used to respond client in download handling)
#define FILE_FOUND_CODE 0xff

// this code means the file doesn't exist (used to respond client in download handling)
#define FILE_NOT_FOUND_CODE 0x00



#endif
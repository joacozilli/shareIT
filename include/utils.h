#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>
#include <stdio.h>

#include "array.h"

/**
 * Parse string input into tokens based on delim and return array with the tokens.
 * If there is no token, return NULL;
 */
Array parse_input(char* input, char* delim);



#endif
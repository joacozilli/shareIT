#ifndef __STR_H__
#define __STR_H__

/**
 * Return deep copy of string.
 */
void* str_copy(void* str);

/**
 * Return 0 if equals, < 0 if str1 < str2 and > 0 if str1 > str2.
 */
int str_compare(void* str1, void* str2);

/**
 * Print string to stdout with newline.
 */
void str_print(void* str);

/**
 * Delete string from memory.
 */
void str_delete(void* str);


#endif
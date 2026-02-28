#ifndef __FUNCS_H__
#define __FUNCS_H__

/**
 * Must return a deep copy of value.
 */
typedef void* (*functionCopy)(void* value);

/**
 * Must return 0 if value1 and value2 are equal, otherwise non-zero value.
 */
typedef int (*functionCompareEq)(void* value1, void* value2);

/**
 * Must return 0 if value1 == value2, less than 0 if value1 < value2 or greater
 * than 0 if value1 > value2
 */
typedef int (*functionCompareOrd)(void* value1, void* value2);

/**
 * Must delete value from memory.
 */
typedef void (*functionDelete)(void* value);

/**
 * Must print value to stdout.
 */
typedef void (*functionPrint)(void* value);

/**
 * Must return value of the same type.
 */
typedef void* (*functionMap)(void* value);

/**
 * Must return a hash value for value.
 */
typedef unsigned long (*funcionHash)(void* value);

#endif /* __FUNCS_H__ */
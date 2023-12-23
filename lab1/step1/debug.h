/**
 * @file     debug.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Debugging print
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_DEBUG__H
#define __ITMO_DISTRIBUTED_CLASS_DEBUG__H

#define DEBUG 0

#define debug_print(fmt, ...)                         \
    do {                                              \
        if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); \
    } while (0)

#endif  // __ITMO_DISTRIBUTED_CLASS_DEBUG__H

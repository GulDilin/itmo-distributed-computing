/**
 * @file     args.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Command-line arguments parsing
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_ARGS__H
#define __ITMO_DISTRIBUTED_CLASS_ARGS__H

#include <argp.h>
#include <stdint.h>

/* Used by main to communicate with parse_opt. */
typedef struct {
    uint8_t proc_n;
} arguments;

/**
 * @brief      Parse arguments
 *
 * @param[in]  argc       The count of arguments
 * @param      argv       The arguments array
 * @param      arguments  The arguments result object pointer
 */
void args_parse(int argc, char **argv, arguments *arguments);

#endif  // __ITMO_DISTRIBUTED_CLASS_ARGS__H

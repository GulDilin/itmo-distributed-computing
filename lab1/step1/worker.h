/**
 * @file     worker.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Logging messages
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_WORKER__H
#define __ITMO_DISTRIBUTED_CLASS_WORKER__H

#include "channels.h"

/**
 * @brief      Child worker main logic
 *
 * @param      self  The object
 */
void child_worker(executor *self);

/**
 * @brief      Parent worker main logic
 *
 * @param      self  The executor
 */
void parent_worker(executor *self);

/**
 * @brief      Run worker based on executor
 *
 * @param      self  The executor
 */
void run_worker(executor *self);

#endif  // __ITMO_DISTRIBUTED_CLASS_WORKER__H

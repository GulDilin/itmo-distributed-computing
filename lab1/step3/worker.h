/**
 * @file     worker.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Functions for workers processes
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_WORKER__H
#define __ITMO_DISTRIBUTED_CLASS_WORKER__H

#include <stdint.h>
#include <unistd.h>

#include "banking.h"
#include "channels.h"
#include "executor.h"
#include "ipc.h"

/**
 * @brief      Child worker main logic
 *
 * @param      self  The object
 */
void account_worker(executor *self);

/**
 * @brief      Parent worker main logic
 *
 * @param      self  The executor
 */
void router_worker(executor *self);

/**
 * @brief      Run worker based on executor
 *
 * @param      self  The executor
 */
void run_worker(executor *self);

/**
 * @brief      Initializes the executor.
 *
 * @param      executor       The executor
 * @param      channels       The channels matrix
 * @param[in]  local_id       The local identifier
 * @param[in]  proc_n         The number of processes
 * @param[in]  pid            The pid of executor
 * @param[in]  p_pid          The pid of executor parent
 * @param[in]  start_balance  The start balance of bank account
 */
void init_executor(
    executor *executor, channel **channels, local_id local_id, int proc_n, pid_t pid, pid_t p_pid,
    balance_t start_balance
);

/**
 * @brief      Method that will be called on main end for each executor
 *
 * @param      executor  The executor
 */
void cleanup_executor(executor *executor);

#endif  // __ITMO_DISTRIBUTED_CLASS_WORKER__H

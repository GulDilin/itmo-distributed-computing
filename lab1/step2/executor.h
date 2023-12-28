/**
 * @file     executor.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Structures for executor process
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_EXECUTOR__H
#define __ITMO_DISTRIBUTED_CLASS_EXECUTOR__H

#include <stdint.h>
#include <unistd.h>

#include "banking.h"
#include "channels.h"
#include "ipc.h"

typedef struct {
    balance_t       balance;  ///< Bank account balance state
    BalanceHistory *history;
    AllHistory     *all_history;
} BankAccount;

typedef struct {
    local_id    local_id;      ///< Local process id (usually index of created process)
    channel_h  *ch_read;       ///< Array of reading pipe handlers
    channel_h  *ch_write;      ///< Array of writing pipe handlers
    int8_t      proc_n;        ///< Number of processes
    int8_t      is_running;    ///< Running state
    pid_t       parent_pid;    ///< Parend process id
    pid_t       pid;           ///< Executor process id
    BankAccount bank_account;  ///< Bank account connected with executor
} executor;

#endif                         // __ITMO_DISTRIBUTED_CLASS_EXECUTOR__H

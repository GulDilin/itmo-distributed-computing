/**
 * @file     executor.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Structures for executor process
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_EXECUTOR__H
#define __ITMO_DISTRIBUTED_CLASS_EXECUTOR__H

#include <stdint.h>
#include <unistd.h>

#include "channels.h"
#include "ipc.h"

typedef struct {
    local_id    local_id;  ///< Local process id (usually index of created process)
    channel_h  *ch_read;   ///< Array of reading pipe handlers
    channel_h  *ch_write;  ///< Array of writing pipe handlers
    uint8_t     proc_n;    ///< Number of processes
    uint8_t     proc_done[MAX_PROCESS_ID + 1];  ///< Info which processes are done
    uint8_t     is_self_done;                   ///< Info which processes are done
    uint8_t     all_done;                       ///< Info which processes are done
    uint8_t     use_lock;                       ///< Info which processes are done
    pid_t       parent_pid;                     ///< Parend process id
    pid_t       pid;                            ///< Executor process id
    timestamp_t last_recv_at[MAX_PROCESS_ID + 1];
    timestamp_t last_send_at[MAX_PROCESS_ID + 1];
} executor;

/**
 * @brief      Called on message.
 *
 * @param      self  The executor
 * @param      msg   The message
 * @param[in]  from  The from process local id
 */
void on_message(executor *self, Message *msg, local_id from);

#endif  // __ITMO_DISTRIBUTED_CLASS_EXECUTOR__H

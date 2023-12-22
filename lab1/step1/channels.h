/**
 * @file     channels.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Communication structs and methods for pipes
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_CHANNELS__H
#define __ITMO_DISTRIBUTED_CLASS_CHANNELS__H

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

#include "ipc.h"

typedef int channel_h;

typedef struct {
    channel_h read_h;     ///< read handler for pipe
    channel_h write_h;    ///< write handler for pipe
} channel;

typedef struct {
    local_id local_id;          ///< Local process id (usually index of created process)
    channel_h *ch_read;         ///< Array of reading pipe handlers
    channel_h *ch_write;        ///< Array of writing pipe handlers
    int8_t proc_n;              ///< Number of processes
    pid_t parent_pid;           ///< Parend process id
    pid_t pid;                  ///< Executor process id
} executor;

#define SLEEP_RECEIVE_USEC 10 // 10 usec between recieve any msg

// int nanosleep(const struct timespec *req, struct timespec * rem);

/**
 * @brief      Sleep us
 *
 * @param[in]  microseconds  The microseconds
 */
void sleep_us(unsigned long microseconds);

/**
 * @brief      init a communication channel.
 *
 * @param      channel  The channel pointer
 *
 * @return     0 on success, any non-zero value on error
 */
int init_channel(channel * channel);

/**
 * @brief      Opens a channel between processes.
 *
 * @param      channels  The channels
 * @param[in]  from      The from process local id
 * @param[in]  dst       The destination process local id
 *
 * @return     0 on success, any non-zero value on error
 */
int open_channel(channel **channels, local_id from, local_id dst);

/**
 * @brief      Opens channels. Channels matrix structure (row - src, col - dst):
 *
 * 1 -  |   -    | 1 -> 2 | 1 -> 3 |
 * 2 -  | 2 -> 1 |   -    | 2 -> 3 |
 * 3 -  | 3 -> 1 | 3 -> 2 |   -    |
 *
 *
 * @param[in]  proc_n    The number of processes
 * @param      channels  The channels matrix
 *
 * @return     0 on success, any non-zero value on error
 */
int open_channels(int8_t proc_n, channel **channels);

/**
 * @brief      Sets the executor channels.
 *
 * @param[in]  proc_n    The proc n
 * @param      executor  The executor
 * @param      channels  The channels
 */
void set_executor_channels(int8_t proc_n, executor *executor, channel **channels);

/**
 * @brief      Closes unused channels.
 *
 * @param[in]  proc_n    The proc n
 * @param[in]  local_id  The local identifier
 * @param      channels  The channels
 *
 * @return     0 on success, any non-zero value on error
 */
int close_unused_channels(int8_t proc_n, local_id local_id, channel **channels);

/**
 * @brief      Gets the channel read pipe handler by process local id.
 *
 * @param      executor  The executor info about self process
 * @param      from      local_id of process you want to recieve message from
 *
 * @return     The channel read handler.
 */
channel_h get_channel_read_h(executor *self, local_id from);

/**
 * @brief      Gets the channel write pipe handler by process local id.
 *
 * @param      executor  The executor info about self process
 * @param      from      local_id of process you want to send message to
 *
 * @return     The channel write handler.
 */
channel_h get_channel_write_h(executor *self, local_id dst);

/**
 * @brief      Closes a channel from -> dst.
 *
 * @param      channels  The channels matrix
 * @param[in]  from      The from process local id
 * @param[in]  dst       The destination process local id
 *
 * @return     0 on success, any non-zero value on error
 */
int close_channel(channel **channels, local_id from, local_id dst);


/**
 * @brief      Closes a channela used by executor with local id.
 *
 * @param[in]  proc_n    The proc n
 * @param      channels  The channels matrix
 * @param[in]  local_id  The local id of process
 *
 * @return     0 on success, any non-zero value on error
 */
int close_channels(int8_t proc_n, channel **channels);

#endif // __ITMO_DISTRIBUTED_CLASS_CHANNELS__H

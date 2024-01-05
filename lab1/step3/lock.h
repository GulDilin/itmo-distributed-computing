/**
 * @file     lock.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Structures for lock implementation
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_LOCK__H
#define __ITMO_DISTRIBUTED_CLASS_LOCK__H

#include <stdint.h>

#include "ipc.h"

typedef enum {
    LOCK_WAITING,   ///< Lock is waiting to be active
    LOCK_ACTIVE,    ///< Lock is active now
    LOCK_INACTIVE,  ///< message with string (doesn't include trailing '\0')
} LockState;

typedef enum {
    LOCK_REPLY_SENT = 0,
    LOCK_REPLY_PENDING = 1,
} LockReplyOutState;

typedef struct {
    local_id    s_id;    ///< Executor id that requested a lock
    timestamp_t s_time;  ///< Local time when lock is requested
} LockRequest;

typedef struct {
    LockRequest       buffer[MAX_PROCESS_ID + 1];
    LockReplyOutState deffered[MAX_PROCESS_ID + 1];
    uint16_t          size;
} LockQueue;

typedef struct {
    LockState   state;
    LockQueue   queue;
    LockRequest active_request;
} Lock;

/**
 * @brief      Called on request lock.
 *
 * @param      self  The executor
 * @param      msg   The message
 * @param[in]  from  The from process id
 */
void on_request_cs(void* self, Message* msg, local_id from);

/**
 * @brief      Called on reply lock.
 *
 * @param      self  The executor
 * @param      msg   The message
 * @param[in]  from  The from process id
 */
void on_reply_cs(void* self, Message* msg, local_id from);

/**
 * @brief      Called on release lock.
 *
 * @param      self  The executor
 * @param      msg   The message
 * @param[in]  from  The from process id
 */
void on_release_cs(void* self, Message* msg, local_id from);

/**
 * @brief      Initializes the lock.
 *
 * @param      self  The executor
 */
void init_lock(void* self);

#endif  // __ITMO_DISTRIBUTED_CLASS_LOCK__H

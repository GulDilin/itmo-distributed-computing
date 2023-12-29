/**
 * @file     communicator.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Communication structs and methods for pipes
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_COMMUNICATOR__H
#define __ITMO_DISTRIBUTED_CLASS_COMMUNICATOR__H

#include <stdint.h>

#include "channels.h"
#include "executor.h"
#include "ipc.h"

/**
 * @brief      Construct a text message using format to msg pointer;
 *
 * @param      msg        The message
 * @param[in]  type       The message type
 * @param[in]  msg_fmt    The message format
 * @param[in]  <unnamed>  args to string format
 *
 * @return     0 on success, any non-zero value on error
 */
int construct_msg_text(Message *msg, MessageType type, const char *msg_fmt, ...);

/**
 * @brief      Construct a message
 *
 * @param      msg         The message
 * @param[in]  type        The message type
 * @param[in]  buffer_len  The buffer length
 *
 * @return     0 on success, any non-zero value on error
 */
int construct_msg(Message *msg, MessageType type, uint16_t buffer_len);

/**
 * @brief      Sends a started message multicast.
 *
 * @param      self  The executor process
 *
 * @return     0 on success, any non-zero value on error
 */
int send_started_msg_multicast(executor *self);

/**
 * @brief      Sends a done message multicast.
 *
 * @param      self  The executor process
 *
 * @return     0 on success, any non-zero value on error
 */
int send_done_msg_multicast(executor *self);

/**
 * @brief      Sends a stop message multicast.
 *
 * @param      self  The executor process
 *
 * @return     0 on success, any non-zero value on error
 */
int send_stop_msg_multicast(executor *self);

/**
 * @brief      Determines if message received from.
 *
 * @param      self      The executor process
 * @param[in]  received  The received mask
 * @param[in]  from      The from process local id
 *
 * @return     1 if received message from, 0 otherwise.
 */
int is_received_msg_from(executor *self, uint8_t *received, local_id from);

/**
 * @brief      Determines if received from all children.
 *
 * @param      self      The executor process
 * @param[in]  received  The received mask
 *
 * @return     1 if received from all children, 0 otherwise.
 */
int is_received_all_child(executor *self, uint8_t *received);

/**
 * @brief      Mark mask bit connected with local_id process as received.
 *
 * @param      received  The received mask
 * @param[in]  from  The from process local id
 */
void mark_received(uint8_t *received, local_id from);

/**
 * Callback type for message handling
 *
 * @param       self        The executor process info pointer
 * @param       msg         The message pointer
 * @param       local_id    Local process id mesage received from
 */
typedef void (*on_message_t)(executor *, Message *, local_id);

/**
 * @brief      Wait for all messages with specified type received from children
 *
 * @param      self        The executor process
 * @param[in]  type        The message type
 * @param[in]  on_message  On message callback (can be NULL for no callback)
 *
 * @return     0 on success, any non-zero value on error
 */

int wait_receive_all_child_msg_by_type(executor *self, MessageType type, on_message_t on_message);

/**
 * @brief      Wait for a message with specified type received from specified children
 *
 * @param      self  The executor process
 * @param[in]  type  The message type
 * @param[in]  from   The source process local id
 *
 * @return     0 on success, any non-zero value on error
 */
int wait_receive_msg_by_type(executor *self, MessageType type, local_id from);

/**
 * @brief      Update time and send a message
 *
 * @param      self  The object
 * @param[in]  dst   The destination
 * @param      msg   The message
 *
 * @return     0 on success, any non-zero value on error
 */
int tick_send(executor *self, local_id dst, Message *msg);

/**
 * @brief      Update time and send a message musticast
 *
 * @param      self  The object
 * @param[in]  dst   The destination
 * @param      msg   The message
 *
 * @return     0 on success, any non-zero value on error
 */
int tick_send_multicast(executor *self, Message *msg);

/**
 * @brief      Deserialize message payload buffer and write into target pointer
 *
 * @param      msg     The message
 * @param      target  The target struct poinger
 * @param[in]  t_size  The size of target struct type, usually sizeof(<type>), where <type> is type
 * of target
 */
void deserialize_struct(Message *msg, void *target, size_t t_size);

/**
 * @brief      Serialize target struct into message payload buffer
 *
 * @param      msg     The message
 * @param      target  The target struct poinger
 * @param[in]  t_size  The size of target struct type, usually sizeof(<type>), where <type> is type
 * of target
 */
void serialize_struct(Message *msg, void *target, size_t t_size);

#endif  // __ITMO_DISTRIBUTED_CLASS_COMMUNICATOR__H

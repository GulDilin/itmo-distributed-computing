/**
 * @file     communicator.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Communication structs and methods for pipes
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_COMMUNICATOR__H
#define __ITMO_DISTRIBUTED_CLASS_COMMUNICATOR__H

#include "channels.h"
#include "ipc.h"

/**
 * @brief      Construct a message to msg pointer;
 *
 * @param      msg        The message
 * @param[in]  type       The message type
 * @param[in]  msg_fmt    The message format
 * @param[in]  <unnamed>  args to string format
 *
 * @return     0 on success, any non-zero value on error
 */
int construct_msg(Message *msg, MessageType type, const char *msg_fmt, ...);

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
 * @brief      Determines if message recieved from.
 *
 * @param      self      The executor process
 * @param[in]  recieved  The recieved mask
 * @param[in]  from      The from process local id
 *
 * @return     1 if recieved message from, 0 otherwise.
 */
int is_recieved_msg_from(executor *self, uint16_t recieved, local_id from);

/**
 * @brief      Determines if recieved from all children.
 *
 * @param      self      The executor process
 * @param[in]  recieved  The recieved mask
 *
 * @return     1 if recieved from all children, 0 otherwise.
 */
int is_recieved_all_child(executor *self, uint16_t recieved);

/**
 * @brief      Mark mask bit connected with local_id process as recieved.
 *
 * @param      recieved  The recieved mask
 * @param[in]  from  The from process local id
 */
void mark_recieved(uint16_t *recieved, local_id from);

/**
 * @brief      Wait for all messages with specified type recieved from children
 *
 * @param      self  The executor process
 * @param[in]  type  The message type
 *
 * @return     0 on success, any non-zero value on error
 */
int wait_receive_all_child_msg_by_type(executor *self, MessageType type);

#endif  // __ITMO_DISTRIBUTED_CLASS_COMMUNICATOR__H

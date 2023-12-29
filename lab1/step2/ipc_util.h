/**
 * @file     ipc_util.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Utils functions for IPC
 */

#ifndef __IFMO_DISTRIBUTED_CLASS_IPC_UTIL__H
#define __IFMO_DISTRIBUTED_CLASS_IPC_UTIL__H

#include "ipc.h"

/**
 * @brief      Gets the message type text.
 *
 * @param[in]  msg   The message
 *
 * @return     The message type text.
 */
char *get_msg_type_text(const MessageType type);

#endif  // __IFMO_DISTRIBUTED_CLASS_IPC_UTIL__H

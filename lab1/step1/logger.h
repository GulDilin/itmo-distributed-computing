/**
 * @file     logger.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Logging messages
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_LOGGER__H
#define __ITMO_DISTRIBUTED_CLASS_LOGGER__H

#include <stdarg.h>

static const char * const log_channel_opened_fmt =
    "Channel opened (%5d -> %5d)\n";

static const char * const log_channel_closed_fmt =
    "Channel closed (%5d -> %5d)\n";

/**
 * @brief      Logs a channel opened message.
 *
 * @param[in]  from  The from process local id
 * @param[in]  dst   The destination process local id
 *
 * @return     0 on success, any non-zero value on error
 */
int log_channel_opened(int from, int dst);

/**
 * @brief      Logs a channel closed message.
 *
 * @param[in]  from  The from process local id
 * @param[in]  dst   The destination process local id
 *
 * @return     0 on success, any non-zero value on error
 */
int log_channel_closed(int from, int dst);

/**
 * @brief      Logs a started process message.
 *
 * @param      self  The object
 *
 * @return     0 on success, any non-zero value on error
 */
int log_started_msg(void * self);

/**
 * @brief      Opens a log file handler.
 *
 * @param[in]  fname  The filename
 *
 * @return     file handler
 */
int open_log_f(const char *fname);

/**
 * @brief      Opens a pipes log file handler.
 *
 * @return     file handler
 */
int open_pipes_log_f();

/**
 * @brief      Opens an events log file handler.
 *
 * @return     file handler
 */
int open_events_log_f();

/**
 * @brief      Closes a pipes log file.
 */
void close_pipes_log_f();

/**
 * @brief      Closes an events log file.
 */
void close_events_log_f();

/**
 * @brief      Logs a message to a file.
 *
 * @param[in]  handler    The file handler
 * @param[in]  fmt        The message format
 * @param[in]  <unnamed>  args
 *
 * @return     0 on success, any non-zero value on error
 */
int log_file_msg(int handler, const char *fmt, va_list args);

/**
 * @brief      Logs a message to the pipes log file.
 *
 * @param[in]  fmt        The format
 * @param[in]  <unnamed>  args
 *
 * @return     0 on success, any non-zero value on error
 */
int log_pipes_msg(const char *fmt, ...);

/**
 * @brief      Logs a message to the events log file.
 *
 * @param[in]  fmt        The format
 * @param[in]  <unnamed>  args
 *
 * @return     0 on success, any non-zero value on error
 */
int log_events_msg(const char *fmt, ...);

#endif // __ITMO_DISTRIBUTED_CLASS_LOGGER__H

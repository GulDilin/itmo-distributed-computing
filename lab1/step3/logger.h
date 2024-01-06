/**
 * @file     logger.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Logging messages
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_LOGGER__H
#define __ITMO_DISTRIBUTED_CLASS_LOGGER__H

#include <stdarg.h>

static const char *const log_channel_opened_fmt
    = "Channel opened (%2d -> %2d) [w] -> [r] [%2d] -> [%2d]\n";

static const char *const log_channel_closed_fmt = "Channel closed (%2d -> %2d)\n";

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
 * @brief      Logs a message both to a file and stdout.
 *
 * @param[in]  handler    The file handler
 * @param[in]  fmt        The message format
 * @param[in]  <unnamed>  args
 *
 * @return     0 on success, any non-zero value on error
 */
int log_msg(int handler, const char *fmt, va_list args);

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

int get_events_log_fh();

#endif  // __ITMO_DISTRIBUTED_CLASS_LOGGER__H

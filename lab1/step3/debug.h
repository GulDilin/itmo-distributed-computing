/**
 * @file     debug.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Debugging print
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_DEBUG__H
#define __ITMO_DISTRIBUTED_CLASS_DEBUG__H

#include <stdarg.h>

/**
 * @brief      Sets the debug.
 *
 * @param[in]  is_enabled  Indicates if enabled
 */
void set_debug(int is_enabled);

/**
 * @brief      Sets the debug ipc.
 *
 * @param[in]  is_enabled  Indicates if enabled
 */
void set_debug_ipc(int is_enabled);

/**
 * @brief      Sets the debug time.
 *
 * @param[in]  is_enabled  Indicates if enabled
 */
void set_debug_time(int is_enabled);

/**
 * @brief      Sets the debug worker.
 *
 * @param[in]  is_enabled  Indicates if enabled
 */
void set_debug_worker(int is_enabled);

/**
 * @brief      Gets the debug.
 */
int get_debug();

/**
 * @brief      Gets the debug ipc.
 */
int get_debug_ipc();

/**
 * @brief      Gets the debug time.
 */
int get_debug_time();

/**
 * @brief      Gets the debug worker.
 */
int get_debug_worker();

/**
 * @brief      Print message if debug is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_print(const char* fmt, ...);
// #define debug_print(fmt, ...) {}

/**
 * @brief      Print message if debug IPC is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_ipc_print(const char* fmt, ...);
// #define debug_ipc_print(fmt, ...) {}

/**
 * @brief      Print message if debug TIME is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_time_print(const char* fmt, ...);
// #define debug_time_print(fmt, ...) {}

/**
 * @brief      Print message if debug WORKER is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_worker_print(const char* fmt, ...);
// #define debug_worker_print(fmt, ...) {}

static const char* const debug_channel_init_fmt = "init_channel read_h=%3d write_h=%3d\n";
static const char* const debug_channel_open_fmt
    = "open_channel %2d -> %2d [rc=%d] [ %2d -> %2d ]\n";
static const char* const debug_channel_open_start_fmt = "open_channels start. proc_n = %d\n";
static const char* const debug_channel_set_fmt = "[local_id=%2d] ch set %c %d -> %d: %d\n";

static const char* const debug_ipc_receive_fmt
    = "%2d: [local_id=%2d] recv %2d <- %2d <type=%15s> [msg_time=%2d] [prev_time=%2d] [bytes=%d]\n";
static const char* const debug_ipc_send_fmt
    = "%2d: [local_id=%2d] send %2d -> %2d <type=%15s> [msg_time=%2d] [bytes=%d] [channel_h=%d]\n";
static const char* const debug_ipc_send_multicast_fmt
    = "%2d: [local_id=%2d] send %2d ->  * <type=%15s> [msg_time=%2d]\n";
static const char* const debug_ipc_wait_msg_fmt
    = "%2d: [local_id=%2d] Wait for message <type=%15s> [from=%2d]\n";
static const char* const debug_ipc_await_msg_fmt
    = "%2d: [local_id=%2d] Awaited message <type=%s> [from=%2d]\n";
static const char* const debug_ipc_send_failed_fmt
    = "%2d: [local_id=%2d] send failed %2d -> %2d <type=%10s> [msg_time=%2d]\n";

static const char* const debug_log_open_file_fmt = "open %s [fd=%d]\n";
static const char* const debug_log_msg_file_fmt = "log_file_msg [fd=%d] [bufsz=%lu]\n";

static const char* const debug_forked_fmt = "Hello from Proc pid=%4d parent=%4d local_id=%2d\n";
static const char* const debug_start_fork_fmt = "[pid=%4d] Start create processes\n";
static const char* const debug_proc_created_fmt = "[pid=%4d] Processes created\n";
static const char* const debug_malloc_ch_fin_fmt = "malloc channels finished [channels=%p]\n";
static const char* const debug_main_start_fmt = "Start %d\n";
static const char* const debug_main_finish_fmt = "Finish %d\n";
static const char* const debug_main_args_parse_fmt = "Parse args %d\n";
static const char* const debug_main_args_parsed_fmt = "Parsed args %d. processes: %d\n";
static const char* const debug_executor_info_fmt = "Executor pid=%4d parent=%4d local_id=%2d\n";

static const char* const debug_worker_run_fmt = "Run worker pid=%d parent=%d local_id=%d\n";
static const char* const debug_worker_start_loop_fmt = "%2d: [local_id=%2d] worker run main loop\n";

static const char* const debug_time_next_tick_fmt = "%2d: next_tick [other_time=%2d] result: %2d\n";

static const char* const debug_lock_acquire_fmt = "%2d: [local_id=%2d] lock_acquire\n";
static const char* const debug_lock_wait_fmt = "%2d: [local_id=%2d] lock_wait\n";
static const char* const debug_lock_defered_reply_fmt
    = "%2d: [local_id=%2d] defer reply for [id=%d]\n";
static const char* const debug_lock_released_fmt = "%2d: [local_id=%2d] lock_released\n";
static const char* const debug_lock_queue_fmt = "[local_id=%d] [active_t=%2d] queue (%2d): ";
static const char* const debug_lock_queue_part_fmt = "[%d, %d]";
static const char* const debug_lock_reply_at_fmt = "[local_id=%d] [active_t=%2d] reply_at: ";

#endif  // __ITMO_DISTRIBUTED_CLASS_DEBUG__H

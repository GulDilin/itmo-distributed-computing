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
 * @brief      Print message if debug is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_print(const char* fmt, ...);

/**
 * @brief      Print message if debug IPC is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_ipc_print(const char* fmt, ...);

/**
 * @brief      Print message if debug TIME is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_time_print(const char* fmt, ...);

/**
 * @brief      Print message if debug WORKER is enabled
 *
 * @param      fmt        The format
 * @param[in]  <unnamed>  args
 */
void debug_worker_print(const char* fmt, ...);

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
static const char* const debug_main_args_parse_fmt = "Parse args %d\n";
static const char* const debug_main_args_parsed_fmt = "Parsed args %d. processes: %d\n";
static const char* const debug_executor_info_fmt = "Executor pid=%4d parent=%4d local_id=%2d\n";

static const char* const debug_worker_run_fmt = "Run worker pid=%d parent=%d local_id=%d\n";
static const char* const debug_worker_balance_fmt = "%2d: [local_id=%2d] balance $%d\n";
static const char* const debug_worker_transfer_in
    = "%2d: [local_id=%2d] got_transfer_in [from=%2d] [msg_time=%2d] [balance=$%d] $%d\n";
static const char* const debug_worker_transfer_out
    = "%2d: [local_id=%2d] got_transfer_out [  to=%2d] [msg_time=%2d] [balance=$%d] $%d\n";
static const char* const debug_worker_balance_pending_fmt
    = "%2d: [local_id=%2d] Pending balance add [ts=%2d] $%d\n";
static const char* const debug_worker_set_pending_fmt
    = "%2d: [local_id=%2d] Pending set balance [ts=%2d] $%d\n";
static const char* const debug_worker_set_history_fmt
    = "%2d: [local_id=%2d] History set balance [ts=%2d] $%d\n";
static const char* const debug_worker_update_history_fmt
    = "%2d: [local_id=%2d] History update balance [from_ts=%2d] [to_ts=%2d] [old_balance=$%d] "
      "$%d\n";
static const char* const debug_worker_on_hist_fmt
    = "%2d: [local_id=%2d] got history [from=%2d] [len=%d]\n";
static const char* const debug_worker_fill_trailling_history_fmt
    = "fill_trailling_history [for=%2d]\n";
static const char* const debug_worker_stop_fmt = "%2d: [local_id=%2d] STOP\n";

static const char* const debug_time_next_tick_fmt = "%2d: next_tick [other_time=%2d] result: %2d\n";

#endif  // __ITMO_DISTRIBUTED_CLASS_DEBUG__H

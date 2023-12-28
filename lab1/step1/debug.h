/**
 * @file     debug.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Debugging print
 */

#ifndef __ITMO_DISTRIBUTED_CLASS_DEBUG__H
#define __ITMO_DISTRIBUTED_CLASS_DEBUG__H

#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

#define debug_print(fmt, ...)                         \
    do {                                              \
        if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); \
    } while (0)

static const char* const debug_channel_init_fmt = "init_channel read_h=%3d write_h=%3d\n";
static const char* const debug_channel_open_fmt = "open_channel %d -> %d [rc=%d] [ %2d -> %2d ]\n";
static const char* const debug_channel_open_start_fmt = "open_channels start. proc_n = %d\n";
static const char* const debug_channel_set_fmt = "[local_id=%d] ch set %c %d -> %d: %d\n";

static const char* const debug_ipc_receive_fmt = "[local_id=%d] receive [from=%d] rc=%3d\n";
static const char* const debug_ipc_send_fmt
    = "[local_id=%1d] [pid=%5d] send %d -> %d [channel_h=%d] [msg_size=%d] bytes=%d\n";
static const char* const debug_ipc_send_multicast_fmt
    = "[local_id=%1d] [pid=%5d] send_multicast.  proc_n: %d \n";

static const char* const debug_log_open_file_fmt = "open %s [fd=%d]\n";
static const char* const debug_log_msg_file_fmt = "log_file_msg [fd=%d] [bufsz=%lu]\n";

static const char* const debug_forked_fmt = "Hello from Proc pid=%d parent=%d local_id=%d\n";
static const char* const debug_start_fork_fmt = "[pid=%d] Start create processes\n";
static const char* const debug_proc_created_fmt = "[pid=%d] Processes created\n";
static const char* const debug_malloc_ch_fin_fmt = "malloc channels finished [channels=%p]\n";
static const char* const debug_main_start_fmt = "Start %d\n";
static const char* const debug_main_args_parse_fmt = "Parse args %d\n";
static const char* const debug_main_args_parsed_fmt = "Parsed args %d. processes: %d\n";
static const char* const debug_executor_info_fmt = "Executor pid=%d parent=%d local_id=%d\n";

static const char* const debug_worker_run_fmt = "Run worker pid=%d parent=%d local_id=%d\n";

#endif  // __ITMO_DISTRIBUTED_CLASS_DEBUG__H

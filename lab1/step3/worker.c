#include "worker.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "channels.h"
#include "communicator.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "lock.h"
#include "logger.h"
#include "pa2345.h"
#include "time.h"

/**
 * @brief      Determines whether the specified self and other children is all done.
 *
 * @param      self  The object
 *
 * @return     True if the specified self is all done, False otherwise.
 */
int is_all_done(executor *self) {
    for (int i = 1; i < self->proc_n; ++i) {
        if (!self->proc_done[i]) return 0;
    }
    return 1;
}

/**
 * @brief      Determines whether the specified self is done.
 *
 * @param      self  The object
 *
 * @return     True if the specified self is self done, False otherwise.
 */
int is_self_done(executor *self) {
    return self->proc_done[self->local_id];
}

void set_done(executor *self, local_id from) {
    self->proc_done[from] = 1;
}

void on_done(executor *self, Message *msg, local_id from) {
    set_done(self, from);
    if (is_all_done(self)) {
        log_events_msg(log_received_all_done_fmt, get_lamport_time(), self->local_id);
    }
}

void on_message(executor *self, Message *msg, local_id from) {
    switch (msg->s_header.s_type) {
        case CS_RELEASE:
            on_release_cs(self, msg, from);
            return;
        case CS_REQUEST:
            on_request_cs(self, msg, from);
            return;
        case CS_REPLY:
            on_reply_cs(self, msg, from);
            return;
        case DONE:
            on_done(self, msg, from);
            return;
    }
}

void child_start(executor *self) {
    log_events_msg(
        log_started_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid, 0
    );
    send_started_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, STARTED, NULL);
    log_events_msg(log_received_all_started_fmt, get_lamport_time(), self->local_id);
}

void child_done(executor *self) {
    set_done(self, self->local_id);
    log_events_msg(log_done_fmt, get_lamport_time(), self->local_id, 0);
    send_done_msg_multicast(self);
}

#define STR_BUF_SZ 128

void do_main_work(executor *self, int *loop_idx) {
    const int MAX_ITER_N = self->local_id * 5;
    char      str[STR_BUF_SZ];
    snprintf(str, STR_BUF_SZ, log_loop_operation_fmt, self->local_id, *loop_idx, MAX_ITER_N);
    print(str);
    *loop_idx += 1;
    if (*loop_idx >= MAX_ITER_N) child_done(self);
}

void child_worker(executor *self) {
    child_start(self);
    int main_loop_idx = 1;
    debug_worker_print(debug_worker_start_loop_fmt, get_lamport_time(), self->local_id);
    while (!is_all_done(self)) {
        receive_any_cb(self, on_message);
        // if (receive_any_cb(self, on_message) != 0) usleep(SLEEP_RECEIVE_USEC);
        if (is_self_done(self)) continue;
        if (self->use_lock && request_cs(self) != 0) continue;
        do_main_work(self, &main_loop_idx);
        if (self->use_lock) release_cs(self);
    }
}

void parent_worker(executor *self) {
    log_events_msg(
        log_started_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid, 0
    );

    wait_receive_all_child_msg_by_type(self, STARTED, NULL);
    log_events_msg(log_received_all_started_fmt, get_lamport_time(), self->local_id);

    wait_receive_all_child_msg_by_type(self, DONE, NULL);
    log_events_msg(log_received_all_done_fmt, get_lamport_time(), self->local_id);
    log_events_msg(log_done_fmt, get_lamport_time(), self->local_id, 0);
}

void run_worker(executor *self) {
    debug_worker_print(debug_worker_run_fmt, self->pid, self->parent_pid, self->local_id);
    if (self->local_id == PARENT_ID) parent_worker(self);
    else child_worker(self);
}

void init_executor(
    executor *executor, channel **channels, local_id local_id, int proc_n, pid_t pid, pid_t p_pid,
    uint8_t use_lock
) {
    executor->local_id = local_id;
    executor->proc_n = proc_n;
    executor->pid = pid;
    executor->parent_pid = p_pid;
    executor->use_lock = use_lock;

    init_lock(executor);

    for (int i = 0; i <= MAX_PROCESS_ID; ++i) { executor->proc_done[i] = 0; }

    set_executor_channels(proc_n, executor, channels);
    close_unused_channels(proc_n, local_id, channels);
}

void cleanup_executor(executor *executor) {
    free(executor->ch_read);
    free(executor->ch_write);
}

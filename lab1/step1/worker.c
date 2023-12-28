#include <stdio.h>

#include "communicator.h"
#include "debug.h"
#include "ipc.h"
#include "logger.h"
#include "pa1.h"

void child_start(executor *self) {
    log_events_msg(log_started_fmt, self->local_id, self->pid, self->parent_pid);
    send_started_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, STARTED);
    log_events_msg(log_received_all_started_fmt, self->local_id);
}

void child_done(executor *self) {
    log_events_msg(log_done_fmt, self->local_id, self->pid, self->parent_pid);
    send_done_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, DONE);
    log_events_msg(log_received_all_done_fmt, self->local_id);
}

void child_worker(executor *self) {
    child_start(self);
    child_done(self);
}

void parent_worker(executor *self) {
    log_events_msg(log_started_fmt, self->local_id, self->pid, self->parent_pid);

    wait_receive_all_child_msg_by_type(self, STARTED);
    log_events_msg(log_received_all_started_fmt, self->local_id);

    wait_receive_all_child_msg_by_type(self, DONE);
    log_events_msg(log_received_all_done_fmt, self->local_id);

    log_events_msg(log_done_fmt, self->local_id, self->pid, self->parent_pid);
}

void run_worker(executor *self) {
    debug_print(debug_worker_run_fmt, self->pid, self->parent_pid, self->local_id);

    if (self->local_id == PARENT_ID) parent_worker(self);
    else child_worker(self);
}

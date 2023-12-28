#include "worker.h"

#include <stdio.h>
#include <string.h>

#include "banking.h"
#include "channels.h"
#include "communicator.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "logger.h"
#include "pa2345.h"

void account_start(executor *self) {
    log_events_msg(
        log_started_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid,
        self->bank_account.balance
    );
    send_started_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, STARTED);
    log_events_msg(log_received_all_started_fmt, get_lamport_time(), self->local_id);
}

void account_done(executor *self) {
    log_events_msg(log_done_fmt, get_lamport_time(), self->local_id, self->bank_account.balance);
    send_done_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, DONE);
    log_events_msg(log_received_all_done_fmt, get_lamport_time(), self->local_id);

    // TODO: send history to parent
}

void deserialize_transfer_order(Message *msg, TransferOrder *order) {
    memcpy(order, msg->s_payload, sizeof(TransferOrder));
}

void serialize_transfer_order(Message *msg, TransferOrder *order) {
    memcpy(msg->s_payload, order, sizeof(TransferOrder));
}

void update_balance_in(executor *self, balance_t amount) {
    self->bank_account.balance += amount;
    debug_print(
        debug_worker_balance_fmt, get_lamport_time(), self->local_id, self->bank_account.balance
    );
    // TODO: update history
}

void update_balance_out(executor *self, balance_t amount) {
    self->bank_account.balance -= amount;
    debug_print(
        debug_worker_balance_fmt, get_lamport_time(), self->local_id, self->bank_account.balance
    );
    // TODO: update history
}

void on_transfer_in(executor *self, Message *msg, TransferOrder *order) {
    // got incoming money
    debug_print(
        debug_worker_transfer_in, get_lamport_time(), self->local_id, order->s_src,
        msg->s_header.s_local_time, self->bank_account.balance
    );
    update_balance_in(self, order->s_amount);
    log_events_msg(
        log_transfer_in_fmt, get_lamport_time(), self->local_id, order->s_amount, order->s_src
    );
    Message msg_ack;
    construct_msg(&msg_ack, ACK, 0);
    // send transfer confirmation to parent (router)
    tick_send(self, PARENT_ID, &msg_ack);
}

void on_transfer_out(executor *self, Message *msg, TransferOrder *order) {
    debug_print(
        debug_worker_transfer_out, get_lamport_time(), self->local_id, order->s_dst,
        msg->s_header.s_local_time, self->bank_account.balance
    );
    update_balance_out(self, order->s_amount);
    log_events_msg(
        log_transfer_out_fmt, get_lamport_time(), self->local_id, order->s_amount, order->s_dst
    );
    // send to transfer destination
    tick_send(self, order->s_dst, msg);
}

void on_transfer(executor *self, Message *msg) {
    TransferOrder order;
    deserialize_transfer_order(msg, &order);
    if (order.s_src == self->local_id) on_transfer_out(self, msg, &order);
    if (order.s_dst == self->local_id) on_transfer_in(self, msg, &order);
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    executor *router = parent_data;
    if (router->local_id != PARENT_ID) return;
    Message       msg;
    TransferOrder order = {.s_src = src, .s_dst = dst, .s_amount = amount};
    construct_msg(&msg, TRANSFER, sizeof(TransferOrder));
    serialize_transfer_order(&msg, &order);
    // send to transfer source
    tick_send(router, src, &msg);
    // wait for transfer confirmation from destination account
    wait_receive_msg_by_type(router, ACK, dst);
}

void on_stop(executor *self, Message *msg) {
    self->is_running = 0;
}

void on_message(executor *self, Message *msg) {
    switch (msg->s_header.s_type) {
        case STOP:
            on_stop(self, msg);
            return;
        case TRANSFER:
            on_transfer(self, msg);
            return;
    }
}

void account_worker(executor *self) {
    account_start(self);
    while (self->is_running) {
        Message msg;
        receive_any(self, &msg);
        on_message(self, &msg);
    }
    account_done(self);
}

void router_worker(executor *self) {
    log_events_msg(
        log_started_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid,
        self->bank_account.balance
    );

    wait_receive_all_child_msg_by_type(self, STARTED);
    log_events_msg(log_received_all_started_fmt, get_lamport_time(), self->local_id);

    bank_robbery(self, self->proc_n - 1);
    send_stop_msg_multicast(self);

    wait_receive_all_child_msg_by_type(self, DONE);
    log_events_msg(log_received_all_done_fmt, get_lamport_time(), self->local_id);
    // TODO: get history

    log_events_msg(log_done_fmt, get_lamport_time(), self->local_id, self->bank_account.balance);
}

void run_worker(executor *self) {
    debug_print(debug_worker_run_fmt, self->pid, self->parent_pid, self->local_id);
    if (self->local_id == PARENT_ID) router_worker(self);
    else account_worker(self);
}

void init_executor(
    executor *executor, channel **channels, local_id local_id, int proc_n, pid_t pid, pid_t p_pid,
    balance_t start_balance
) {
    executor->local_id = local_id;
    executor->proc_n = proc_n;
    executor->pid = pid;
    executor->parent_pid = p_pid;

    executor->bank_account.balance = start_balance;

    set_executor_channels(proc_n, executor, channels);
    close_unused_channels(proc_n, local_id, channels);
}

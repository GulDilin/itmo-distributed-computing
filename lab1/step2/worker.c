#include "worker.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "banking.h"
#include "channels.h"
#include "communicator.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "logger.h"
#include "pa2345.h"
#include "time.h"

void account_start(executor *self) {
    log_events_msg(
        log_started_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid,
        self->bank_account.balance
    );
    send_started_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, STARTED, NULL);
    log_events_msg(log_received_all_started_fmt, get_lamport_time(), self->local_id);
}

void update_history(executor *self) {
    timestamp_t cur_history_ts = self->bank_account.history->s_history_len - 1;
    timestamp_t local_time = get_lamport_time();
    self->bank_account.history->s_history_len = local_time + 1;
    BalanceState *cur_state = &self->bank_account.history->s_history[cur_history_ts];
    debug_worker_print(
        debug_worker_update_history_fmt, local_time, self->local_id, cur_history_ts, local_time,
        cur_state->s_balance, self->bank_account.balance
    );

    for (timestamp_t history_t = cur_history_ts + 1; history_t < local_time; ++history_t) {
        // fill history between cur_history_ts and local_time with same state
        BalanceState state
            = {.s_time = history_t,
               .s_balance = cur_state->s_balance,
               .s_balance_pending_in = cur_state->s_balance_pending_in};
        self->bank_account.history->s_history[history_t] = state;
        debug_worker_print(
            debug_worker_set_history_fmt, local_time, self->local_id, history_t,
            cur_state->s_balance
        );
    }
    BalanceState new_state
        = {.s_time = local_time, .s_balance = self->bank_account.balance, .s_balance_pending_in = 0
        };
    self->bank_account.history->s_history[local_time] = new_state;
    debug_worker_print(
        debug_worker_set_history_fmt, local_time, self->local_id, local_time, new_state.s_balance
    );
}

void update_history_pending_in(executor *self, balance_t amount, timestamp_t incoming_time) {
    timestamp_t local_time = get_lamport_time();
    debug_worker_print(
        debug_worker_balance_pending_fmt, local_time, self->local_id, incoming_time, amount
    );
    for (timestamp_t history_t = incoming_time; history_t < local_time; ++history_t) {
        debug_worker_print(
            debug_worker_set_pending_fmt, local_time, self->local_id, history_t, amount
        );
        self->bank_account.history->s_history[history_t].s_balance_pending_in += amount;
    }
}

void update_balance_in(executor *self, Message *msg, TransferOrder *order) {
    self->bank_account.balance += order->s_amount;
    debug_worker_print(
        debug_worker_balance_fmt, get_lamport_time(), self->local_id, self->bank_account.balance
    );
    update_history(self);
    update_history_pending_in(self, order->s_amount, msg->s_header.s_local_time);
}

void update_balance_out(executor *self, TransferOrder *order) {
    self->bank_account.balance -= order->s_amount;
    debug_worker_print(
        debug_worker_balance_fmt, get_lamport_time(), self->local_id, self->bank_account.balance
    );
    update_history(self);
    // TODO: update history
}

void on_transfer_in(executor *self, Message *msg, TransferOrder *order) {
    // got incoming money
    debug_worker_print(
        debug_worker_transfer_in, get_lamport_time(), self->local_id, order->s_src,
        msg->s_header.s_local_time, self->bank_account.balance, order->s_amount
    );
    update_balance_in(self, msg, order);
    log_events_msg(
        log_transfer_in_fmt, get_lamport_time(), self->local_id, order->s_amount, order->s_src
    );
    Message msg_ack;
    construct_msg(&msg_ack, ACK, 0);
    // send transfer confirmation to parent (router)
    tick_send(self, PARENT_ID, &msg_ack);
}

void on_transfer_out(executor *self, Message *msg, TransferOrder *order) {
    debug_worker_print(
        debug_worker_transfer_out, get_lamport_time(), self->local_id, order->s_dst,
        msg->s_header.s_local_time, self->bank_account.balance, order->s_amount
    );
    log_events_msg(
        log_transfer_out_fmt, get_lamport_time(), self->local_id, order->s_amount, order->s_dst
    );
    // send to transfer destination
    tick_send(self, order->s_dst, msg);
    update_balance_out(self, order);
}

void on_transfer(executor *self, Message *msg) {
    TransferOrder order;
    deserialize_struct(msg, &order, sizeof(TransferOrder));
    if (order.s_src == self->local_id) on_transfer_out(self, msg, &order);
    if (order.s_dst == self->local_id) on_transfer_in(self, msg, &order);
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    executor *router = parent_data;
    if (router->local_id != PARENT_ID) return;
    Message       msg;
    TransferOrder order = {.s_src = src, .s_dst = dst, .s_amount = amount};
    construct_msg(&msg, TRANSFER, sizeof(TransferOrder));
    serialize_struct(&msg, &order, sizeof(TransferOrder));
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

void account_done(executor *self) {
    log_events_msg(log_done_fmt, get_lamport_time(), self->local_id, self->bank_account.balance);
    send_done_msg_multicast(self);
    wait_receive_all_child_msg_by_type(self, DONE, NULL);
    log_events_msg(log_received_all_done_fmt, get_lamport_time(), self->local_id);

    Message msg;
    construct_msg(&msg, BALANCE_HISTORY, sizeof(BalanceHistory));
    serialize_struct(&msg, self->bank_account.history, sizeof(BalanceHistory));
    tick_send(self, PARENT_ID, &msg);
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

void on_balance_history(executor *self, Message *msg, local_id from) {
    BalanceHistory history;
    deserialize_struct(msg, &history, sizeof(BalanceHistory));
    // from - 1 because children id starts from 1
    self->bank_account.all_history->s_history[from - 1] = history;
}

/**
 * @brief      Gets all history maximum time.
 *
 * @param      all_history  All history
 *
 * @return     All history maximum time.
 */
timestamp_t get_all_history_max_time(AllHistory *all_history) {
    timestamp_t max_time = 0;

    for (int acc_i = 0; acc_i < all_history->s_history_len; ++acc_i) {
        BalanceHistory *acc_history = &all_history->s_history[acc_i];
        for (int history_t = 0; history_t < acc_history->s_history_len; ++history_t) {
            const BalanceState *change = &acc_history->s_history[history_t];
            if (max_time < change->s_time) { max_time = change->s_time; }
        }
    }

    return max_time;
}

/**
 * @brief      Fills empty history cells at the end using max time of all_history
 *
 * @param      all_history  All history
 */
void fill_trailling_history(AllHistory *all_history) {
    timestamp_t max_time = get_all_history_max_time(all_history);

    printf("max_time=%d\n", max_time);

    for (int acc_i = 0; acc_i < all_history->s_history_len; ++acc_i) {
        BalanceHistory *acc_history = &all_history->s_history[acc_i];
        timestamp_t     last_history_t = acc_history->s_history_len - 1;
        acc_history->s_history_len = max_time + 1;
        BalanceState *last_state = &acc_history->s_history[last_history_t];

        for (timestamp_t history_t = last_history_t + 1; history_t < acc_history->s_history_len; ++history_t) {
            BalanceState state = {
                .s_time = history_t, .s_balance = last_state->s_balance, .s_balance_pending_in = 0
            };
            acc_history->s_history[history_t] = state;
        }
    }
}

void router_worker(executor *self) {
    log_events_msg(
        log_started_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid,
        self->bank_account.balance
    );

    wait_receive_all_child_msg_by_type(self, STARTED, NULL);
    log_events_msg(log_received_all_started_fmt, get_lamport_time(), self->local_id);

    bank_robbery(self, self->proc_n - 1);
    send_stop_msg_multicast(self);

    wait_receive_all_child_msg_by_type(self, DONE, NULL);
    log_events_msg(log_received_all_done_fmt, get_lamport_time(), self->local_id);
    // TODO: get history

    wait_receive_all_child_msg_by_type(self, BALANCE_HISTORY, on_balance_history);
    fill_trailling_history(self->bank_account.all_history);
    print_history(self->bank_account.all_history);

    log_events_msg(log_done_fmt, get_lamport_time(), self->local_id, self->bank_account.balance);
}

void run_worker(executor *self) {
    debug_worker_print(debug_worker_run_fmt, self->pid, self->parent_pid, self->local_id);
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
    if (local_id == PARENT_ID) {
        executor->bank_account.history = NULL;
        executor->bank_account.all_history = malloc(sizeof(AllHistory));
        // number of children
        executor->bank_account.all_history->s_history_len = proc_n - 1;
    } else {
        executor->bank_account.history = malloc(sizeof(BalanceHistory));
        executor->bank_account.history->s_id = local_id;
        executor->bank_account.history->s_history_len = 1;
        BalanceState initial_state = {
            .s_time = MIN_T,
            .s_balance = start_balance,
            .s_balance_pending_in = 0,
        };
        executor->bank_account.history->s_history[0] = initial_state;
        executor->bank_account.all_history = NULL;
    }

    set_executor_channels(proc_n, executor, channels);
    close_unused_channels(proc_n, local_id, channels);
}

void cleanup_executor(executor *executor) {
    if (executor->bank_account.all_history != NULL) free(executor->bank_account.all_history);
    if (executor->bank_account.history != NULL) free(executor->bank_account.history);
    free(executor->ch_read);
    free(executor->ch_write);
}

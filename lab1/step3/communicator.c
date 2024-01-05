#include "communicator.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "channels.h"
#include "debug.h"
#include "ipc.h"
#include "ipc_util.h"
#include "logger.h"
#include "pa2345.h"
#include "time.h"

int usleep(__useconds_t useconds);

int construct_msg_text(Message *msg, MessageType type, const char *msg_fmt, ...) {
    va_list args;
    va_start(args, msg_fmt);

    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = type;

    const int length = vsnprintf(msg->s_payload, MAX_PAYLOAD_LEN, msg_fmt, args);
    va_end(args);
    if (length < 0) return -1;

    msg->s_header.s_payload_len = length;
    return 0;
}

int construct_msg(Message *msg, MessageType type, uint16_t payload_len) {
    if (payload_len > MAX_PAYLOAD_LEN) return -1;
    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = type;
    msg->s_header.s_payload_len = payload_len;
    return 0;
}

int tick_send(executor *self, local_id dst, Message *msg) {
    next_tick(TIME_UNSET);
    msg->s_header.s_local_time = get_lamport_time();
    return send(self, dst, msg);
}

int tick_send_multicast(executor *self, Message *msg) {
    next_tick(TIME_UNSET);
    msg->s_header.s_local_time = get_lamport_time();
    return send_multicast(self, msg);
}

int send_started_msg_multicast(executor *self) {
    Message msg;
    construct_msg_text(
        &msg, STARTED, log_started_fmt, get_lamport_time(), self->local_id, self->pid,
        self->parent_pid
    );
    return tick_send_multicast(self, &msg);
}

int send_done_msg_multicast(executor *self) {
    Message msg;
    construct_msg_text(
        &msg, DONE, log_done_fmt, get_lamport_time(), self->local_id, self->pid, self->parent_pid
    );
    return tick_send_multicast(self, &msg);
}

int send_request_cs_msg_multicast(executor *self) {
    Message msg;
    construct_msg(&msg, CS_REQUEST, 0);
    return tick_send_multicast(self, &msg);
}

int send_reply_cs_msg(executor *self, local_id to) {
    Message msg;
    construct_msg(&msg, CS_REPLY, 0);
    return tick_send(self, to, &msg);
}

int send_release_cs_msg_multicast(executor *self) {
    Message msg;
    construct_msg(&msg, CS_RELEASE, 0);
    return tick_send_multicast(self, &msg);
}

int is_received_msg_from(executor *self, uint8_t *received, local_id from) {
    return received[from];
}

int is_received_all_child(executor *self, uint8_t *received) {
    for (int other_id = 0; other_id < self->proc_n; ++other_id) {
        if (other_id == self->local_id) continue;
        if (other_id == PARENT_ID) continue;
        if (!is_received_msg_from(self, received, other_id)) return 0;
    }
    return 1;
}

void mark_received(uint8_t *received, local_id from) {
    received[from] = 1;
}

int wait_receive_all_child_if(
    executor *self, on_message_condition_t condition, void *condition_param, on_message_t on_message
) {
    uint8_t  received[MAX_PROCESS_ID + 1] = {0};
    Message  msg;
    local_id from = 0;
    while (!is_received_all_child(self, received)) {
        if (from == self->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        from = (from + 1) % self->proc_n;
        if (self->local_id == from) continue;
        if (is_received_msg_from(self, received, from)) continue;
        if (receive(self, from, &msg) != 0) continue;
        if (condition(self, &msg, from, condition_param)) mark_received(received, from);
        if (on_message != NULL) on_message(self, &msg, from);
    }
    return 0;
}

int condifion_msg_type(executor *self, Message *msg, local_id from, void *condition_param) {
    MessageType *type = condition_param;
    return msg->s_header.s_type == *type;
}

int wait_receive_all_child_msg_by_type(executor *self, MessageType type, on_message_t on_message) {
    MessageType s_type = type;
    return wait_receive_all_child_if(self, condifion_msg_type, &s_type, on_message);
}

int condifion_msg_after(executor *self, Message *msg, local_id from, void *condition_param) {
    timestamp_t *after = condition_param;
    return msg->s_header.s_local_time > *after;
}

int wait_receive_all_child_msg_after(executor *self, timestamp_t after, on_message_t on_message) {
    timestamp_t s_after = after;
    return wait_receive_all_child_if(self, condifion_msg_after, &s_after, on_message);
}

int receive_any_cb(executor *self, on_message_t on_message) {
    Message msg;
    int     received = 0;
    for (local_id from = 0; from < self->proc_n; ++from) {
        if (self->local_id == from) continue;
        if (receive(self, from, &msg) == 0) {
            if (on_message != NULL) on_message(self, &msg, from);
            received++;
        }
    }
    return received > 0 ? 0 : 1;
}

void hanle_pending(executor *self, on_message_t on_message) {
    while (receive_any_cb(self, on_message) == 0) {}
}

int wait_receive_msg_by_type(executor *self, MessageType type, local_id from) {
    Message  msg;
    uint16_t received = 0;
    debug_ipc_print(
        debug_ipc_wait_msg_fmt, get_lamport_time(), self->local_id, get_msg_type_text(type), from
    );
    while (!received) {
        int rc = receive(self, from, &msg);
        if (rc == 0 && msg.s_header.s_type == type) received = 1;
        if (rc != 0) usleep(SLEEP_RECEIVE_USEC);
    }
    debug_ipc_print(
        debug_ipc_await_msg_fmt, get_lamport_time(), self->local_id, get_msg_type_text(type), from
    );
    return 0;
}

void deserialize_struct(Message *msg, void *target, size_t t_size) {
    memcpy(target, msg->s_payload, t_size);
}

void serialize_struct(Message *msg, void *target, size_t t_size) {
    memcpy(msg->s_payload, target, t_size);
}

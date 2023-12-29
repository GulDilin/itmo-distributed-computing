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
    construct_msg(&msg, CS_REQUEST, 0);
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

int wait_receive_all_child_msg_by_type(executor *self, MessageType type, on_message_t on_message) {
    uint8_t  received[MAX_PROCESS_ID + 1] = {0};
    Message  msg;
    local_id from = 0;
    while (!is_received_all_child(self, received)) {
        if (from == self->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        from = (from + 1) % self->proc_n;
        if (self->local_id == from) continue;
        if (is_received_msg_from(self, received, from)) continue;
        int rc = receive(self, from, &msg);
        if (rc == 0 && msg.s_header.s_type == type) {
            if (on_message != NULL) on_message(self, &msg, from);
            mark_received(received, from);
        }
    }
    return 0;
}

int receive_any_cb(executor *self, on_message_t on_message) {
    Message  msg;
    local_id from = 0;
    while (1) {
        if (self->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        from = (from + 1) % self->proc_n;
        if (self->local_id == from) continue;
        if (receive(self, from, &msg) == 0) {
            if (on_message != NULL) on_message(self, &msg, from);
            return 0;
        }
    }
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
        usleep(SLEEP_RECEIVE_USEC);
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

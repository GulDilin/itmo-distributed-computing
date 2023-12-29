#include "ipc.h"

#include <stdint.h>
#include <unistd.h>

#include "channels.h"
#include "debug.h"
#include "executor.h"
#include "ipc_util.h"
#include "time.h"

size_t compute_msg_size(const Message *msg) {
    return sizeof(MessageHeader) + msg->s_header.s_payload_len;
}

char *get_msg_type_text(const MessageType type) {
    switch (type) {
        case STARTED:
            return "STARTED";
        case DONE:
            return "DONE";
        case TRANSFER:
            return "TRANSFER";
        case ACK:
            return "ACK";
        case STOP:
            return "STOP";
        case BALANCE_HISTORY:
            return "BALANCE_HISTORY";
        case CS_REQUEST:
            return "CS_REQUEST";
        case CS_REPLY:
            return "CS_REPLY";
        case CS_RELEASE:
            return "CS_RELEASE";
        default:
            return "UNDEFINED";
    }
}

int send(void *self, local_id dst, const Message *msg) {
    executor *executor = self;
    uint16_t  msg_size = compute_msg_size(msg);
    channel_h channel_h = get_channel_write_h(executor, dst);
    int       bytes = write(channel_h, msg, msg_size);
    debug_ipc_print(
        debug_ipc_send_fmt, get_lamport_time(), executor->local_id, executor->local_id, dst,
        get_msg_type_text(msg->s_header.s_type), msg->s_header.s_local_time, bytes, channel_h
    );
    int rc = bytes > 0 ? 0 : 1;
    if (rc != 0) {
        debug_ipc_print(
            debug_ipc_send_failed_fmt, get_lamport_time(), executor->local_id, executor->local_id,
            dst, get_msg_type_text(msg->s_header.s_type), msg->s_header.s_local_time
        );
    }
    return rc;
}

int send_multicast(void *self, const Message *msg) {
    executor *executor = self;
    debug_ipc_print(
        debug_ipc_send_multicast_fmt, get_lamport_time(), executor->local_id, executor->local_id,
        get_msg_type_text(msg->s_header.s_type), msg->s_header.s_local_time
    );
    int rc = 0;
    for (int dst = 0; dst < executor->proc_n; ++dst) {
        if (executor->local_id == dst) continue;
        rc = send(executor, dst, msg);
        if (rc != 0) break;
    }
    return rc;
}

int receive(void *self, local_id from, Message *msg) {
    executor *executor = self;
    channel_h channel_h = get_channel_read_h(executor, from);
    if (channel_h == -1) return -1;
    int bytes = 0;
    if ((bytes = read(channel_h, &(msg->s_header), sizeof(MessageHeader))) <= 0) return -1;
    if (msg->s_header.s_payload_len > 0
        && (bytes = read(channel_h, msg->s_payload, msg->s_header.s_payload_len)) <= 0)
        return 1;
    timestamp_t prev_time = get_lamport_time();
    next_tick(msg->s_header.s_local_time);
    debug_ipc_print(
        debug_ipc_receive_fmt, get_lamport_time(), executor->local_id, executor->local_id, from,
        get_msg_type_text(msg->s_header.s_type), msg->s_header.s_local_time, prev_time, bytes
    );
    return 0;
}

int receive_any(void *self, Message *msg) {
    executor *executor = self;
    int       received_n = 0;
    local_id  local_id = 0;
    while (received_n == 0) {
        // if (executor->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        local_id = (local_id + 1) % executor->proc_n;
        if (executor->local_id == local_id) continue;
        int rc = receive(executor, local_id, msg);
        received_n += (rc == 0);
    }
    return 0;
}

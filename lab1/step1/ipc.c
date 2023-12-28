#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
// #include <time.h>

int usleep(__useconds_t useconds);

#include "channels.h"
#include "debug.h"
#include "ipc.h"

size_t compute_msg_size(const Message *msg) {
    return sizeof(MessageHeader) + msg->s_header.s_payload_len;
}

int send(void *self, local_id dst, const Message *msg) {
    executor *executor = self;
    uint16_t  msg_size = compute_msg_size(msg);
    channel_h channel_h = get_channel_write_h(executor, dst);
    int       bytes = write(channel_h, msg, msg_size);
    debug_print(
        debug_ipc_send_fmt, executor->local_id, executor->pid, executor->local_id, dst, channel_h,
        msg_size, bytes
    );
    return bytes > 0 ? 0 : 1;
}

int send_multicast(void *self, const Message *msg) {
    executor *executor = self;
    debug_print(debug_ipc_send_multicast_fmt, executor->local_id, executor->pid, executor->proc_n);
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
    if (read(channel_h, &(msg->s_header), sizeof(MessageHeader)) == -1) return -1;
    if (read(channel_h, msg->s_payload, msg->s_header.s_payload_len) == -1) return 1;
    return 0;
}

int receive_any(void *self, Message *msg) {
    executor *executor = self;
    int       received_n = 0;
    local_id  local_id = 0;
    while (received_n == 0) {
        if (executor->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        local_id = (local_id + 1) % executor->proc_n;
        if (executor->local_id == local_id) continue;
        int rc = receive(executor, local_id, msg);
        received_n += (rc == 0);
    }
    return 0;
}

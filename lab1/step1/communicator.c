#include "communicator.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "channels.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "logger.h"
#include "pa1.h"

int usleep(__useconds_t useconds);

int construct_msg(Message *msg, MessageType type, const char *msg_fmt, ...) {
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

int send_started_msg_multicast(executor *self) {
    Message msg;
    construct_msg(&msg, STARTED, log_started_fmt, self->local_id, self->pid, self->parent_pid);
    return send_multicast(self, &msg);
}

int send_done_msg_multicast(executor *self) {
    Message msg;
    construct_msg(&msg, DONE, log_done_fmt, self->local_id, self->pid, self->parent_pid);
    return send_multicast(self, &msg);
}

int is_received_msg_from(executor *self, uint16_t received, local_id from) {
    /*
     * example:
     *      self - (local_id = 5)
     *      received - 0000 0000 0000 1010    (message received from processes 1
     * and 3) from - 3
     *
     *      received >> 3
     *              0000 0000 0000 0001
     *      MASK    0000 0000 0000 0001
     *      RESULT  0000 0000 0000 0001 (is received)
     */
    const uint16_t MASK = 1;
    return (received >> from) & MASK;
}

int is_received_all_child(executor *self, uint16_t received) {
    for (int other_id = 0; other_id < self->proc_n; ++other_id) {
        if (other_id == self->local_id) continue;
        if (other_id == PARENT_ID) continue;
        if (!is_received_msg_from(self, received, other_id)) return 0;
    }
    return 1;
}

void mark_received(uint16_t *received, local_id from) {
    const uint16_t MASK = 1;
    *received |= (MASK << from);
}

int wait_receive_all_child_msg_by_type(executor *self, MessageType type) {
    uint16_t received = 0;
    Message  msg;
    local_id from = 0;
    while (!is_received_all_child(self, received)) {
        if (self->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        from = (from + 1) % self->proc_n;
        if (self->local_id == from) continue;
        if (is_received_msg_from(self, received, from)) continue;
        int rc = receive(self, from, &msg);
        if (rc == 0) { debug_print(debug_ipc_receive_fmt, self->local_id, from, rc); }

        if (rc == 0 && msg.s_header.s_type == type) mark_received(&received, from);
    }
    return 0;
}

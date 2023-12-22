#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
// #include <time.h>

#include "communicator.h"
#include "channels.h"
#include "debug.h"
#include "logger.h"
#include "pa1.h"
#include "ipc.h"

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

int send_started_msg_multicast(executor * self) {
    Message msg;
    construct_msg(
        &msg, STARTED,
        log_started_fmt, self->local_id, self->pid, self->parent_pid
    );
    return send_multicast(self, &msg);
}

int send_done_msg_multicast(executor * self) {
    Message msg;
    construct_msg(
        &msg, DONE,
        log_done_fmt, self->local_id, self->pid, self->parent_pid
    );
    return send_multicast(self, &msg);
}

int is_recieved_msg_from(executor * self, uint16_t recieved, local_id from) {
    /*
     * example:
     *      self - (local_id = 5)
     *      recieved - 0000 0000 0000 1010    (message recieved from processes 1 and 3)
     *      from - 3
     *
     *      recieved >> 3
     *              0000 0000 0000 0001
     *      MASK    0000 0000 0000 0001
     *      RESULT  0000 0000 0000 0001 (is recieved)
     */
    const uint16_t MASK = 1;
    return (recieved >> from) & MASK;
}

int is_recieved_all_child(executor * self, uint16_t recieved) {
    for (int other_id = 0; other_id < self->proc_n; ++other_id) {
        if (other_id == self -> local_id) continue;
        if (other_id == PARENT_ID) continue;
        if (!is_recieved_msg_from(self, recieved, other_id)) return 0;
    }
    return 1;
}

void mark_recieved(uint16_t * recieved, local_id from) {
    const uint16_t MASK = 1;
    *recieved |= (MASK << from);
}

int wait_receive_all_child_msg_by_type(executor * self, MessageType type) {
    uint16_t recieved = 0;
    Message msg;
    local_id from = 0;
    while (!is_recieved_all_child(self, recieved)) {
        if (self->proc_n - 1) usleep(SLEEP_RECEIVE_USEC);
        from = (from + 1) % self->proc_n;
        if (self->local_id == from) continue;
        if (is_recieved_msg_from(self, recieved, from)) continue;
        int rc = receive(self, from, &msg);
        if (rc == 0) debug_print("[local_id=%d] receive [from=%d] rc=%3d\n", self->local_id, from, rc);

        if (rc == 0 && msg.s_header.s_type == type) mark_recieved(&recieved, from);
    }
    return 0;
}

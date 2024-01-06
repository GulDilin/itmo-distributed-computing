#include "lock.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "channels.h"
#include "communicator.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "pa2345.h"
#include "time.h"

void print_queue(executor* self) {
    if (!get_debug_worker()) return;
    char buffer[256];
    int  printed = 0;
    printed += sprintf(
        buffer, debug_lock_queue_fmt, get_lamport_time(), self->local_id,
        self->lock.active_request.s_time, self->lock.queue.size
    );
    for (int i = 0; i < self->lock.queue.size; ++i) {
        printed += sprintf(
            buffer + printed, debug_lock_queue_part_fmt, self->lock.queue.buffer[i].s_time,
            self->lock.queue.buffer[i].s_id
        );
    }
    debug_worker_print("%s\n", buffer);
}

void push_request(executor* self, LockRequest* req) {
    uint8_t idx = 0;  // index of item where to insert new request
    if (self->lock.queue.size > 0) {
        while (idx < self->lock.queue.size && self->lock.queue.buffer[idx].s_time < req->s_time) {
            idx++;
        }
        while (idx < self->lock.queue.size && self->lock.queue.buffer[idx].s_time == req->s_time
               && self->lock.queue.buffer[idx].s_id < req->s_id) {
            idx++;
        }
    }
    print_queue(self);
    debug_worker_print(
        debug_lock_queue_pop_fmt, get_lamport_time(), self->local_id, req->s_time, req->s_id, idx
    );
    /*
     * Move end part of queue if insert index is not the last
     *           N
     *           v
     * 0 1 2 3 4   5 6 7
     * X X X X X   Y Y Y
     *              \ \ \
     * 0 1 2 3 4  5  6 7 8
     * X X X X X  N  Y Y Y
     */
    if (idx < self->lock.queue.size) {
        LockRequest buffer[MAX_PROCESS_ID + 1];
        memcpy(buffer, self->lock.queue.buffer, sizeof(LockRequest) * self->lock.queue.size);
        memcpy(
            self->lock.queue.buffer + idx + 1, buffer + idx,
            sizeof(LockRequest) * (self->lock.queue.size - idx)
        );
    }
    self->lock.queue.buffer[idx] = *req;
    self->lock.queue.size++;
    print_queue(self);
}

void pop_request(executor* self, local_id from) {
    if (self->lock.queue.size < 1) return;
    /*
     * Move part of buffer on delete item
     *           v
     * 0 1 2 3 4 5 6 7 8
     * X X X X X D Y Y Y
     *            / / /
     * 0 1 2 3 4 5 6 7
     * X X X X X Y Y Y
     */
    uint8_t idx = 0;  // index of item we want to remove
    for (idx = 0; idx < self->lock.queue.size; ++idx) {
        if (self->lock.queue.buffer[idx].s_id == from) break;
    }
    if (idx >= self->lock.queue.size) return;
    print_queue(self);
    debug_worker_print(
        debug_lock_queue_pop_fmt, get_lamport_time(), self->local_id,
        self->lock.queue.buffer[idx].s_time, self->lock.queue.buffer[idx].s_id, from, idx
    );
    if (idx < self->lock.queue.size - 1) {
        LockRequest buffer[MAX_PROCESS_ID + 1];
        memcpy(buffer, self->lock.queue.buffer, sizeof(LockRequest) * self->lock.queue.size);
        memcpy(
            self->lock.queue.buffer + idx, buffer + idx + 1,
            sizeof(LockRequest) * (self->lock.queue.size - idx - 1)
        );
    }
    self->lock.queue.size--;
    print_queue(self);
}

void init_lock(void* s_self) {
    executor* self = s_self;
    self->lock.state = LOCK_INACTIVE;
    self->lock.queue.size = 0;
    self->lock.active_request.s_id = self->local_id;
    self->lock.active_request.s_time = 0;
}

/**
 * @brief      Determines ability to activate self->lock.
 *
 * @param[in]  self  The object
 *
 * @return     True if able to activate lock, False otherwise.
 */
int can_activate_lock(executor* self) {
    if (self->lock.queue.size < 1) return 0;
    if (self->lock.queue.buffer[0].s_id != self->lock.active_request.s_id) return 0;
    return 1;
}

void on_request_cs(void* s_self, Message* msg, local_id from) {
    executor*   self = s_self;
    LockRequest req = {.s_id = from, .s_time = msg->s_header.s_local_time};
    push_request(self, &req);
    send_reply_cs_msg(self, from);
}

void on_reply_cs(void* s_self, Message* msg, local_id from) {
    // do nothing because we just waiting incoming messages after timestamp
}

void on_release_cs(void* s_self, Message* msg, local_id from) {
    executor* self = s_self;
    pop_request(self, from);
}

int request_cs(const void* s_self) {
    executor* self = (executor*)s_self;
    if (self->lock.state == LOCK_ACTIVE) return 0;
    if (self->lock.state == LOCK_INACTIVE) {
        send_request_cs_msg_multicast(self);
        self->lock.state = LOCK_WAITING;
        LockRequest req = {.s_id = self->local_id, .s_time = get_lamport_time()};
        self->lock.active_request = req;
        debug_worker_print(debug_lock_wait_fmt, get_lamport_time(), self->local_id);

        push_request(self, &req);
        wait_receive_all_child_msg_after(self, req.s_time, on_message);
        debug_worker_print(debug_lock_await_reply_fmt, get_lamport_time(), self->local_id);
        print_queue(self);

        while (!can_activate_lock(self)) {
            if (receive_any_cb(self, on_message) != 0) usleep(SLEEP_RECEIVE_USEC);
        }

        self->lock.state = LOCK_ACTIVE;
        debug_worker_print(debug_lock_acquire_fmt, get_lamport_time(), self->local_id);
        return 0;
    }
    return 1;
}

int release_cs(const void* s_self) {
    executor* self = (executor*)s_self;
    if (self->lock.state == LOCK_WAITING) return 1;
    if (self->lock.state == LOCK_INACTIVE) return 0;
    if (self->lock.state == LOCK_ACTIVE) {
        pop_request(self, self->local_id);
        self->lock.state = LOCK_INACTIVE;
        debug_worker_print(debug_lock_released_fmt, get_lamport_time(), self->local_id);
        send_release_cs_msg_multicast(self);
    }
    return 0;
}

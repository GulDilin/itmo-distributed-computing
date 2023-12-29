#include "lock.h"

#include <stdint.h>
#include <string.h>

#include "communicator.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "pa2345.h"
#include "time.h"

static Lock lock;

void push_request(const executor* self, LockRequest* req) {
    uint8_t idx = 0;  // index of item where to insert new request
    while (lock.queue.size > 0 && idx < lock.queue.size
           && lock.queue.buffer[idx].s_time < req->s_time
           && lock.queue.buffer[idx].s_id < req->s_id) {
        idx++;
    }
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
    if (idx < lock.queue.size) {
        memcpy(
            lock.queue.buffer + idx + 1, lock.queue.buffer + idx,
            sizeof(LockRequest) * (lock.queue.size - idx)
        );
    }
    lock.queue.buffer[idx] = *req;
    lock.queue.size++;
}

void pop_request(const executor* self, local_id from) {
    if (lock.queue.size < 1) return;
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
    for (idx = 0; idx < lock.queue.size; ++idx) {
        if (lock.queue.buffer[idx].s_id == from) break;
    }
    if (idx >= lock.queue.size) return;
    if (idx < lock.queue.size - 1) {
        memcpy(
            lock.queue.buffer + idx, lock.queue.buffer + idx + 1,
            sizeof(LockRequest) * (lock.queue.size - idx - 1)
        );
    }
    lock.queue.size--;
}

void clean_replied(const executor* self) {
    for (int i = 0; i < MAX_PROCESS_ID + 1; ++i) { lock.replied_at[i] = 0; }
}

void init_lock(executor* self) {
    lock.state = LOCK_INACTIVE;
    lock.queue.size = 0;
    lock.active_request.s_id = self->local_id;
    lock.active_request.s_time = 0;
    clean_replied(self);
}

int is_all_replied_after_request(const executor* self) {
    for (int i = 1; i < MAX_PROCESS_ID + 1; ++i) {
        if (lock.replied_at[i] <= lock.active_request.s_time) return 0;
    }
    return 1;
}

int can_activate_lock(const executor* self) {
    if (lock.state != LOCK_WAITING) return 0;
    if (lock.queue.size < 1) return 0;
    if (lock.queue.buffer[0].s_id != lock.active_request.s_id) return 0;
    if (!is_all_replied_after_request(self)) return 0;
    return 1;
}

void on_reply_cs(executor* self, Message* msg, local_id from) {
    lock.replied_at[from] = msg->s_header.s_local_time;
}

void on_request_cs(executor* self, Message* msg, local_id from) {
    LockRequest req = {.s_id = from, .s_time = msg->s_header.s_local_time};
    push_request(self, &req);
    send_reply_cs_msg(self, from);
}

void on_release_cs(executor* self, Message* msg, local_id from) {
    pop_request(self, from);
}

int request_cs(const void* self) {
    const executor* s_executor = self;
    if (lock.state == LOCK_ACTIVE) return 0;
    if (lock.state == LOCK_INACTIVE) {
        clean_replied(self);
        LockRequest req = {.s_id = s_executor->local_id, .s_time = get_lamport_time()};
        push_request(s_executor, &req);
        send_request_cs_msg_multicast(s_executor);
        lock.state = LOCK_WAITING;
    }
    if (can_activate_lock(s_executor)) {
        lock.state = LOCK_ACTIVE;
        debug_worker_print(debug_lock_acquire_fmt, get_lamport_time(), s_executor->local_id);
        return 0;
    }
    return 1;
}

int release_cs(const void* self) {
    const executor* s_executor = self;
    if (lock.state == LOCK_WAITING) return 1;
    if (lock.state == LOCK_INACTIVE) return 0;
    if (lock.state == LOCK_ACTIVE) {
        send_release_cs_msg_multicast(s_executor);
        pop_request(s_executor, s_executor->local_id);
        debug_worker_print(debug_lock_released_fmt, get_lamport_time(), s_executor->local_id);
    }
}
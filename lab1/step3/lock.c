#include "lock.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "communicator.h"
#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "pa2345.h"
#include "time.h"

static Lock lock;

void print_queue(const executor* self) {
    if (!get_debug_worker()) return;
    char buffer[256];
    int  printed = 0;
    printed += sprintf(
        buffer, debug_lock_queue_fmt, self->local_id, lock.active_request.s_time, lock.queue.size
    );
    for (int i = 0; i < lock.queue.size; ++i) {
        printed += sprintf(
            buffer + printed, debug_lock_queue_part_fmt, lock.queue.buffer[i].s_time,
            lock.queue.buffer[i].s_id
        );
    }
    printf("%s\n", buffer);
}

void print_reply_at(const executor* self) {
    if (!get_debug()) return;
    char buffer[256];
    int  printed = 0;
    printed += sprintf(buffer, debug_lock_reply_at_fmt, self->local_id, lock.active_request.s_time);
    for (int i = 1; i < self->proc_n; ++i) {
        printed += sprintf(buffer + printed, "%2d | ", lock.replied_at[i]);
    }
    printf("%s\n", buffer);
}

void push_request(const executor* self, LockRequest* req) {
    uint8_t idx = 0;  // index of item where to insert new request
    if (lock.queue.size > 0) {
        while (idx < lock.queue.size && lock.queue.buffer[idx].s_time < req->s_time) { idx++; }
        while (idx < lock.queue.size && lock.queue.buffer[idx].s_time == req->s_time
               && lock.queue.buffer[idx].s_id < req->s_id) {
            idx++;
        }
    }
    print_queue(self);
    debug_worker_print(
        "[local_id=%d] push req [%d, %d] to idx %d\n", self->local_id, req->s_time, req->s_id, idx
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
    if (idx < lock.queue.size) {
        memcpy(
            lock.queue.buffer + idx + 1, lock.queue.buffer + idx,
            sizeof(LockRequest) * (lock.queue.size - idx)
        );
    }
    lock.queue.buffer[idx] = *req;
    lock.queue.size++;
    print_queue(self);
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
    print_queue(self);
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
    print_reply_at(self);
    for (int i = 1; i < self->proc_n; ++i) {
        if (i == self->local_id) continue;
        if (self->last_recv_at[i] <= lock.active_request.s_time) return 0;
    }
    return 1;
}

/**
 * @brief      Determines ability to activate lock.
 *
 * @param[in]  self  The object
 *
 * @return     True if able to activate lock, False otherwise.
 */
int can_activate_lock(const executor* self) {
    if (lock.state != LOCK_WAITING) return 0;
    if (lock.queue.size < 1) return 0;
    if (lock.queue.buffer[0].s_id != lock.active_request.s_id) return 0;
    if (!is_all_replied_after_request(self)) return 0;
    return 1;
}

/**
 * @brief      Determines if other lock request has priority.
 *
 * @param      orig_req  The original request
 * @param      oth_req   The oth request
 *
 * @return     True if other lock request has priority, False otherwise.
 */
int has_other_lock_req_priority(LockRequest* orig_req, LockRequest* oth_req) {
    if (oth_req->s_time < orig_req->s_time) return 1;
    return (oth_req->s_time == orig_req->s_time) && (oth_req->s_id < orig_req->s_id);
}

void defer_reply(executor* self, local_id dst) {
    lock.queue.deffered[dst] = LOCK_REPLY_PENDING;
    debug_worker_print(debug_lock_defered_reply_fmt, get_lamport_time(), self->local_id);
}

void on_request_cs(executor* self, Message* msg, local_id from) {
    LockRequest req = {.s_id = from, .s_time = msg->s_header.s_local_time};
    switch (lock.state) {
        case LOCK_ACTIVE:
            defer_reply(self, from);
            debug_worker_print(debug_lock_wait_fmt, get_lamport_time(), self->local_id);
            return;
        case LOCK_WAITING: {
            if (has_other_lock_req_priority(&lock.active_request, &req)) {
                send_reply_cs_msg(self, from);
            } else {
                defer_reply(self, from);
            }
            return;
        }
        case LOCK_INACTIVE:
            send_reply_cs_msg(self, from);
            return;
    }
}

void on_reply_cs(executor* self, Message* msg, local_id from) {
    // do nothing
}

void on_release_cs(executor* self, Message* msg, local_id from) {
    pop_request(self, from);
}

void send_deffered(executor* self) {
    for (local_id s_id = 1; s_id <= MAX_PROCESS_ID; ++s_id) {
        if (lock.queue.deffered[s_id] == LOCK_REPLY_SENT) continue;
        send_reply_cs_msg(self, s_id);
        lock.queue.deffered[s_id] = LOCK_REPLY_SENT;
    }
}

int request_cs(const void* self) {
    executor* s_executor = (executor*)self;
    if (lock.state == LOCK_ACTIVE) return 0;
    if (lock.state == LOCK_INACTIVE) {
        clean_replied(self);
        send_request_cs_msg_multicast(s_executor);
        lock.state = LOCK_WAITING;
        LockRequest req = {.s_id = s_executor->local_id, .s_time = get_lamport_time()};
        lock.active_request = req;
        debug_worker_print(debug_lock_wait_fmt, get_lamport_time(), s_executor->local_id);
        wait_receive_all_child_msg_by_type(s_executor, CS_REPLY, on_message);
        lock.state = LOCK_ACTIVE;
        debug_worker_print(debug_lock_acquire_fmt, get_lamport_time(), s_executor->local_id);
        // push_request(s_executor, &req);
        return 0;
    }
    return 1;
}

int release_cs(const void* self) {
    executor* s_executor = (executor*)self;
    if (lock.state == LOCK_WAITING) return 1;
    if (lock.state == LOCK_INACTIVE) return 0;
    if (lock.state == LOCK_ACTIVE) {
        send_deffered(s_executor);
        // pop_request(s_executor, s_executor->local_id);
        lock.state = LOCK_INACTIVE;
        debug_worker_print(debug_lock_released_fmt, get_lamport_time(), s_executor->local_id);
    }
    return 0;
}

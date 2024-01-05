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
        buffer, debug_lock_queue_fmt, self->local_id, self->lock.active_request.s_time,
        self->lock.queue.size
    );
    for (int i = 0; i < self->lock.queue.size; ++i) {
        printed += sprintf(
            buffer + printed, debug_lock_queue_part_fmt, self->lock.queue.buffer[i].s_time,
            self->lock.queue.buffer[i].s_id
        );
    }
    printf("%s\n", buffer);
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
    if (idx < self->lock.queue.size) {
        memcpy(
            self->lock.queue.buffer + idx + 1, self->lock.queue.buffer + idx,
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
    if (idx < self->lock.queue.size - 1) {
        memcpy(
            self->lock.queue.buffer + idx, self->lock.queue.buffer + idx + 1,
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
    for (local_id s_id = 1; s_id < self->proc_n; ++s_id) {
        self->lock.queue.deffered[s_id] = LOCK_REPLY_SENT;
    }
}

/**
 * @brief      Determines ability to activate self->lock.
 *
 * @param[in]  self  The object
 *
 * @return     True if able to activate lock, False otherwise.
 */
int can_activate_lock(executor* self) {
    // if (self->lock.state != LOCK_WAITING) return 0;
    if (self->lock.queue.size < 1) return 0;
    if (self->lock.queue.buffer[0].s_id != self->lock.active_request.s_id) return 0;
    // if (!is_all_replied_after_request(self)) return 0;
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
    self->lock.queue.deffered[dst] = LOCK_REPLY_PENDING;
    debug_worker_print(debug_lock_defered_reply_fmt, get_lamport_time(), self->local_id);
}

void on_request_cs(void* s_self, Message* msg, local_id from) {
    executor*   self = s_self;
    LockRequest req = {.s_id = from, .s_time = msg->s_header.s_local_time};
    switch (self->lock.state) {
        case LOCK_ACTIVE:
            defer_reply(self, from);
            return;
        case LOCK_WAITING: {
            if (has_other_lock_req_priority(&self->lock.active_request, &req)) {
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

void on_reply_cs(void* s_self, Message* msg, local_id from) {
    // executor* self = s_self;
    // do nothing
}

void on_release_cs(void* s_self, Message* msg, local_id from) {
    executor* self = s_self;
    pop_request(self, from);
}

void send_deffered(executor* self) {
    for (local_id s_id = 1; s_id < self->proc_n; ++s_id) {
        if (self->lock.queue.deffered[s_id] == LOCK_REPLY_SENT) continue;
        send_reply_cs_msg(self, s_id);
        self->lock.queue.deffered[s_id] = LOCK_REPLY_SENT;
    }
}

int request_cs(const void* self) {
    executor* s_executor = (executor*)self;
    if (s_executor->lock.state == LOCK_ACTIVE) return 0;
    if (s_executor->lock.state == LOCK_INACTIVE) {
        send_request_cs_msg_multicast(s_executor);
        s_executor->lock.state = LOCK_WAITING;
        LockRequest req = {.s_id = s_executor->local_id, .s_time = get_lamport_time()};
        s_executor->lock.active_request = req;
        debug_worker_print(debug_lock_wait_fmt, get_lamport_time(), s_executor->local_id);

        wait_receive_all_child_msg_by_type(s_executor, CS_REPLY, on_message);
        // wait_receive_all_child_msg_after(s_executor, req.s_time, on_message);
        // while (!can_activate_lock(s_executor)) {
        //     if (receive_any_cb(s_executor, on_message) != 0) usleep(SLEEP_RECEIVE_USEC);
        // }

        s_executor->lock.state = LOCK_ACTIVE;
        debug_worker_print(debug_lock_acquire_fmt, get_lamport_time(), s_executor->local_id);
        // push_request(s_executor, &req);
        return 0;
    }
    return 1;
}

int release_cs(const void* self) {
    executor* s_executor = (executor*)self;
    if (s_executor->lock.state == LOCK_WAITING) return 1;
    if (s_executor->lock.state == LOCK_INACTIVE) return 0;
    if (s_executor->lock.state == LOCK_ACTIVE) {
        // pop_request(s_executor, s_executor->local_id);
        s_executor->lock.state = LOCK_INACTIVE;
        debug_worker_print(debug_lock_released_fmt, get_lamport_time(), s_executor->local_id);
        send_deffered(s_executor);
    }
    return 0;
}

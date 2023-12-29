#include "time.h"

#include "banking.h"
#include "debug.h"
#include "ipc.h"
#include "util.h"

static timestamp_t __local_time = MIN_T;

void next_tick(timestamp_t other_time) {
    timestamp_t prev = __local_time;
    __local_time = max_v(other_time + 1, __local_time + 1);
    debug_print(debug_time_next_tick_fmt, prev, other_time, __local_time);
}

timestamp_t get_lamport_time() {
    return __local_time;
}

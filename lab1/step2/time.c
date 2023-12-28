#include "time.h"

#include "banking.h"
#include "ipc.h"
#include "util.h"

static timestamp_t __local_time = MIN_T;

void next_tick(timestamp_t other_time) {
    __local_time = max_v(other_time + 1, __local_time + 1);
}

timestamp_t get_lamport_time() {
    return __local_time;
}

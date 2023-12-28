#include "channels.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "executor.h"
#include "ipc.h"
#include "logger.h"

int init_channel(channel *channel) {
    int fd[2];
    int res = pipe(fd);
    if (res < 0) return 1;

    // set non blocking flag for read pipe handler
    int flags = fcntl(fd[0], F_GETFD);
    flags |= O_NONBLOCK;
    if (fcntl(fd[0], F_SETFL, flags)) return 1;

    channel->read_h = fd[0];
    channel->write_h = fd[1];
    debug_print(debug_channel_init_fmt, fd[0], fd[1]);

    return fd[0] > 0 && fd[1] > 0 ? 0 : 1;
}

int open_channel(channel **channels, local_id from, local_id dst) {
    channel *ch = &channels[from][dst];
    ch->read_h = 0;
    ch->write_h = 0;
    int rc = init_channel(ch);
    debug_print(debug_channel_open_fmt, from, dst, rc, ch->read_h, ch->write_h);
    return rc;
}

int open_channels(int8_t proc_n, channel **channels) {
    debug_print(debug_channel_open_start_fmt, proc_n);
    for (int local_id = 0; local_id < proc_n; ++local_id) {
        for (int other_id = 0; other_id < proc_n; ++other_id) {
            // do not open channel for itself
            if (other_id == local_id) continue;

            // channel local_id -> other_id
            if (open_channel(channels, local_id, other_id) != 0) return 1;
            log_channel_opened(local_id, other_id);
        }
    }
    return 0;
}

void close_channel_handler(channel_h *channel_h) {
    if (*channel_h != -1) {
        close(*channel_h);
        *channel_h = -1;
    }
}

int close_channel(channel **channels, local_id from, local_id dst) {
    close_channel_handler(&channels[from][dst].read_h);
    close_channel_handler(&channels[from][dst].write_h);
    return 0;
}

int close_channels(int8_t proc_n, channel **channels) {
    for (int from = 0; from < proc_n; ++from) {
        for (int dst = 0; dst < proc_n; ++dst) {
            if (from == dst) continue;
            close_channel(channels, from, dst);
            log_channel_closed(from, dst);
        }
    }
    return 0;
}

int close_unused_channels(int8_t proc_n, local_id local_id, channel **channels) {
    // closed unused pipes. Like in example
    // https://www.man7.org/linux/man-pages/man2/pipe.2.html#EXAMPLES proc with
    // local_id uses only local_id -> other_id write side other_id -> local_id
    // read size
    for (int other_id = 0; other_id < proc_n; ++other_id) {
        if (other_id == local_id) continue;
        for (int other_id_2 = 0; other_id_2 < proc_n; ++other_id_2) {
            // in proc with local_id we do not use channels from other procs
            if (other_id == other_id_2) continue;
            if (local_id == other_id_2) continue;
            close_channel(channels, other_id, other_id_2);
            close_channel(channels, other_id_2, other_id);
        }
        // close unused read end from local_id -> other_id
        close_channel_handler(&channels[local_id][other_id].read_h);
        // close unused write end from other_id -> local_id
        close_channel_handler(&channels[other_id][local_id].write_h);
    }
    return 0;
}

void set_executor_channels(int8_t proc_n, void *self, channel **channels) {
    executor *executor = self;
    local_id  local_id = executor->local_id;

    executor->ch_read = malloc(proc_n * sizeof(channel_h));
    executor->ch_write = malloc(proc_n * sizeof(channel_h));

    executor->ch_read[local_id] = 0;
    executor->ch_write[local_id] = 0;

    for (int other_id = 0; other_id < proc_n; ++other_id) {
        // do not set channel for itself
        if (other_id == local_id) continue;
        // read handler from channel other_id -> local_id
        executor->ch_read[other_id] = channels[other_id][local_id].read_h;
        // read handler from channel local_id -> other_id
        executor->ch_write[other_id] = channels[local_id][other_id].write_h;
        debug_print(
            debug_channel_set_fmt, local_id, 'w', local_id, other_id,
            channels[local_id][other_id].write_h
        );
        debug_print(
            debug_channel_set_fmt, local_id, 'r', other_id, local_id,
            channels[other_id][local_id].read_h
        );
    }
}

channel_h get_channel_read_h(void *self, local_id from) {
    executor *executor = self;
    return executor->ch_read[from];
}

channel_h get_channel_write_h(void *self, local_id dst) {
    executor *executor = self;
    return executor->ch_write[dst];
}

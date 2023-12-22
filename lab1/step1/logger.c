#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "logger.h"
#include "common.h"
#include "debug.h"


int open_log_f(const char *fname) {
    return open(fname, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP );
}

int open_pipes_log_f() {
    return open_log_f(pipes_log);
}

int open_events_log_f() {
    return open_log_f(events_log);
}

int log_file_msg(int fh, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    size_t bufsz = vsnprintf(NULL, 0, fmt, args);
    debug_print("log_file_msg [bufsz=%lu]\n", bufsz);

    if (bufsz < 1) return -1;
    bufsz += 1;

    char * buf;
    if ((buf = malloc(bufsz)) == NULL) return -1;
    vsnprintf(buf, bufsz, fmt, args_copy);
    va_end(args_copy);
    write(fh, buf, bufsz - 1);
    free(buf);
    return 0;
}

int log_msg(int fh, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    vprintf(fmt, args_copy);
    log_file_msg(fh, fmt, args);
    va_end(args_copy);
    return 0;
}

int log_pipes_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int fh = open_pipes_log_f();
    log_msg(fh, fmt, args);
    close(fh);
    va_end(args);
    return 0;
}

int log_events_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int fh = open_events_log_f();
    log_msg(fh, fmt, args);
    close(fh);
    va_end(args);
    return 0;
}

int log_channel_opened(int from, int dst) {
    return log_pipes_msg(log_channel_opened_fmt, from, dst);
}

int log_channel_closed(int from, int dst) {
    return log_pipes_msg(log_channel_closed_fmt, from, dst);
}


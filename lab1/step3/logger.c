#include "logger.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "debug.h"

static int events_log_fd = 0;
static int pipes_log_fd = 0;

int open_log_f(const char *fname) {
    return open(fname, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
}

int open_pipes_log_f() {
    pipes_log_fd = open_log_f(pipes_log);
    debug_print(debug_log_open_file_fmt, pipes_log, pipes_log_fd);
    return pipes_log_fd;
}

int open_events_log_f() {
    events_log_fd = open_log_f(events_log);
    debug_print(debug_log_open_file_fmt, events_log, events_log_fd);
    return events_log_fd;
}

void close_pipes_log_f() {
    close(pipes_log_fd);
}

void close_events_log_f() {
    close(events_log_fd);
}

int log_file_msg(int fd, const char *fmt, va_list args) {
    if (!fd) return 1;

    va_list args_copy;
    va_copy(args_copy, args);

    size_t bufsz = vsnprintf(NULL, 0, fmt, args);
    debug_print(debug_log_msg_file_fmt, fd, bufsz);

    if (bufsz < 1) return -1;
    bufsz += 1;

    char *buf;
    if ((buf = malloc(bufsz)) == NULL) return -1;
    vsnprintf(buf, bufsz, fmt, args_copy);
    va_end(args_copy);
    write(fd, buf, bufsz - 1);
    free(buf);
    return 0;
}

int log_msg(int fd, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    vprintf(fmt, args_copy);
    fflush(stdout);
    int rc = log_file_msg(fd, fmt, args);
    va_end(args_copy);
    return rc;
}

int log_pipes_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int rc = log_msg(pipes_log_fd, fmt, args);
    va_end(args);
    return rc;
}

int log_events_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int rc = log_msg(events_log_fd, fmt, args);
    va_end(args);
    return rc;
}

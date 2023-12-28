#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static int is_debug_enabled = 0;
static int is_debug_ipc_enabled = 0;
static int is_debug_time_enabled = 0;
static int is_debug_worker_enabled = 0;

void set_debug(int is_enabled) {
    is_debug_enabled = is_enabled;
    is_debug_ipc_enabled = is_enabled;
    is_debug_time_enabled = is_enabled;
}

void set_debug_ipc(int is_enabled) {
    is_debug_ipc_enabled = is_enabled;
}

void set_debug_time(int is_enabled) {
    is_debug_time_enabled = is_enabled;
}

void set_debug_worker(int is_enabled) {
    is_debug_worker_enabled = is_enabled;
}

void debug_print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (is_debug_enabled) vprintf(fmt, args);
    va_end(args);
}

void debug_ipc_print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (is_debug_ipc_enabled) vprintf(fmt, args);
    va_end(args);
}

void debug_time_print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (is_debug_time_enabled) vprintf(fmt, args);
    va_end(args);
}

void debug_worker_print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (is_debug_worker_enabled) vprintf(fmt, args);
    va_end(args);
}

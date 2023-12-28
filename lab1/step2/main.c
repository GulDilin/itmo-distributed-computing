#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Include pre defined libraries for lab implementation
#include "args.h"
#include "channels.h"
#include "common.h"
#include "debug.h"
#include "ipc.h"
#include "logger.h"
#include "worker.h"

int is_parent(pid_t parent_pid) {
    return getpid() == parent_pid;
}

void create_child_process(
    int proc_n, pid_t parent_pid, int local_id, executor *executor, channel **channels
) {
    // fork only main parent process
    if (!is_parent(parent_pid)) return;
    pid_t forked_pid = fork();
    if (forked_pid == 0) {
        // forked process
        pid_t pid = getpid();
        pid_t p_pid = getppid();
        init_executor(executor, channels, local_id, proc_n, pid, p_pid, 30);
        debug_print(debug_forked_fmt, pid, p_pid, local_id);
    }
}

void create_processes(int proc_n, pid_t parent_pid, executor *executor, channel **channels) {
    debug_print(debug_forked_fmt, getpid(), getppid(), PARENT_ID);
    debug_print(debug_start_fork_fmt, getpid());

    for (int local_id = 1; local_id < proc_n; ++local_id) {
        create_child_process(proc_n, parent_pid, local_id, executor, channels);
    }
    if (is_parent(parent_pid)) {
        init_executor(executor, channels, PARENT_ID, proc_n, getpid(), 0, 0);
    }
    debug_print(debug_proc_created_fmt, getpid());
}

void init(int proc_n, channel ***channels) {
    *channels = malloc(proc_n * sizeof(channel *));
    for (int i = 0; i < proc_n; ++i) { (*channels)[i] = malloc(proc_n * sizeof(channel)); }
    debug_print(debug_malloc_ch_fin_fmt, (void *)*channels);
    if (*channels == NULL) {
        perror("Failed to create channels");
        exit(1);
    }
    if (open_channels(proc_n, *channels) != 0) {
        perror("Failed to open channels");
        exit(1);
    };
    open_pipes_log_f();
    open_events_log_f();
}

void cleanup(int proc_n, channel **channels, executor *executor) {
    if (executor->local_id == PARENT_ID) {
        // wait all child processes
        while (wait(NULL) > 0) {}
        close_channels(proc_n, channels);
    }
    free(executor->ch_read);
    free(executor->ch_write);
    for (int i = 0; i < proc_n; ++i) free(channels[i]);
    free(channels);
    close_pipes_log_f();
    close_events_log_f();
}

void set_debug_args(arguments *arguments) {
    set_debug(arguments->debug);
    set_debug_ipc(arguments->debug_ipc);
    set_debug_time(arguments->debug_time);
}

int main(int argc, char **argv) {
    debug_print(debug_main_start_fmt, 0);
    arguments arguments;
    debug_print(debug_main_args_parse_fmt, argc);
    args_parse(argc, argv, &arguments);
    set_debug_args(&arguments);
    debug_print(debug_main_args_parsed_fmt, argc, arguments.proc_n);

    channel **channels;
    init(arguments.proc_n, &channels);

    executor executor;
    pid_t    parent_pid = getpid();
    create_processes(arguments.proc_n, parent_pid, &executor, channels);
    // if (is_parent(parent_pid)) close_unused_channels(arguments.proc_n, PARENT_ID, channels);

    debug_print(debug_executor_info_fmt, executor.pid, executor.parent_pid, executor.local_id);
    run_worker(&executor);

    cleanup(arguments.proc_n, channels, &executor);
    return 0;
}

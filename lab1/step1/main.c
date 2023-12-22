#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

// Include pre defined libraries for lab implementation
#include "ipc.h"
#include "common.h"
#include "pa1.h"
#include "worker.h"
#include "channels.h"
#include "debug.h"
#include "args.h"

void create_child_process(int proc_n, pid_t parent_pid, int local_id, executor * executor, channel ** channels) {
    // fork only main parent process
    if (getpid() != parent_pid) return;
    pid_t forked_pid = fork();
    if (forked_pid == 0) {
        // forked process
        pid_t pid = getpid();
        pid_t p_pid = getppid();

        executor -> local_id = local_id;
        executor -> proc_n = proc_n;
        executor -> parent_pid = p_pid;
        executor -> pid = pid;

        set_executor_channels(proc_n, executor, channels);
        debug_print("Hello from Proc pid=%d parent=%d local_id=%d\n", pid, p_pid, local_id);
    }
}


void create_processes(int proc_n, pid_t parent_pid, executor * executor, channel ** channels) {
    debug_print("Hello from Proc pid=%d parent=%d local_id=%d\n", getpid(), getppid(), PARENT_ID);
    debug_print("[pid=%d] Start create processes\n", getpid());

    executor -> local_id = PARENT_ID;
    executor -> proc_n = proc_n;
    executor -> parent_pid = 0;
    executor -> pid = getpid();

    set_executor_channels(proc_n, executor, channels);

    for (int local_id = 1; local_id < proc_n; ++local_id) {
        create_child_process(proc_n, parent_pid, local_id, executor, channels);
    }
    debug_print("[pid=%d] Processes created\n", getpid());
}

void init(int proc_n, channel *** channels) {
    *channels = malloc(proc_n * sizeof(channel*));
    for (int i = 0; i < proc_n; ++i) {
        (*channels)[i] = malloc( proc_n * sizeof(channel));
    }
    debug_print("malloc channels finished [channels=%p]\n", (void *) *channels);
    if (*channels == NULL) {
        perror("Failed to create channels");
        exit(1);
    }
    if (open_channels(proc_n, *channels) != 0) {
        perror("Failed to open channels");
        exit(1);
    };
}

void cleanup(int proc_n, channel ** channels, executor * executor) {
    if (executor -> local_id == PARENT_ID) {
        close_channels(proc_n, channels);
    }
    free(executor->ch_read);
    free(executor->ch_write);
    for (int i = 0; i < proc_n; ++i) free(channels[i]);
    free(channels);
}

int main(int argc, char **argv) {
    debug_print("Start %d\n", 0);

    arguments arguments;
    debug_print("Parse args %d\n", argc);
    args_parse(argc, argv, &arguments);
    debug_print("Parsed args %d. processes: %d\n", argc, arguments.proc_n);

    channel ** channels;
    init(arguments.proc_n, &channels);

    executor executor;
    pid_t parent_pid = getpid();
    create_processes(arguments.proc_n, parent_pid, &executor, channels);

    debug_print("Executor pid=%d parent=%d local_id=%d\n", executor.pid, executor.parent_pid, executor.local_id);
    run_worker(&executor);

    cleanup(arguments.proc_n, channels, &executor);
    return 0;
}

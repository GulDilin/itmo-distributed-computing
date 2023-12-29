#include "args.h"

#include <argp.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BALANCE 65535

const char *argp_program_version = "pa1";
const char *argp_program_bug_address = "zhenyagurin@gmail.com";

/* Program documentation. */
static char doc[] = "Distributed computing lab1 step1 -- a distributed program "
                    "communicates with pipes";

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* The options we understand. */
static struct argp_option options[] = {
    {"process", 'p', "NUMBER OF PROCESSES", 0, "Amount of processes (2-15)"},
    {"debug", 'd', 0, OPTION_ARG_OPTIONAL, "Enable debug messages"},
    {"debug-ipc", 'i', 0, OPTION_ARG_OPTIONAL, "Enable debug messages for IPC"},
    {"debug-time", 't', 0, OPTION_ARG_OPTIONAL, "Enable debug messages for TIME"},
    {"debug-worker", 'w', 0, OPTION_ARG_OPTIONAL, "Enable debug messages for WORKER"},
    {0}
};

static const char *argp_err_key_nan_fmt = "-%c is not a number. See --help for more information";
static const char *arg_err_key_required_fmt = "-%c is required. See --help for more information";
static const char *arg_err_key_range_fmt
    = "-%c value is not in correct range. See --help for more information";
static const char *arg_err_too_many_args_fmt = "Too many arguments for start balance. Should be same as number of processes. See --help for more information";
static const char *arg_err_not_enough_args_fmt = "Not enough arguments for start balance. Should be same as number of processes. See --help for more information";

/**
 * @brief      Parse a single option.
 *
 * @param[in]  key    The key
 * @param      arg    The argument
 * @param      state  The state
 *
 * @return     The error.
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    arguments *arguments = state->input;

    switch (key) {
        case 'p': {
            if (arg == NULL) {
                // not passed
                argp_failure(state, 1, 0, arg_err_key_required_fmt, key);
                argp_usage(state);
                return ARGP_ERR_UNKNOWN;
            }

            char    *endptr = NULL;
            long int proc_n = strtol(arg, &endptr, 10);
            if (*endptr != 0) {
                // non-number letters after number
                argp_failure(state, 1, 0, argp_err_key_nan_fmt, key);
                return ARGP_ERR_UNKNOWN;
            }
            if (proc_n < 2 || proc_n > 15) {
                // not in range
                argp_failure(state, 1, 0, arg_err_key_range_fmt, key);
                return ARGP_ERR_UNKNOWN;
            }
            arguments->proc_n = (uint8_t)proc_n + 1;
            break;
        }

        case 'd':
            arguments->debug = 1;
            arguments->debug_ipc = 1;
            arguments->debug_time = 1;
            arguments->debug_worker = 1;
            break;

        case 'i':
            arguments->debug_ipc = 1;
            break;

        case 't':
            arguments->debug_time = 1;
            break;

        case 'w':
            arguments->debug_worker = 1;
            break;

        case ARGP_KEY_ARG:
            {
            if (state->arg_num >= arguments->proc_n) {
                argp_failure(state, 1, 0, "%s", arg_err_too_many_args_fmt);
                return ARGP_ERR_UNKNOWN;
            }
            char    *endptr = NULL;
            long int start_balance = strtol(arg, &endptr, 10);
            if (*endptr != 0) {
                // non-number letters after number
                argp_failure(state, 1, 0, argp_err_key_nan_fmt, ' ');
                return ARGP_ERR_UNKNOWN;
            }
            if (start_balance > MAX_BALANCE || start_balance < 1) {
                argp_failure(state, 1, 0, arg_err_key_range_fmt, ' ');
                return ARGP_ERR_UNKNOWN;
            }

            arguments->start_balance[state->arg_num] = (uint16_t)start_balance;
            break;
        }

        case ARGP_KEY_END:
            // check if not enough args
            if (arguments->proc_n == 0) {
                argp_failure(state, 1, 0, arg_err_key_required_fmt, 'p');
                exit(ARGP_ERR_UNKNOWN);
            }
            if (state->arg_num >= arguments->proc_n) {
                argp_failure(state, 1, 0, "%s", arg_err_too_many_args_fmt);
                return ARGP_ERR_UNKNOWN;
            }
            if (state->arg_num < arguments->proc_n - 1) {
                argp_failure(state, 1, 0, "%s", arg_err_not_enough_args_fmt);
                return ARGP_ERR_UNKNOWN;
            }

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

void init_defaults(arguments *arguments) {
    arguments->proc_n = 0;
    arguments->debug = 0;
    arguments->debug_ipc = 0;
    arguments->debug_time = 0;
    arguments->debug_worker = 0;
}

void args_parse(int argc, char **argv, arguments *arguments) {
    init_defaults(arguments);
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}

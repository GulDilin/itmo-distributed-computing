#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>

#include "args.h"

const char *argp_program_version = "pa1";
const char *argp_program_bug_address = "zhenyagurin@gmail.com";

/* Program documentation. */
static char doc[] = "Distributed computing lab1 step1 -- a distributed program communicates with pipes";

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* The options we understand. */
static struct argp_option options[] = {
  {"process",  'p',   "NUMBER OF PROCESSES",  0,  "Amount of processes (2-15)" },
  { 0 }
};

/**
 * @brief      Parse a single option.
 *
 * @param[in]  key    The key
 * @param      arg    The argument
 * @param      state  The state
 *
 * @return     The error.
 */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  arguments *arguments = state->input;

  switch (key) {
    case 'p': {
      if (arg == NULL) {
        // not passed
        argp_usage (state);
        return 1;
      }

      char *endptr = NULL;
      long int proc_n = strtol(arg, &endptr, 10);
      if(*endptr != 0) {
        // non-number letters after number
        argp_usage (state);
        return 1;
      }
      if (proc_n < 2 || proc_n > 15) {
        // not in range
        argp_usage (state);
        return 1;
      }
      arguments->proc_n = (uint8_t) proc_n + 1;
      break;
    }

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

void args_parse(int argc, char **argv, arguments * arguments) {
  argp_parse(&argp, argc, argv, 0, 0, arguments);
}

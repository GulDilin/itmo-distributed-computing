/**
 * @file     time.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Functions for time tick
 */

#ifndef __IFMO_DISTRIBUTED_CLASS_TIME__H
#define __IFMO_DISTRIBUTED_CLASS_TIME__H

#include <unistd.h>
#include "ipc.h"

#define TIME_UNSET 0
#define MIN_T      0

void next_tick(timestamp_t other_time);

int usleep(__useconds_t useconds);

#endif  // __IFMO_DISTRIBUTED_CLASS_TIME__H

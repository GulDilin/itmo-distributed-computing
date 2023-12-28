/**
 * @file     time.h
 * @Author   Gurin Evgeny and Kamyshanskaya Kseniia
 * @brief    Functions for time tick
 */

#ifndef __IFMO_DISTRIBUTED_CLASS_TIME__H
#define __IFMO_DISTRIBUTED_CLASS_TIME__H

#include "ipc.h"

#define TIME_UNSET 0
#define MIN_T      1

void next_tick(timestamp_t other_time);

#endif  // __IFMO_DISTRIBUTED_CLASS_TIME__H

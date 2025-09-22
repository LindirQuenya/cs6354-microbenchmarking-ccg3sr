#ifndef MICROBENCH_01_CONTEXT_SWITCH_H
#define MICROBENCH_01_CONTEXT_SWITCH_H

#include "stats.h"

calibrated_stats contextswitch_syscall(int iterations);
calibrated_stats contextswitch_thread(int iterations);
void * contextswitch_ping(void * ptr);
void * contextswitch_pong(void * ptr);
void contextswitch_pingpong(int *store);
#endif
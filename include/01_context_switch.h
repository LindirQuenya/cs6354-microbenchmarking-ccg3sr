#ifndef MICROBENCH_01_CONTEXT_SWITCH_H
#define MICROBENCH_01_CONTEXT_SWITCH_H

#include "stats.h"

typedef struct {
    calibrated_stats syscall;
    calibrated_stats thread;
} contextswitch_stats;

void *contextswitch_ping(void *ptr);
void *contextswitch_pong(void *ptr);
void contextswitch_pingpong(int *store);

contextswitch_stats context_switch(int iterations);
void storeResults_01(contextswitch_stats stats);
void displayResults_01(contextswitch_stats stats);
#endif
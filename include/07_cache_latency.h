#ifndef MICROBENCH_07_CACHE_LATENCY
#define MICROBENCH_07_CACHE_LATENCY

#include "stats.h"

typedef struct {
    calibrated_stats l1i;
    calibrated_stats l1d;
    calibrated_stats l2;
    calibrated_stats l3;
} cache_latency_stats;

cache_latency_stats cache_latency(int iterations);

#endif
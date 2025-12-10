#ifndef MICROBENCH_08_CACHE_BANDWIDTH
#define MICROBENCH_08_CACHE_BANDWIDTH

#include "stats.h"

#define CACHEBW_LOOPS_CALIBRATION (8)
#define CACHEBW_LOOPS_MEASUREMENT (16)

typedef struct {
    // 4k
    stats_rw_pair l1d;
    // 64k
    stats_rw_pair l2;
    // 512k
    stats_rw_pair l3;
} cache_bandwidth_stats;

cache_bandwidth_stats cache_bandwidth(int iterations);

void storeResults_08(cache_bandwidth_stats stats);
void displayResults_08(cache_bandwidth_stats stats);

#endif
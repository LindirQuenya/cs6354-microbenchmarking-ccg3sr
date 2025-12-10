#ifndef MICROBENCH_10_DRAM_BANDWIDTH
#define MICROBENCH_10_DRAM_BANDWIDTH

#include "stats.h"

#define DRAMBW_LOOPS_CALIBRATION (8)
#define DRAMBW_LOOPS_MEASUREMENT (16)

typedef struct {
    // 32M
    stats_rw_pair dram;
} dram_bandwidth_stats;

dram_bandwidth_stats dram_bandwidth(int iterations);

void storeResults_10(dram_bandwidth_stats stats);
void displayResults_10(dram_bandwidth_stats stats);

#endif
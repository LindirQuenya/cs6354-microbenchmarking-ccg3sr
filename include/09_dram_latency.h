#ifndef MICROBENCH_09_DRAM_LATENCY
#define MICROBENCH_09_DRAM_LATENCY

#include "stats.h"

calibrated_stats dram_latency(int iterations);
void storeResults_09(calibrated_stats stats);
void displayResults_09(calibrated_stats stats);

#endif
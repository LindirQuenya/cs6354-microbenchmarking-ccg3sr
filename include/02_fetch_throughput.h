#ifndef MICROBENCH_02_FETCH_THROUGHPUT
#define MICROBENCH_02_FETCH_THROUGHPUT

#include "stats.h"

#define FETCH_LOOPS_CALIBRATION 10000
#define FETCH_LOOPS_MEASUREMENT 20000

typedef struct {
    calibrated_stats fetch8;
    calibrated_stats fetch16;
} fetch_throughput_stats;

fetch_throughput_stats fetch_throughput(int iterations);
void storeResults_02(fetch_throughput_stats stats);
void displayResults_02(fetch_throughput_stats stats);

#endif
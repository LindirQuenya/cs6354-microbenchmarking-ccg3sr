#ifndef MICROBENCH_03_RETIRE_THROUGHPUT
#define MICROBENCH_03_RETIRE_THROUGHPUT

#include "stats.h"

#define RETIRE_LOOPS_CALIBRATION 10000
#define RETIRE_LOOPS_MEASUREMENT 20000

typedef struct {
    calibrated_stats lowILP;
    calibrated_stats highILP;
} retire_stats;

retire_stats retire_throughput(int iterations);
void storeResults_03(retire_stats stats);
void displayResults_03(retire_stats stats);

#endif
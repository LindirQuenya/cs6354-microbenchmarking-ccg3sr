#ifndef MICROBENCH_04_LOAD_STORE_THROUGHPUT
#define MICROBENCH_04_LOAD_STORE_THROUGHPUT

#include "stats.h"

#define LOADSTORE_LOOPS_CALIBRATION 10000
#define LOADSTORE_LOOPS_MEASUREMENT 20000

typedef struct {
    calibrated_stats load8;
    calibrated_stats load16;
    calibrated_stats store8;
    calibrated_stats store16;
} load_store_stats;

load_store_stats loadstore_throughput(int iterations);

#endif
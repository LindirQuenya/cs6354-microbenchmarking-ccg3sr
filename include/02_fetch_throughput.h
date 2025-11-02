#ifndef MICROBENCH_02_FETCH_THROUGHPUT
#define MICROBENCH_02_FETCH_THROUGHPUT

#include "stats.h"

#define FETCH_LOOPS_CALIBRATION 10000
#define FETCH_LOOPS_MEASUREMENT 20000

calibrated_stats fetch_throughput(int iterations);


#endif
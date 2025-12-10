#ifndef MICROBENCH_06_EXEC_UNIT_THROUGHPUT
#define MICROBENCH_06_EXEC_UNIT_THROUGHPUT

#include "stats.h"

#define EXECUNIT_LOOPS_CALIBRATION 10000
#define EXECUNIT_LOOPS_MEASUREMENT 20000

typedef struct {
    calibrated_stats iadd;
    calibrated_stats imul;
    calibrated_stats idiv;
} exec_unit_stats;

exec_unit_stats execunit_throughput(int iterations);
void storeResults_06(exec_unit_stats stats);
void displayResults_06(exec_unit_stats stats);

#endif
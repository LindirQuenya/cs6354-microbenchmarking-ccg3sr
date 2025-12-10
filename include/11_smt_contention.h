#ifndef MICROBENCH_11_SMT_CONTENTION
#define MICROBENCH_11_SMT_CONTENTION

#include "stats.h"

#define SMT_LOOPS_CALIBRATION (100)
#define SMT_LOOPS_MEASUREMENT (200)

typedef struct {
    calibrated_stats competing;
    calibrated_stats complementary;
} smt_contention_stats;

smt_contention_stats smt_contention(int iterations);

void storeResults_11(smt_contention_stats stats);
void displayResults_11(smt_contention_stats stats);

#endif

#ifndef MICROBENCH_05_BRANCH_MISPREDICT
#define MICROBENCH_05_BRANCH_MISPREDICT

#include "stats.h"

calibrated_stats mispredict(int iterations);
void storeResults_05(calibrated_stats stats);
void displayResults_05(calibrated_stats stats);

#endif
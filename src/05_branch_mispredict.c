#include <x86intrin.h>
#include <emmintrin.h>
#include <stdio.h>

#include "harness.h"
#include "stats.h"
#include "05_branch_mispredict.h"

__attribute__((optimize("Og"))) int measure_branches(int rounds,
                                                     int threshold) {
    long long start, end;
    unsigned int tsc_aux;
    for (int i = 0; i < rounds; i++) {
        start = __rdtscp(&tsc_aux);
        _mm_lfence();
        if (i >= threshold) {
            end = __rdtscp(&tsc_aux);
            _mm_lfence();
        }
    }
    return end - start;
}

calibrated_stats mispredict(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    for (int i = 0; i < iterations; i++) {
        times[i] = measure_branches(100, 99);
        times_calib[i] = measure_branches(100, 0);
    }
    calibrated_stats s = calibrated_int_stats(times, times_calib, iterations);
    free(times);
    free(times_calib);
    return s;
}

void storeResults_05(calibrated_stats stats) {
    storeResults(stats.calibration, "05BranchPred_Mispredict_Calibration");
    storeResults(stats.measurement, "05BranchPred_Mispredict_Measurement");
}
void displayResults_05(calibrated_stats stats) {
    printf("\nBranch Penalty: (%d runs)\n", (int)stats.calibration.n_samples);
    printCalibrated(stats);
}
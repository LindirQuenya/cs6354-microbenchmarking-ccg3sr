#include "stats.h"
#include <x86intrin.h>
#include <emmintrin.h>

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
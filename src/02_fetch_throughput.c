#include <x86intrin.h>
#include <emmintrin.h>

#include "02_fetch_throughput.h"
#include "stats.h"
#include "opt.h"

NOINLINE NOUNROLL long loop_nop8(int count) {
    long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        asm(".rept 8 ; nop ; .endr");
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE NOUNROLL long loop_nop16(int count) {
    long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        asm(".rept 16 ; nop ; .endr");
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

// In optimized form, each loop has 10 instructions.
calibrated_stats fetch_throughput8(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_nop8(FETCH_LOOPS_CALIBRATION);
        times[i] = loop_nop8(FETCH_LOOPS_MEASUREMENT);
    }
    calibrated_stats s = calibrated_int_stats(times, times_calib, iterations);
    free(times);
    free(times_calib);

    return s;
}

// In optimized form, each loop has 18 instructions.
calibrated_stats fetch_throughput16(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_nop16(FETCH_LOOPS_CALIBRATION);
        times[i] = loop_nop16(FETCH_LOOPS_MEASUREMENT);
    }
    calibrated_stats s = calibrated_int_stats(times, times_calib, iterations);
    free(times);
    free(times_calib);

    return s;
}
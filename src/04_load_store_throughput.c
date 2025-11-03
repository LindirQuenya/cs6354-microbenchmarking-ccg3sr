#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>

#include "04_load_store_throughput.h"
#include "stats.h"
#include "util.h"
#include "opt.h"

// 11 instructions per iteration, ish.
NOINLINE_NOUNROLL long loop_load8(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        REP8(reg = temp;)
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

// 19 instructions per iteration, ish.
NOINLINE_NOUNROLL long loop_load16(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        REP(0, 1, 6, reg = temp;)
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE_NOUNROLL long loop_store8(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg = temp;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        REP8(temp = reg;)
    }
    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE_NOUNROLL long loop_store16(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg = temp;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        REP(0, 1, 6, temp = reg;)
    }
    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

load_store_stats loadstore_throughput(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);

    load_store_stats s;

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_load8(LOADSTORE_LOOPS_CALIBRATION);
        times[i] = loop_load8(LOADSTORE_LOOPS_MEASUREMENT);
    }
    s.load8 = calibrated_int_stats(times, times_calib, iterations);

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_load16(LOADSTORE_LOOPS_CALIBRATION);
        times[i] = loop_load16(LOADSTORE_LOOPS_MEASUREMENT);
    }
    s.load16 = calibrated_int_stats(times, times_calib, iterations);

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_store8(LOADSTORE_LOOPS_CALIBRATION);
        times[i] = loop_store8(LOADSTORE_LOOPS_MEASUREMENT);
    }
    s.store8 = calibrated_int_stats(times, times_calib, iterations);

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_store16(LOADSTORE_LOOPS_CALIBRATION);
        times[i] = loop_store16(LOADSTORE_LOOPS_MEASUREMENT);
    }
    s.store16 = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);

    return s;
}
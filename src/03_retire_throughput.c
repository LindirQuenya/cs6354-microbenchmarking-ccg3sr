#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>

#include "03_retire_throughput.h"
#include "stats.h"
#include "util.h"
#include "opt.h"

NOINLINE NOUNROLL long loop_lowILP(int count) {
    long long start, end;
    unsigned int tsc_aux;
    double f0 = 4.0, f1 = 2.0;
    volatile long m0 = 0, m1 = 0;
    int i0 = 1, i1 = 1, i2 = 1, i3 = 1;
    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        i1 = m0;
        f0 += i1;
        f1 *= f0;
        f1 += i2;
        f0 = i3 + m1 * f1;
        i2 = f0 + i0;
        i3 += i2;
        m1 = i3;
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE NOUNROLL long loop_highILP(int count) {
    long long start, end;
    unsigned int tsc_aux;
    double f0 = 4.0, f1 = 2.0;
    volatile long m0 = 0, m1 = 0;
    int i0 = 1, i1 = 1, i2 = 1, i3 = 1;
    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        i1 = m0;
        f0 += i2;
        f1 *= i3;
        f0 = i3 + m1 * f0;
        i2 = f1 + i0;
        f1 += i1;
        i3 += i1;
        m1 = i2;
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

retire_stats retire_throughput(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);

    retire_stats s;

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_lowILP(RETIRE_LOOPS_CALIBRATION);
        times[i] = loop_lowILP(RETIRE_LOOPS_MEASUREMENT);
    }
    s.lowILP = calibrated_int_stats(times, times_calib, iterations);

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_highILP(RETIRE_LOOPS_CALIBRATION);
        times[i] = loop_highILP(RETIRE_LOOPS_MEASUREMENT);
    }
    s.highILP = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);

    return s;
}
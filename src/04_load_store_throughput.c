#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <stdio.h>

#include "04_load_store_throughput.h"
#include "stats.h"
#include "util.h"
#include "opt.h"
#include "harness.h"

// 11 instructions per iteration, but only 8 are loads.
NOINLINE NOUNROLL long loop_load8(int count) {
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
    temp = reg;
    return end - start;
}

// 19 instructions per iteration, but only 16 are loads.
NOINLINE NOUNROLL long loop_load16(int count) {
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
    temp = reg;
    return end - start;
}

NOINLINE NOUNROLL long loop_store8(int count) {
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

NOINLINE NOUNROLL long loop_store16(int count) {
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

void storeResults_04(load_store_stats stats) {

    storeResults(stats.load8.calibration, "04LoadStore_Load8_Calibration");
    storeResults(stats.load8.measurement, "04LoadStore_Load8_Measurement");

    storeResults(stats.load16.calibration, "04LoadStore_Load16_Calibration");
    storeResults(stats.load16.measurement, "04LoadStore_Load16_Measurement");

    storeResults(stats.store8.calibration, "04LoadStore_Store8_Calibration");
    storeResults(stats.store8.measurement, "04LoadStore_Store8_Measurement");

    storeResults(stats.store16.calibration, "04LoadStore_Store16_Calibration");
    storeResults(stats.store16.measurement, "04LoadStore_Store16_Measurement");
}
void displayResults_04(load_store_stats stats) {
    int runs = stats.load16.measurement.n_samples;
    int loopdiff = LOADSTORE_LOOPS_MEASUREMENT - LOADSTORE_LOOPS_CALIBRATION;
    printf("\nL/S Throughput (8 loads): (%d runs)\n", runs);
    printCalibrated(stats.load8);
    printf(
        "L/S per cycle (8 loads): %f\n",
        loopdiff * 8.0 /
            (stats.load8.measurement.median - stats.load8.calibration.median));

    printf("\nL/S Throughput (16 loads): (%d runs)\n", runs);
    printCalibrated(stats.load16);
    printf("L/S per cycle (16 loads): %f\n",
           loopdiff * 16.0 /
               (stats.load16.measurement.median -
                stats.load16.calibration.median));

    printf("\nL/S Throughput (8 stores): (%d runs)\n", runs);
    printCalibrated(stats.store8);
    printf("L/S per cycle (8 stores): %f\n",
           loopdiff * 8.0 /
               (stats.store8.measurement.median -
                stats.store8.calibration.median));
    printf("\nL/S Throughput (16 stores): (%d runs)\n", runs);
    printCalibrated(stats.store16);
    printf("L/S per cycle (16 stores): %f\n",
           loopdiff * 16.0 /
               (stats.store16.measurement.median -
                stats.store16.calibration.median));
}
#include <x86intrin.h>
#include <emmintrin.h>
#include <stdio.h>

#include "02_fetch_throughput.h"
#include "stats.h"
#include "opt.h"
#include "harness.h"

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

fetch_throughput_stats fetch_throughput(int iterations) {
    fetch_throughput_stats s;
    s.fetch8 = fetch_throughput8(iterations);
    s.fetch16 = fetch_throughput16(iterations);
    return s;
}

void storeResults_02(fetch_throughput_stats stats) {

    storeResults(stats.fetch8.calibration, "02FetchBW_Fetch8_Calibration");
    storeResults(stats.fetch8.measurement, "02FetchBW_Fetch8_Measurement");
    storeResults(stats.fetch16.calibration, "02FetchBW_Fetch16_Calibration");
    storeResults(stats.fetch16.measurement, "02FetchBW_Fetch16_Measurement");
}
void displayResults_02(fetch_throughput_stats stats) {
    int runs = stats.fetch16.measurement.n_samples;
    int loopdiff = FETCH_LOOPS_MEASUREMENT - FETCH_LOOPS_CALIBRATION;

    printf("\nFetch Throughput (8 nops): (%d runs)\n", runs);
    printCalibrated(stats.fetch8);
    printf("Instr fetched per cycle (8 nops): %f\n",
           loopdiff * 10.0 /
               (stats.fetch8.measurement.median -
                stats.fetch8.calibration.median));

    printf("\nFetch Throughput (16 nops): (%d runs)\n", runs);
    printCalibrated(stats.fetch16);
    printf("Instr fetched per cycle (16 nops): %f\n",
           loopdiff * 18.0 /
               (stats.fetch16.measurement.median -
                stats.fetch16.calibration.median));
}
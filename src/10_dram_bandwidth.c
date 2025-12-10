#include <x86intrin.h>
#include <emmintrin.h>
#include <string.h>
#include <stdio.h>

#include "10_dram_bandwidth.h"
#include "stats.h"
#include "opt.h"
#include "cache.h"
#include "harness.h"

volatile unsigned char *dram_arr;

void setup_drambw(void) {
    _mm_mfence();
    dram_arr = malloc(L3_CACHE_SIZE * 4);
    memset(dram_arr, 'a', L3_CACHE_SIZE * 4);
    __cpuid();
}

NOINLINE long measure_dram_read(int count) {
    long long start, end;
    unsigned int tsc_aux;
    unsigned char a;
    int i, j;

    // Don't need a warmup this time, it'll already be in RAM.

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly read 32M from main memory.
    for (j = 0; j < count; j++) {
        for (i = 0; i < L3_CACHE_SIZE * 4; i++) {
            a = dram_arr[i];
        };
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_dram_write(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L2 (warmup).
    for (i = 0; l2_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly write 32M to main memory.
    for (j = 0; j < count; j++) {
        for (i = 0; i < L3_CACHE_SIZE * 4; i++) {
            dram_arr[i] = 'a';
        }
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

dram_bandwidth_stats dram_bandwidth(int iterations) {
    int i;
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    // Prep the arrays.
    setup_drambw();
    dram_bandwidth_stats s;
    for (i = 0; i < iterations; i++) {
        times[i] = measure_dram_read(DRAMBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_dram_read(DRAMBW_LOOPS_CALIBRATION);
    }
    s.dram.read = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_dram_write(DRAMBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_dram_write(DRAMBW_LOOPS_CALIBRATION);
    }
    s.dram.write = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);
    free(dram_arr);

    return s;
}

void storeResults_10(dram_bandwidth_stats stats) {
    storeResults(stats.dram.read.calibration, "10DRAMBW_Read_Calibration");
    storeResults(stats.dram.read.measurement, "10DRAMBW_Read_Measurement");
    storeResults(stats.dram.write.calibration, "10DRAMBW_Write_Calibration");
    storeResults(stats.dram.write.measurement, "10DRAMBW_Write_Measurement");
}

void displayResults_10(dram_bandwidth_stats stats) {
    int runs = stats.dram.read.measurement.n_samples;
    double loopdiff = DRAMBW_LOOPS_MEASUREMENT - DRAMBW_LOOPS_CALIBRATION;

    printf("\nL1d Read  Time for (%d - %d) * 4KiB: (%d runs)\n",
           DRAMBW_LOOPS_MEASUREMENT, DRAMBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.dram.read);
    printf("L1d Write Time for (%d - %d) * 4KiB: (%d runs)\n",
           DRAMBW_LOOPS_MEASUREMENT, DRAMBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.dram.write);
    printf("Bytes per cycle: %f R, %f W\n",
           32.0 * 1024.0 * 1024.0 * loopdiff /
               (stats.dram.read.measurement.median -
                stats.dram.read.calibration.median),
           32.0 * 1024.0 * 1024.0 * loopdiff /
               (stats.dram.write.measurement.median -
                stats.dram.write.calibration.median));
}
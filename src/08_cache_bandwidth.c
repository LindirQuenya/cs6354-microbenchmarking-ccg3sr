#include <x86intrin.h>
#include <emmintrin.h>
#include <string.h>
#include <stdio.h>

#include "08_cache_bandwidth.h"
#include "stats.h"
#include "opt.h"
#include "cache.h"
#include "harness.h"

// If these aren't marked volatile the compiler optimizes out the cache-filling
// operations.

void setup_cachebw(void) {
    _mm_mfence();
    memset((void *)l1d_arr, 'a', L1d_CACHE_SIZE);
    // All we need when setting these sizes is to ensure that we won't be
    // accidentally reading from a higher-level or lower-level cache.

    // For L1d, we can just use an eighth of the cache. This should keep us
    // from overflowing into L2 - we should just take one way in each set.
    l1d_arr[L1d_CACHE_SIZE / 8] = 0;
    memset((void *)l2_arr, 'a', L2_CACHE_SIZE);
    // For L2, we just need to ensure that we won't be reading from the L1d.
    // Using double the size of the L1d will do that for us.
    l2_arr[L1d_CACHE_SIZE * 2] = 0;
    memset((void *)l3_arr, 'a', L3_CACHE_SIZE);
    // Similarly, using double the size of L2 should keep us in the L3.
    l3_arr[L2_CACHE_SIZE * 2] = 0;
    __cpuid();
}

NOINLINE long measure_l1d_read(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L1d (warmup).
    for (i = 0; l1d_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly read 4K from the L1d.
    for (j = 0; j < count; j++) {
        for (i = 0; l1d_arr[i] != 0; i++);
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_l2_read(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L2 (warmup).
    for (i = 0; l2_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly read 64K from the L2.
    for (j = 0; j < count; j++) {
        for (i = 0; l2_arr[i] != 0; i++);
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_l3_read(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L3 (warmup).
    for (i = 0; l3_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly read 512K from the L3.
    for (j = 0; j < count; j++) {
        for (i = 0; l3_arr[i] != 0; i++);
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_l1d_write(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L1d (warmup).
    for (i = 0; l1d_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly write 4K to the L1d.
    for (j = 0; j < count; j++) {
        for (i = 0; i < L1d_CACHE_SIZE / 8; i++) {
            l1d_arr[i] = 'a';
        }
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_l2_write(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L2 (warmup).
    for (i = 0; l2_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly write 64K to the L2.
    for (j = 0; j < count; j++) {
        for (i = 0; i < L1d_CACHE_SIZE * 2; i++) {
            l2_arr[i] = 'a';
        }
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_l3_write(int count) {
    long long start, end;
    unsigned int tsc_aux;
    int i, j;

    // Pull the data into the L3 (warmup).
    for (i = 0; l3_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // Repeatedly write 512K to the L3.
    for (j = 0; j < count; j++) {
        for (i = 0; i < L2_CACHE_SIZE * 2; i++) {
            l3_arr[i] = 'a';
        }
    }

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

cache_bandwidth_stats cache_bandwidth(int iterations) {
    int i;
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    // Prep the arrays.
    setup_cachebw();
    cache_bandwidth_stats s;
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l1d_read(CACHEBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_l1d_read(CACHEBW_LOOPS_CALIBRATION);
    }
    s.l1d.read = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l1d_write(CACHEBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_l1d_write(CACHEBW_LOOPS_CALIBRATION);
    }
    s.l1d.write = calibrated_int_stats(times, times_calib, iterations);

    for (i = 0; i < iterations; i++) {
        times[i] = measure_l2_read(CACHEBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_l2_read(CACHEBW_LOOPS_CALIBRATION);
    }
    s.l2.read = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l2_write(CACHEBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_l2_write(CACHEBW_LOOPS_CALIBRATION);
    }
    s.l2.write = calibrated_int_stats(times, times_calib, iterations);

    for (i = 0; i < iterations; i++) {
        times[i] = measure_l3_read(CACHEBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_l3_read(CACHEBW_LOOPS_CALIBRATION);
    }
    s.l3.read = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l3_write(CACHEBW_LOOPS_MEASUREMENT);
        times_calib[i] = measure_l3_write(CACHEBW_LOOPS_CALIBRATION);
    }
    s.l3.write = calibrated_int_stats(times, times_calib, iterations);

    return s;
}

void storeResults_08(cache_bandwidth_stats stats) {
    storeResults(stats.l1d.read.calibration, "08CacheBW_L1dRead_Calibration");
    storeResults(stats.l1d.read.measurement, "08CacheBW_L1dRead_Measurement");
    storeResults(stats.l1d.write.calibration, "08CacheBW_L1dWrite_Calibration");
    storeResults(stats.l1d.write.measurement, "08CacheBW_L1dWrite_Measurement");

    storeResults(stats.l2.read.calibration, "08CacheBW_L2Read_Calibration");
    storeResults(stats.l2.read.measurement, "08CacheBW_L2Read_Measurement");
    storeResults(stats.l2.write.calibration, "08CacheBW_L2Write_Calibration");
    storeResults(stats.l2.write.measurement, "08CacheBW_L2Write_Measurement");

    storeResults(stats.l3.read.calibration, "08CacheBW_L3Read_Calibration");
    storeResults(stats.l3.read.measurement, "08CacheBW_L3Read_Measurement");
    storeResults(stats.l3.write.calibration, "08CacheBW_L3Write_Calibration");
    storeResults(stats.l3.write.measurement, "08CacheBW_L3Write_Measurement");
}

void displayResults_08(cache_bandwidth_stats stats) {
    int runs = stats.l1d.read.measurement.n_samples;
    double loopdiff = CACHEBW_LOOPS_MEASUREMENT - CACHEBW_LOOPS_CALIBRATION;

    printf("\nL1d Read  Time for (%d - %d) * 4KiB: (%d runs)\n",
           CACHEBW_LOOPS_MEASUREMENT, CACHEBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.l1d.read);
    printf("L1d Write Time for (%d - %d) * 4KiB: (%d runs)\n",
           CACHEBW_LOOPS_MEASUREMENT, CACHEBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.l1d.write);
    printf("Bytes per cycle: %f R, %f W\n",
           4.0 * 1024.0 * loopdiff /
               (stats.l1d.read.measurement.median -
                stats.l1d.read.calibration.median),
           4.0 * 1024.0 * loopdiff /
               (stats.l1d.write.measurement.median -
                stats.l1d.write.calibration.median));

    printf("\nL2 Read  Time for (%d - %d) * 64KiB: (%d runs)\n",
           CACHEBW_LOOPS_MEASUREMENT, CACHEBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.l2.read);
    printf("L2 Write Time for (%d - %d) * 64KiB: (%d runs)\n",
           CACHEBW_LOOPS_MEASUREMENT, CACHEBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.l2.write);
    printf("Bytes per cycle: %f R, %f W\n",
           64.0 * 1024.0 * loopdiff /
               (stats.l2.read.measurement.median -
                stats.l2.read.calibration.median),
           64.0 * 1024.0 * loopdiff /
               (stats.l2.write.measurement.median -
                stats.l2.write.calibration.median));

    printf("\nL3 Read  Time for (%d - %d) * 512KiB: (%d runs)\n",
           CACHEBW_LOOPS_MEASUREMENT, CACHEBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.l3.read);
    printf("L3 Write Time for (%d - %d) * 512KiB: (%d runs)\n",
           CACHEBW_LOOPS_MEASUREMENT, CACHEBW_LOOPS_CALIBRATION, runs);
    printCalibrated(stats.l3.write);
    printf("Bytes per cycle: %f R, %f W\n",
           512.0 * 1024.0 * loopdiff /
               (stats.l3.read.measurement.median -
                stats.l3.read.calibration.median),
           512.0 * 1024.0 * loopdiff /
               (stats.l3.write.measurement.median -
                stats.l3.write.calibration.median));
}
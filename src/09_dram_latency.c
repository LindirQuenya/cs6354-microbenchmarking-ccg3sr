#include <x86intrin.h>
#include <emmintrin.h>
#include <string.h>

#include "09_dram_latency.h"
#include "stats.h"
#include "opt.h"
#include "cache.h"

// If these aren't marked volatile the compiler optimizes out the cache-filling
// operations.
volatile __attribute__((aligned(64))) unsigned char l3_arr[L3_CACHE_SIZE];

volatile
    __attribute__((aligned(64))) unsigned char dram_target_cacheline[LINE_SIZE];

void setup_dramlat(void) {
    _mm_mfence();
    memset((void *)l3_arr, 'a', L3_CACHE_SIZE);
    l3_arr[L3_CACHE_SIZE - 1] = 0;
    __cpuid();
}

NOINLINE long measure_dram_calib(void) {
    long long start, end;
    unsigned int tsc_aux;
    // Load the target cache line into L1d,L2,L3.
    dram_target_cacheline[0] = 'b';

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    // But don't actually do the memory operation.

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

NOINLINE long measure_dram(void) {
    long long start, end;
    unsigned int tsc_aux;
    register unsigned char target;
    // Load the target cache line into L1d,L2,L3.
    dram_target_cacheline[0] = 'b';
    // But now fill up L3 (and thus also L2+L1d) with a different array.
    for (int i = 0; l3_arr[i] != 0; i++);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    target = dram_target_cacheline[0];

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    // To avoid dead value elimination.
    dram_target_cacheline[0] = target;
    return end - start;
}

calibrated_stats dram_latency(int iterations) {
    int i;
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    // Prep the array.
    setup_dramlat();
    for (i = 0; i < iterations; i++) {
        times_calib[i] = measure_dram_calib();
    }
    for (i = 0; i < iterations; i++) {
        times[i] = measure_dram();
    }
    return calibrated_int_stats(times, times_calib, iterations);
}

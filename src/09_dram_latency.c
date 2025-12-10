#include <x86intrin.h>
#include <emmintrin.h>

#include "09_dram_latency.h"
#include "stats.h"
#include "opt.h"
#include "cache.h"

NOINLINE long measure_dram_calib(void) {
    long long start, end;
    unsigned int tsc_aux;

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

    // Explicitly evict the target from the cache.
    _mm_clflush((void *)target_cacheline);

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    target = target_cacheline[0];

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    // To avoid dead value elimination.
    target_cacheline[0] = target;
    return end - start;
}

calibrated_stats dram_latency(int iterations) {
    int i;
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);

    for (i = 0; i < iterations; i++) {
        times_calib[i] = measure_dram_calib();
    }
    for (i = 0; i < iterations; i++) {
        times[i] = measure_dram();
    }
    return calibrated_int_stats(times, times_calib, iterations);
}

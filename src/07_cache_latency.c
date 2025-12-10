#include <x86intrin.h>
#include <emmintrin.h>
#include <string.h>

#include "07_cache_latency.h"
#include "stats.h"
#include "opt.h"

#define LINE_SIZE      (64)
#define L1d_CACHE_SIZE (32768)
#define L2_CACHE_SIZE  (256 * 1024)
#define L3_CACHE_SIZE  (8192 * 1024)

__attribute__((aligned(64))) unsigned char l1d_arr[L1d_CACHE_SIZE];
__attribute__((aligned(64))) unsigned char l2_arr[L2_CACHE_SIZE];

volatile __attribute__((aligned(64))) unsigned char target_cacheline[LINE_SIZE];

void setup(void) {
    _mm_mfence();
    memset((void *)l1d_arr, 'a', L1d_CACHE_SIZE);
    l1d_arr[L1d_CACHE_SIZE - 1] = 0;
    memset((void *)l2_arr, 'a', L2_CACHE_SIZE);
    l2_arr[L2_CACHE_SIZE - 1] = 0;
    __cpuid();
}

NOINLINE long measure_l1i(void) {
    long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    // cpuid prevents *fetching* the next instruction
    // until the rdtscp completes.
    __cpuid();
    // This should be eliminated at the frontend.
    __nop();
    __cpuid();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE long measure_l1i_calib(void) {
    long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    __cpuid();
    __cpuid();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE long measure_l1d(void) {
    long long start, end;
    unsigned int tsc_aux;
    register unsigned char target;
    // Load the target cache line into L1d,L2,L3.
    target_cacheline[0] = 'b';
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

NOINLINE long measure_datacache_calib(void) {
    long long start, end;
    unsigned int tsc_aux;
    // Load the target cache line into L1d,L2,L3.
    target_cacheline[0] = 'b';
    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    // But don't actually do the memory operation.
    // a = 3;
    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

NOINLINE long measure_l2(void) {
    long long start, end;
    unsigned int tsc_aux;
    register unsigned char target;
    // Load the target cache line into L1d,L2,L3.
    target_cacheline[0] = 'b';
    // But now fill up L1d with a different array.
    for (int i = 0; l1d_arr[i] != 0; i++);
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

NOINLINE long measure_l3(void) {
    long long start, end;
    unsigned int tsc_aux;
    register unsigned char target;
    // Load the target cache line into L1d,L2,L3.
    target_cacheline[0] = 'b';
    // But now fill up L2 (and thus also L1d) with a different array.
    for (int i = 0; l2_arr[i] != 0; i++);
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

cache_latency_stats cache_latency(int iterations) {
    int i;
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    // Prep the arrays.
    setup();
    cache_latency_stats s;
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l1i();
        times_calib[i] = measure_l1i_calib();
    }
    s.l1i = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l1d();
        times_calib[i] = measure_datacache_calib();
    }
    s.l1d = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l2();
    }
    s.l2 = calibrated_int_stats(times, times_calib, iterations);
    for (i = 0; i < iterations; i++) {
        times[i] = measure_l3();
    }
    s.l3 = calibrated_int_stats(times, times_calib, iterations);
    return s;
}

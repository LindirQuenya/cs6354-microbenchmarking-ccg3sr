#include <pthread.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <string.h>
#include <stdio.h>

#include "11_smt_contention.h"
#include "stats.h"
#include "opt.h"
#include "cache.h"
#include "harness.h"

#define ARRLEN (L2_CACHE_SIZE / sizeof(int))
int arr1[ARRLEN];
int arr2[ARRLEN];

void smt_setup(void) {
    memset(arr1, 12, ARRLEN);
    memset(arr2, 12, ARRLEN);
    __cpuid();
}

void mean_arr1(int *count) {
    volatile double mean;
    for (int i = 0; i < *count; i++) {
        mean = int_stats_mean(arr1, ARRLEN);
    }
}

void mean_arr2(int *count) {
    volatile double mean;
    for (int i = 0; i < *count; i++) {
        mean = int_stats_mean(arr2, ARRLEN);
    }
}

long long measure_smt_contention(void *(r1)(void *), void *(r2)(void *)) {
    long long start, end;
    unsigned int tsc_aux;
    int count = 100000;
    pthread_t t1, t2;

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    pthread_create(&t1, NULL, (void *)r1, &count);
    pthread_create(&t2, NULL, (void *)r2, &count);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

smt_contention_stats smt_contention(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    smt_contention_stats s;
    for (int i = 0; i < iterations; i++) {
        times[i] =
            measure_smt_contention((void *)&mean_arr1, (void *)&mean_arr1);
    }
    s.complementary = int_stats(times, iterations);
    for (int i = 0; i < iterations; i++) {
        times[i] =
            measure_smt_contention((void *)&mean_arr1, (void *)&mean_arr2);
    }
    s.competing = int_stats(times, iterations);
}

void displayResults_11(smt_contention_stats stats) {
    printf("\nSMT Complementary Runtime:\n");
    printRuntimeStats(stats.complementary);
    printf("\nSMT Competing Runtime:\n");
    printRuntimeStats(stats.competing);
}
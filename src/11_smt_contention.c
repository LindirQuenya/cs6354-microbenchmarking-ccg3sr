#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <threads.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <string.h>
#include <stdio.h>

#include "11_smt_contention.h"
#include "stats.h"
#include "opt.h"
#include "cache.h"
#include "harness.h"

#define ARRLEN (L1d_CACHE_SIZE / sizeof(int))
int *arr1;
int *arr2;
pthread_barrier_t barrier;

void smt_setup(void) {
    arr1 = aligned_alloc(L1d_CACHE_SIZE, ARRLEN * 4);
    arr2 = aligned_alloc(L1d_CACHE_SIZE, ARRLEN * 4);
    memset(arr1, 12, ARRLEN * 4);
    memset(arr2, 12, ARRLEN * 4);
    // pthread_barrier_init(&barrier, const int *restrict attr, unsigned int
    // count)
    pthread_barrier_init(&barrier, NULL, 2);
    __cpuid();
}

inline int lcg(int x) {
    // Hull-Dobell conditions:
    // - c must be coprime with m
    // - a-1 must be divisible by all the prime factors of m
    // - a-1 must be divisible by 4 if m is
    const int a = 41;
    const int c = 17;
    const int m = L1d_NUM_SETS;
    return (a * x + c) % m;
}

inline int perm8(int x) { return (5 * x + 7) % 8; }

typedef struct {
    int count;
    cpu_set_t affinity;
    int first;
    int *arr;
} smt_args;

void striding_mean(smt_args *arg) {
    pthread_t thisThread = pthread_self();
    pthread_setaffinity_np(thisThread, sizeof(arg->affinity), &(arg->affinity));
    pthread_barrier_wait(&barrier);
    int *arr = arg->arr;
    volatile int mean;
    for (int i = 0; i < arg->count; i++) {
        int jrand = lcg(0);
        long long sum = 0;
        for (int j = 0; j < L1d_NUM_SETS; j++) {
            if (!arg->first) {
                pthread_barrier_wait(&barrier);
            }
            // index = j << 6
            // This selects a cache set for us. Now we
            // fill up all the ways in that set.
            for (int a = 0; a < L1d_ASSOCIATIVITY; a++) {
                // tag = a << 12
                // There are multiple elements in each line, sum all of those.
                for (int k = 0; k < LINE_SIZE / sizeof(int); k++) {
                    // offset = k << 2
                    // Everything gets automatic << 2 from int indexing.
                    sum += arr[(((perm8(a) << 6) + j) << 4) + k];
                }
            }
            if (arg->first) {
                pthread_barrier_wait(&barrier);
            }
            jrand = lcg(jrand);
        }
        mean = sum / ARRLEN;
    }
}

long long measure_smt_contention(int count, void *(r1)(void *),
                                 void *(r2)(void *), int *in1, int *in2) {
    long long start, end;
    unsigned int tsc_aux;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    smt_args s1 = {
        .affinity = cpuset,
        .count = count,
        .first = 0,
        .arr = in1,
    };
    // CPU_ZERO(&cpuset);
    // CPU_SET(7, &cpuset);
    smt_args s2 = {
        .affinity = cpuset,
        .count = count,
        .first = 1,
        .arr = in2,
    };
    pthread_t t1, t2;

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    pthread_create(&t1, NULL, (void *)r1, &s1);
    pthread_create(&t2, NULL, (void *)r2, &s2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    _mm_mfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();

    return end - start;
}

smt_contention_stats smt_contention(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    smt_setup();
    smt_contention_stats s;
    for (int i = 0; i < iterations; i++) {
        times_calib[i] = measure_smt_contention(
                             SMT_LOOPS_CALIBRATION, (void *)&striding_mean,
                             (void *)&striding_mean, arr1, arr1) /
                         SMT_LOOPS_CALIBRATION;
        times[i] = measure_smt_contention(SMT_LOOPS_MEASUREMENT,
                                          (void *)&striding_mean,
                                          (void *)&striding_mean, arr1, arr1) /
                   SMT_LOOPS_CALIBRATION;
    }
    s.complementary = calibrated_int_stats(times, times_calib, iterations);
    for (int i = 0; i < iterations; i++) {
        times_calib[i] = measure_smt_contention(
                             SMT_LOOPS_CALIBRATION, (void *)&striding_mean,
                             (void *)&striding_mean, arr1, arr2) /
                         SMT_LOOPS_CALIBRATION;
        times[i] = measure_smt_contention(SMT_LOOPS_MEASUREMENT,
                                          (void *)&striding_mean,
                                          (void *)&striding_mean, arr1, arr2) /
                   SMT_LOOPS_CALIBRATION;
    }
    s.competing = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);
    free(arr1);
    free(arr2);

    return s;
}

void displayResults_11(smt_contention_stats stats) {
    printf("\nSMT Complementary Runtime (%d-%d):\n", SMT_LOOPS_MEASUREMENT,
           SMT_LOOPS_CALIBRATION);
    printCalibrated(stats.complementary);
    printf("\nSMT Competing Runtime (%d-%d):\n", SMT_LOOPS_MEASUREMENT,
           SMT_LOOPS_CALIBRATION);
    printCalibrated(stats.competing);
}
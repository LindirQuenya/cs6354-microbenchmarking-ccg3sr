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
    int *arr;
} smt_args;

void *striding_mean(void *varg) {
    smt_args *arg = varg;
    pthread_t thisThread = pthread_self();
    pthread_setaffinity_np(thisThread, sizeof(arg->affinity), &(arg->affinity));
    pthread_barrier_wait(&barrier);
    int *arr = arg->arr;
    volatile int mean;
    for (int i = 0; i < arg->count; i++) {
        int jrand = lcg(0);
        long long sum = 0;
        // We're going to do something slightly diabolical.
        // We're going to try to fill up the L1 cache, set by set,
        // as quickly as we can. If we're using the same array
        // as the other SMT thread, this will be fine. If we're
        // using a different array, we'll end up mutually evicting.
        // This won't be a *huge* performance issue (12 cycles vs. 4)
        // but it should be measurable.
        for (int j = 0; j < L1d_NUM_SETS; j++) {
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
            jrand = lcg(jrand);
        }
        mean = sum / ARRLEN;
    }
    return NULL;
}

long long measure_smt_contention(int count, int *in1, int *in2) {
    long long start, end;
    unsigned int tsc_aux;
    // Bind the first thread to CPU 3.
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    smt_args arg1 = {
        .affinity = cpuset,
        .count = count,
        .arr = in1,
    };
    // The second thread should go to CPU 7, its SMT pair.
    CPU_ZERO(&cpuset);
    CPU_SET(7, &cpuset);
    smt_args arg2 = {
        .affinity = cpuset,
        .count = count,
        .arr = in2,
    };
    pthread_t t1, t2;
    void *(*fnptr)(void *) = &striding_mean;

    _mm_mfence();
    start = __rdtscp(&tsc_aux);
    _mm_lfence();

    pthread_create(&t1, NULL, fnptr, &arg1);
    pthread_create(&t2, NULL, fnptr, &arg2);
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
        times_calib[i] =
            measure_smt_contention(SMT_LOOPS_CALIBRATION, arr1, arr1) /
            SMT_LOOPS_CALIBRATION;
        times[i] = measure_smt_contention(SMT_LOOPS_MEASUREMENT, arr1, arr1) /
                   SMT_LOOPS_CALIBRATION;
    }
    s.complementary = calibrated_int_stats(times, times_calib, iterations);
    for (int i = 0; i < iterations; i++) {
        times_calib[i] =
            measure_smt_contention(SMT_LOOPS_CALIBRATION, arr1, arr2) /
            SMT_LOOPS_CALIBRATION;
        times[i] = measure_smt_contention(SMT_LOOPS_MEASUREMENT, arr1, arr2) /
                   SMT_LOOPS_CALIBRATION;
    }
    s.competing = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);
    free(arr1);
    free(arr2);

    return s;
}

void storeResults_11(smt_contention_stats stats) {
    storeResults(stats.complementary.calibration,
                 "11SMT_Complementary_Calibration");
    storeResults(stats.complementary.measurement,
                 "11SMT_Complementary_Measurement");

    storeResults(stats.competing.calibration, "11SMT_Competing_Calibration");
    storeResults(stats.competing.measurement, "11SMT_Competing_Measurement");
}

void displayResults_11(smt_contention_stats stats) {
    printf("\nSMT Complementary Runtime (%d-%d):\n", SMT_LOOPS_MEASUREMENT,
           SMT_LOOPS_CALIBRATION);
    printCalibrated(stats.complementary);
    printf("\nSMT Competing Runtime (%d-%d):\n", SMT_LOOPS_MEASUREMENT,
           SMT_LOOPS_CALIBRATION);
    printCalibrated(stats.competing);
}
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

#define ARRLEN (3 * L2_CACHE_SIZE / sizeof(int) / 4)
int *arr1;
int *arr2;
pthread_barrier_t barrier;

void smt_setup(void) {
    arr1 = malloc(ARRLEN * 4);
    arr2 = malloc(ARRLEN * 4);
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
    const int a = 2869;
    const int c = 2933;
    const int m = ARRLEN;
    return (a * x + c) % m;
}

typedef struct {
    int count;
    cpu_set_t affinity;
} smt_args;

void mean_arr1(smt_args *arg) {
    pthread_t thisThread = pthread_self();
    pthread_setaffinity_np(thisThread, sizeof(arg->affinity), &(arg->affinity));
    pthread_barrier_wait(&barrier);
    volatile int mean;
    for (int i = 0; i < arg->count; i++) {
        int jrand = lcg(0);
        long long sum = 0;
        for (int j = 0; j < ARRLEN; j++) {
            sum += arr1[jrand];
            jrand = lcg(jrand);
        }
        mean = sum / ARRLEN;
        pthread_barrier_wait(&barrier);
    }
}

void mean_arr2(smt_args *arg) {
    pthread_t thisThread = pthread_self();
    pthread_setaffinity_np(thisThread, sizeof(arg->affinity), &(arg->affinity));
    pthread_barrier_wait(&barrier);
    volatile int mean;
    for (int i = 0; i < arg->count; i++) {
        int jrand = lcg(0);
        long long sum = 0;
        for (int j = 0; j < ARRLEN; j++) {
            sum += arr2[jrand];
            jrand = lcg(jrand);
        }
        mean = sum / ARRLEN;
        pthread_barrier_wait(&barrier);
    }
}

long long measure_smt_contention(int count, void *(r1)(void *),
                                 void *(r2)(void *)) {
    long long start, end;
    unsigned int tsc_aux;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    smt_args s1 = {
        .affinity = cpuset,
        .count = count,
    };
    CPU_ZERO(&cpuset);
    CPU_SET(7, &cpuset);
    smt_args s2 = {
        .affinity = cpuset,
        .count = count,
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
        times_calib[i] = measure_smt_contention(100, (void *)&mean_arr1,
                                                (void *)&mean_arr1) /
                         100;
        times[i] = measure_smt_contention(200, (void *)&mean_arr1,
                                          (void *)&mean_arr1) /
                   100;
    }
    s.complementary = calibrated_int_stats(times, times_calib, iterations);
    for (int i = 0; i < iterations; i++) {
        times_calib[i] = measure_smt_contention(100, (void *)&mean_arr1,
                                                (void *)&mean_arr2) /
                         100;
        times[i] = measure_smt_contention(200, (void *)&mean_arr1,
                                          (void *)&mean_arr2) /
                   100;
    }
    s.competing = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);
    free(arr1);
    free(arr2);

    return s;
}

void displayResults_11(smt_contention_stats stats) {
    printf("\nSMT Complementary Runtime (200-100):\n");
    printCalibrated(stats.complementary);
    printf("\nSMT Competing Runtime (200-100):\n");
    printCalibrated(stats.competing);
}
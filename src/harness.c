#define _GNU_SOURCE
#include <sched.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <math.h>
#include <pthread.h>

#include "00_function_call.h"

struct runtime_stats {
    double mean;
    double sd;
};

void printRuntimeStats(struct runtime_stats stats) {
    printf("Mean: %f, SD: %f\n", stats.mean, stats.sd);
}

double int_stats_mean(const int data[], const size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum / n;
}

double int_stats_sd(const int data[], const size_t n) {
    double mean = int_stats_mean(data, n);
    double sum_sq_diff = 0.0;
    for (size_t i = 0; i < n; i++) {
        double diff = data[i] - mean;
        sum_sq_diff += diff * diff;
    }
    return sqrt(sum_sq_diff / n);
}

int measureNothing() {
    unsigned long long start, end;
    _mm_lfence();
    start = __rdtsc();
    _mm_lfence();
    _mm_lfence();
    end = __rdtsc();
    _mm_lfence();
    return end - start;
}

int measureFunction(void (*func)()) {
    unsigned long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    func();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

struct runtime_stats timingOverhead(int runs) {
    int *runtimeArray = malloc(sizeof(int) * runs);

    // Warm-up phase to stabilize CPU state.
    for (int i = 0; i < runs/10; i++) {
        measureNothing();
    }
    for (int i = 0; i < runs; i++) {
        runtimeArray[i] = measureNothing();
;
    }
    double mean = int_stats_mean(runtimeArray, runs);
    double sd = int_stats_sd(runtimeArray, runs);
    free(runtimeArray);
    struct runtime_stats stats = {mean, sd};
    return stats;
}

struct runtime_stats timingHarness(void (*func)(), int runs) {
    int *runtimeArray = malloc(sizeof(int) * runs);

    // Warm-up phase to stabilize CPU state.
    for (int i = 0; i < runs/10; i++) {
        measureFunction(func);
    }
    for (int i = 0; i < runs; i++) {
        runtimeArray[i] = measureFunction(func);
    }
    double mean = int_stats_mean(runtimeArray, runs);
    double sd = int_stats_sd(runtimeArray, runs);
    free(runtimeArray);
    struct runtime_stats stats = {mean, sd};
    return stats;
}

void storeResults(struct runtime_stats stats, const char* benchmarkName) {
    FILE* file = fopen("results.csv", "a");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return;
    }
    fprintf(file, "%s,%f,%f\n", benchmarkName, stats.mean, stats.sd);
    fclose(file);
}

int main(int argc, char** argv){
    /* Setup code -- e.g., CPU pinning, disabling features, etc. */
    // Inspired by the example in man 3 pthread_setaffinity_np
    pthread_t thisThread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    int s = pthread_setaffinity_np(thisThread, sizeof(cpuset), &cpuset);
    if (s != 0) {
        err(EXIT_FAILURE, "code: %d from pthread_setaffinity_np", s);
    }
    s = pthread_getaffinity_np(thisThread, sizeof(cpu_set_t), &cpuset);
    printf("Set returned by pthread_getaffinity_np() contained:\n");
    for (size_t j = 0; j < CPU_SETSIZE; j++)
        if (CPU_ISSET(j, &cpuset))
            printf("    CPU %zu\n", j);

    int runs = argc > 1 ? atoi(argv[1]) : 1000;

    /* Running microbenchmarks and generating results. */
    struct runtime_stats stats;
    stats = timingOverhead(runs);
    printRuntimeStats(stats);
    storeResults(stats, "timingOverhead");

    stats = timingHarness(function_call_v_v, runs);
    printRuntimeStats(stats);
    storeResults(stats, "timingHarness");
    return 0;
}

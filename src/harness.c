#define _GNU_SOURCE
#include <sched.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <pthread.h>

#include "stats.h"
#include "00_function_call.h"
#include "01_context_switch.h"

int measureNothing() {
    unsigned long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    end = __rdtscp(&tsc_aux);
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
    struct runtime_stats stats = int_stats(runtimeArray, runs);
    free(runtimeArray);
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
    struct runtime_stats stats = int_stats(runtimeArray, runs);
    free(runtimeArray);
    return stats;
}

void storeResults(struct runtime_stats stats, const char* benchmarkName) {
    FILE* file = fopen("results.csv", "a");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return;
    }
    fprintf(file, "%s,%f,%f,%f,%zu\n", benchmarkName, stats.mean, stats.median, stats.sd, stats.n_samples);
    fclose(file);
}

int main(int argc, char** argv){
    /* Setup code -- e.g., CPU pinning, disabling features, etc. */
    // Inspired by the example in man 3 pthread_setaffinity_np
    pthread_t thisThread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
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
    storeResults(stats, "00FunctionCall");

    stats = timingHarness(function_call_v_v, runs);
    printRuntimeStats(stats);
    storeResults(stats, "00Calibration");

    calibrated_stats syscall = contextswitch_syscall(runs);
    storeResults(syscall.calibration, "01SyscallCalibration");
    storeResults(syscall.measurement, "01SyscallMeasurement");
    printf("\nSyscall: (%d runs)\n", runs);
    printCalibrated(syscall);

    calibrated_stats context_switch = contextswitch_thread(runs/1000);
    storeResults(context_switch.calibration, "01ThreadCalibration");
    storeResults(context_switch.measurement, "01ThreadMeasurement");
    printf("\nThread switch: (%d runs)\n", runs/1000);
    printCalibrated(context_switch);

    return 0;
}

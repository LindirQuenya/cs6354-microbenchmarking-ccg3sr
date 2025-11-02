#define _GNU_SOURCE
#include <sched.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <pthread.h>
#include <string.h>

#include "stats.h"
#include "00_function_call.h"
#include "01_context_switch.h"
#include "02_fetch_throughput.h"


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
    functioncall_results f = functioncall_measure_all(runs);
    storeResults(f.calibration, "00Calibration");
    storeResults(f.i_v, "00Fn_i_v");
    calibrated_stats c = {f.calibration, f.i_v};
    printf("00Fn_i_v:\n");
    printCalibrated(c);
    char *fncall_fmt = "00Fn_v_i%d";
    // Add four just in case. I don't think we'll need any: the two from the %d should be plenty.
    int fncall_label_len = strlen(fncall_fmt) + 4;
    char *fncall_label = malloc(sizeof(char) * fncall_label_len);
    for (int i = 0; i <= FNCALL_INT_ARGS_MAX; i++) {
        snprintf(fncall_label, fncall_label_len, fncall_fmt, i);
        storeResults(f.v_iN[i], fncall_label);
        printf("%s: ", fncall_label);
        printRuntimeStats(f.v_iN[i]);
    }

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

    calibrated_stats fetchthroughput = fetch_throughput(runs);
    storeResults(fetchthroughput.calibration, "02FetchCalibration_10k");
    storeResults(fetchthroughput.measurement, "02FetchMeasurement_20k");
    printf("\nFetch Throughput: (%d runs)\n", runs);
    printCalibrated(fetchthroughput);

    return 0;
}

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
#include "04_load_store_throughput.h"
#include "05_branch_mispredict.h"


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

    calibrated_stats fetchthroughput8 = fetch_throughput8(runs/10);
    storeResults(fetchthroughput8.calibration, "02FetchCalibration8_10k");
    storeResults(fetchthroughput8.measurement, "02FetchMeasurement8_20k");
    printf("\nFetch Throughput (8 nops): (%d runs)\n", runs/10);
    printCalibrated(fetchthroughput8);
    printf("Instr fetched per cycle (8 nops): %f\n", (FETCH_LOOPS_MEASUREMENT - FETCH_LOOPS_CALIBRATION) * 10.0 / (fetchthroughput8.measurement.median - fetchthroughput8.calibration.median));
    calibrated_stats fetchthroughput16 = fetch_throughput16(runs/10);
    storeResults(fetchthroughput16.calibration, "02FetchCalibration16_10k");
    storeResults(fetchthroughput16.measurement, "02FetchMeasurement16_20k");
    printf("\nFetch Throughput (16 nops): (%d runs)\n", runs/10);
    printCalibrated(fetchthroughput16);
    printf("Instr fetched per cycle (16 nops): %f\n", (FETCH_LOOPS_MEASUREMENT - FETCH_LOOPS_CALIBRATION) * 18.0 / (fetchthroughput16.measurement.median - fetchthroughput16.calibration.median));

    load_store_stats loadstore = loadstore_throughput(runs / 10);
    storeResults(loadstore.load8.calibration, "04LoadCalibration8_10k");
    storeResults(loadstore.load8.measurement, "04LoadMeasurement8_20k");
    printf("\nL/S Throughput (8 loads): (%d runs)\n", runs/10);
    printCalibrated(loadstore.load8);
    printf("L/S per cycle (8 loads): %f\n", (LOADSTORE_LOOPS_MEASUREMENT - LOADSTORE_LOOPS_CALIBRATION) * 10.0 / (loadstore.load8.measurement.median - loadstore.load8.calibration.median));
    storeResults(loadstore.load16.calibration, "04LoadCalibration16_10k");
    storeResults(loadstore.load16.measurement, "04LoadMeasurement16_20k");
    printf("\nL/S Throughput (16 loads): (%d runs)\n", runs/10);
    printCalibrated(loadstore.load16);
    printf("L/S per cycle (16 loads): %f\n", (LOADSTORE_LOOPS_MEASUREMENT - LOADSTORE_LOOPS_CALIBRATION) * 18.0 / (loadstore.load16.measurement.median - loadstore.load16.calibration.median));
    storeResults(loadstore.store8.calibration, "04StoreCalibration8_10k");
    storeResults(loadstore.store8.measurement, "04StoreMeasurement8_20k");
    printf("\nL/S Throughput (8 stores): (%d runs)\n", runs/10);
    printCalibrated(loadstore.store8);
    printf("L/S per cycle (8 stores): %f\n", (LOADSTORE_LOOPS_MEASUREMENT - LOADSTORE_LOOPS_CALIBRATION) * 10.0 / (loadstore.store8.measurement.median - loadstore.store8.calibration.median));
    storeResults(loadstore.store16.calibration, "04StoreCalibration16_10k");
    storeResults(loadstore.store16.measurement, "04StoreMeasurement16_20k");
    printf("\nL/S Throughput (16 stores): (%d runs)\n", runs/10);
    printCalibrated(loadstore.store16);
    printf("L/S per cycle (16 stores): %f\n", (LOADSTORE_LOOPS_MEASUREMENT - LOADSTORE_LOOPS_CALIBRATION) * 18.0 / (loadstore.store16.measurement.median - loadstore.store16.calibration.median));

    calibrated_stats branch_mispredict = mispredict(runs/10);
    storeResults(branch_mispredict.calibration, "05Predicted");
    storeResults(branch_mispredict.measurement, "05Mispredicted");
    printf("\nBranch Penalty: (%d runs)\n", runs/10);
    printCalibrated(branch_mispredict);

    return 0;
}

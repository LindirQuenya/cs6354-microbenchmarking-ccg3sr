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
#include "03_retire_throughput.h"
#include "04_load_store_throughput.h"
#include "05_branch_mispredict.h"
#include "06_exec_unit_throughput.h"
#include "07_cache_latency.h"
#include "08_cache_bandwidth.h"
#include "09_dram_latency.h"
#include "10_dram_bandwidth.h"
#include "11_smt_contention.h"

void storeResults(struct runtime_stats stats, const char *benchmarkName) {
    FILE *file = fopen("results.csv", "a");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return;
    }
    fprintf(file, "%s,%f,%f,%f,%zu\n", benchmarkName, stats.mean, stats.median,
            stats.sd, stats.n_samples);
    fclose(file);
}

int main(int argc, char **argv) {
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
    for (size_t j = 0; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, &cpuset)) {
            printf("    CPU %zu\n", j);
        }
    }

    int runs = argc > 1 ? atoi(argv[1]) : 1000;
#ifndef RUN_LAST_ONLY
    /* Running microbenchmarks and generating results. */
    functioncall_stats fncall = functioncall_measure_all(runs);
    storeResults_00(fncall);
    displayResults_00(fncall);

    contextswitch_stats contextsw = context_switch(runs);
    storeResults_01(contextsw);
    displayResults_01(contextsw);

    fetch_throughput_stats fetch = fetch_throughput(runs / 10);
    storeResults_02(fetch);
    displayResults_02(fetch);

    retire_stats retire = retire_throughput(runs / 10);
    storeResults_03(retire);
    displayResults_03(retire);

    load_store_stats loadstore = loadstore_throughput(runs / 10);
    storeResults_04(loadstore);
    displayResults_04(loadstore);

    calibrated_stats branch_mispredict = mispredict(runs / 10);
    storeResults_05(branch_mispredict);
    displayResults_05(branch_mispredict);

    exec_unit_stats execunit = execunit_throughput(runs / 10);
    storeResults_06(execunit);
    displayResults_06(execunit);

    cache_latency_stats cachelat = cache_latency(runs / 100);
    storeResults_07(cachelat);
    displayResults_07(cachelat);

    calibrated_stats dramlat = dram_latency(runs / 100);
    storeResults_09(dramlat);
    displayResults_09(dramlat);

    cache_bandwidth_stats cachebw = cache_bandwidth(runs / 1000);
    storeResults_08(cachebw);
    displayResults_08(cachebw);

    dram_bandwidth_stats drambw = dram_bandwidth(runs / 10000);
    storeResults_10(drambw);
    displayResults_10(drambw);
    return 0;
#endif
    smt_contention_stats smt = smt_contention(runs / 100000);
    // storeResults_11(smt);
    displayResults_11(smt);
}

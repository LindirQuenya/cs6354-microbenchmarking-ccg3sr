#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <math.h>

__attribute__((noinline)) void emptyFunction() {
    asm ("");
    // Intentionally empty
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

int measureFunction(void (*func)()) {
    unsigned long long start, end;
    _mm_lfence();
    start = __rdtsc();
    _mm_lfence();
    //func();
    _mm_lfence();
    end = __rdtsc();
    _mm_lfence();
    printf("Cycles: %llu\n", end - start);
    return end - start;
}

int main(int argc, char** argv){
    /* Setup code -- e.g., CPU pinning, disabling features, etc. */
    int numberOfRuns = argc > 2 ? atoi(argv[2]) : 10000;
    int runtimeArray[numberOfRuns];

    // Warm-up phase to stabilize CPU state.
    for (int i = 0; i < numberOfRuns/10; i++) {
        measureFunction(emptyFunction);
    }
    for (int i = 0; i < numberOfRuns; i++) {
        runtimeArray[i] = measureFunction(emptyFunction);
    }
    double mean = int_stats_mean(runtimeArray, numberOfRuns);
    double sd = int_stats_sd(runtimeArray, numberOfRuns);
    printf("Average cycles: %f\n", mean);
    printf("Standard deviation: %f\n", sd);
    /* Running microbenchmarks and generating results. */
}
#include <inttypes.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "stats.h"

void printRuntimeStats(struct runtime_stats stats) {
    printf("Mean: %f, Median: %f, SD: %f\n", stats.mean, stats.median, stats.sd);
}

void printCalibrated(calibrated_stats stats) {
    printf("Measurement: Mean: %f, Median: %f, SD: %f\n", stats.measurement.mean, stats.measurement.median, stats.measurement.sd);
    printf("Calibration: Mean: %f, Median: %f, SD: %f\n", stats.calibration.mean, stats.calibration.median, stats.calibration.sd);
    printf("Difference:  Mean: %f, Median: %f, \n", stats.measurement.mean - stats.calibration.mean, stats.measurement.median - stats.calibration.median);
}

double int_stats_mean(const int * data, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum / n;
}

// Helped by: https://stackoverflow.com/a/59890932
int compare_int(const void *pA, const void *pB) {
    int a = *(int *)pA, b = *(int *)pB;
    return (a > b) - (a < b);
}

int int_stats_median(const int * data, size_t n) {
    int * temp = malloc(sizeof(int) * n);
    memcpy(temp, data, sizeof(int) * n);
    qsort(temp, n, sizeof(int), &compare_int);
    int median = temp[n/2];
    free(temp);
    return median;
}

double int_stats_sd(const int * data, size_t n) {
    double mean = int_stats_mean(data, n);
    double sum_sq_diff = 0.0;
    for (size_t i = 0; i < n; i++) {
        double diff = data[i] - mean;
        sum_sq_diff += diff * diff;
    }
    return sqrt(sum_sq_diff / n);
}

struct runtime_stats int_stats(const int * data, size_t n) {
    struct runtime_stats s = {
        int_stats_mean(data, n),
        int_stats_median(data, n),
        int_stats_sd(data, n)
    };
    return s;
}


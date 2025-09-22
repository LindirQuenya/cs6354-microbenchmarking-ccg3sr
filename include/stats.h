#ifndef MICROBENCH_STATS_H
#define MICROBENCH_STATS_H

#include <stddef.h>

struct runtime_stats {
    double mean;
    double median;
    double sd;
    size_t n_samples;
};

typedef struct {
    struct runtime_stats calibration;
    struct runtime_stats measurement;
} calibrated_stats;

void printRuntimeStats(struct runtime_stats stats);
void printCalibrated(calibrated_stats stats);
struct runtime_stats int_stats(const int * data, size_t n);

#endif
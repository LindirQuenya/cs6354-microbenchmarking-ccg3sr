#ifndef MICROBENCH_00_FUNCTION_CALL_H
#define MICROBENCH_00_FUNCTION_CALL_H

#include "stats.h"

#define FNCALL_INT_ARGS_MAX 10

struct runtime_stats functioncall_measure_multi(int (*func)(void), int runs);

// This will be useful elsewhere.
int function_i_v(void);

// test
typedef struct {
    struct runtime_stats calibration;
    struct runtime_stats i_v;
    // The index is equal to the number of int parameters, e.g. 0 is void.
    struct runtime_stats v_iN[FNCALL_INT_ARGS_MAX + 1];
} functioncall_results;

functioncall_results functioncall_measure_all(int runs);

#endif
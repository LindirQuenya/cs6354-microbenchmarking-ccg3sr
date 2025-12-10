#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <stdio.h>

#include "06_exec_unit_throughput.h"
#include "stats.h"
#include "util.h"
#include "opt.h"
#include "harness.h"

// 26 addition (lea, add, and inc) instructions per loop.
NOINLINE NOUNROLL long loop_iadd(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg1 = temp;
    register int reg2 = temp;
    register int reg3 = temp;
    register int reg4 = temp;
    register int reg5 = temp;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        // This is sufficiently complicated that the compiler
        // won't turn it into a multiplication.
        REP4(reg1 += reg2 + reg3; reg2 = reg3 + reg4; reg3 = reg3 + reg5;
             reg4 = reg4 + reg5; reg5 = reg5 + reg1;)
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    // This doesn't do anything but it does mean that the compiler can't
    // dead-value-eliminate reg1/reg2.
    temp = reg1;
    return end - start;
}

// 11 instructions per loop, but only 8 matter.
NOINLINE NOUNROLL long loop_imul(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg1 = temp;
    register int reg2 = temp;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        REP8(reg1 *= reg2;)
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    // This doesn't do anything but it does mean that the compiler can't
    // dead-value-eliminate reg1/reg2.
    temp = reg1;
    return end - start;
}

// 21 instructions per loop, but only 8 of them are div (time-intensive)
NOINLINE NOUNROLL long loop_idiv(int count) {
    long long start, end;
    unsigned int tsc_aux;
    volatile int temp = 0;
    register int reg1 = temp;
    register int reg2 = temp + 3;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    for (int i = 0; i < count; i++) {
        REP8(reg1 /= reg2;)
    }
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    // This doesn't do anything but it does mean that the compiler can't
    // dead-value-eliminate reg1/reg2.
    temp = reg1;
    return end - start;
}

exec_unit_stats execunit_throughput(int iterations) {
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);

    exec_unit_stats s;

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_iadd(EXECUNIT_LOOPS_CALIBRATION);
        times[i] = loop_iadd(EXECUNIT_LOOPS_MEASUREMENT);
    }
    s.iadd = calibrated_int_stats(times, times_calib, iterations);

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_imul(EXECUNIT_LOOPS_CALIBRATION);
        times[i] = loop_imul(EXECUNIT_LOOPS_MEASUREMENT);
    }
    s.imul = calibrated_int_stats(times, times_calib, iterations);

    for (int i = 0; i < iterations; i++) {
        times_calib[i] = loop_idiv(EXECUNIT_LOOPS_CALIBRATION);
        times[i] = loop_idiv(EXECUNIT_LOOPS_MEASUREMENT);
    }
    s.idiv = calibrated_int_stats(times, times_calib, iterations);

    free(times);
    free(times_calib);

    return s;
}

void storeResults_06(exec_unit_stats stats) {
    storeResults(stats.iadd.calibration, "06ExecBW_Add_Calibration");
    storeResults(stats.iadd.measurement, "06ExecBW_Add_Measurement");

    storeResults(stats.imul.calibration, "06ExecBW_Mul_Calibration");
    storeResults(stats.imul.measurement, "06ExecBW_Mul_Measurement");

    storeResults(stats.idiv.calibration, "06ExecBW_Div_Calibration");
    storeResults(stats.idiv.measurement, "06ExecBW_Div_Measurement");
}

void displayResults_06(exec_unit_stats stats) {
    int runs = stats.iadd.measurement.n_samples;
    int loopdiff = EXECUNIT_LOOPS_MEASUREMENT - EXECUNIT_LOOPS_CALIBRATION;

    printf("\nInteger Add Throughput: (%d runs)\n", runs);
    printCalibrated(stats.iadd);
    printf("Adds per cycle (8 adds): %f\n",
           loopdiff * 26.0 /
               (stats.iadd.measurement.median - stats.iadd.calibration.median));

    printf("\nInteger Mul Throughput: (%d runs)\n", runs);
    printCalibrated(stats.imul);
    printf("Muls per cycle (8 muls): %f\n",
           loopdiff * 8.0 /
               (stats.imul.measurement.median - stats.imul.calibration.median));

    printf("\nInteger Div Throughput: (%d runs)\n", runs);
    printCalibrated(stats.idiv);
    printf("Divs per cycle (8 divs): %f\n",
           loopdiff * 8.0 /
               (stats.idiv.measurement.median - stats.idiv.calibration.median));
}
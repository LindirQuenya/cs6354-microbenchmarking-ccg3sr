#include <stdlib.h>
#include <x86intrin.h>
#include <emmintrin.h>

#include "06_exec_unit_throughput.h"
#include "stats.h"
#include "util.h"
#include "opt.h"

// 11 instructions per loop
NOINLINE_NOUNROLL long loop_iadd(int count) {
	long long start, end;
	unsigned int tsc_aux;
	volatile int temp = 0;
	register int reg1 = temp;
    register int reg2 = temp;
	start = __rdtscp(&tsc_aux);
	_mm_lfence();
	for (int i = 0; i < count; i++) {
		// This is sufficiently complicated that the compiler
		// won't turn it into a multiplication.
		REP4(reg1 += reg2; reg2 += reg1;)
	}
	end = __rdtscp(&tsc_aux);
	_mm_lfence();
	// This doesn't do anything but it does mean that the compiler can't
	// dead-value-eliminate reg1/reg2.
    temp = reg1;
	return end - start;
}

// 11 instructions per loop
NOINLINE_NOUNROLL long loop_imul(int count) {
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

// 21 instructions per loop
NOINLINE_NOUNROLL long loop_idiv(int count) {
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

	free(times); free(times_calib);

	return s;
}
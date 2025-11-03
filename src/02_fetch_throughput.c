#include <x86intrin.h>
#include <emmintrin.h>

#include "02_fetch_throughput.h"
#include "stats.h"

// In optimized form, each loop has 10 instructions.
 __attribute__((optimize("no-unroll-loops"))) calibrated_stats fetch_throughput(int iterations) {
	long long start, end;
	unsigned int tsc_aux;
	int *times = malloc(sizeof(int) * iterations);
	int *times_calib = malloc(sizeof(int) * iterations);
	for (int i = 0; i < iterations; i++) {
		// Calibration measurement includes startup overhead.
		start = __rdtscp(&tsc_aux);
		_mm_lfence();
		for (int j = 0; j < FETCH_LOOPS_CALIBRATION; j++) {
			asm(".rept 8 ; nop ; .endr");
		}
		end = __rdtscp(&tsc_aux);
		_mm_mfence();
		times_calib[i] = end - start;
		_mm_mfence();
		start = __rdtscp(&tsc_aux);
		_mm_lfence();
		// Normal measurement is the same but with more iterations.
		for (int j = 0; j < FETCH_LOOPS_MEASUREMENT; j++) {
			asm(".rept 8 ; nop ; .endr");
		}
		end = __rdtscp(&tsc_aux);
		_mm_mfence();
		times[i] = end - start;
		_mm_mfence();
	}
	calibrated_stats s = {
		.calibration = int_stats(times_calib, iterations),
		.measurement = int_stats(times, iterations)
	};
	return s;
}
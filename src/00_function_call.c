#include <sys/cdefs.h>
#include <emmintrin.h>
#include <x86intrin.h>

#include "00_function_call.h"
#include "stats.h"
#include "util.h"

#define FUNCTION_DEF(SPEC, RTYPE, RET, ...)                                    \
    __attribute__((optimize("no-ipa-cp"))) __attribute_noinline__ RTYPE        \
        function_##SPEC(__VA_ARGS__) {                                         \
        asm("");                                                               \
        return RET;                                                            \
    }
#define FUNCTION_DEF_VOID(SPEC, ...) FUNCTION_DEF(v_##SPEC, void, , __VA_ARGS__)

// To make clangd's preamble end so it can process the pragma commands correctly
// in my IDE.
int dummy_decl(void);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
FUNCTION_DEF(i_v, int, 0, void)
FUNCTION_DEF_VOID(v, void)
FUNCTION_DEF_VOID(i1, int i0)
FUNCTION_DEF_VOID(i2, int i0, int i1)
FUNCTION_DEF_VOID(i3, int i0, int i1, int i2)
FUNCTION_DEF_VOID(i4, int i0, int i1, int i2, int i3)
FUNCTION_DEF_VOID(i5, int i0, int i1, int i2, int i3, int i4)
FUNCTION_DEF_VOID(i6, int i0, int i1, int i2, int i3, int i4, int i5)
FUNCTION_DEF_VOID(i7, int i0, int i1, int i2, int i3, int i4, int i5, int i6)
FUNCTION_DEF_VOID(i8, int i0, int i1, int i2, int i3, int i4, int i5, int i6,
                  int i7)
FUNCTION_DEF_VOID(i9, int i0, int i1, int i2, int i3, int i4, int i5, int i6,
                  int i7, int i8)
FUNCTION_DEF_VOID(i10, int i0, int i1, int i2, int i3, int i4, int i5, int i6,
                  int i7, int i8, int i9)
#pragma GCC diagnostic pop

int functioncall_measure_nothing(void) {
    unsigned long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}

// Here, have a macro to cut down on the boilerplate a little bit.
#define FUNCTIONCALL_MEASURE(SPEC, ...)                                        \
    int functioncall_measure_##SPEC(void) {                                    \
        unsigned long long start, end;                                         \
        unsigned int tsc_aux;                                                  \
        start = __rdtscp(&tsc_aux);                                            \
        _mm_lfence();                                                          \
        function_##SPEC(__VA_ARGS__);                                          \
        end = __rdtscp(&tsc_aux);                                              \
        _mm_lfence();                                                          \
        return end - start;                                                    \
    }
/*
e.g. v_v
int functioncall_measure_v_v(void) {
    unsigned long long start, end;
    unsigned int tsc_aux;
    start = __rdtscp(&tsc_aux);
    _mm_lfence();
    function_v_v();
    end = __rdtscp(&tsc_aux);
    _mm_lfence();
    return end - start;
}
*/

FUNCTIONCALL_MEASURE(i_v)
FUNCTIONCALL_MEASURE(v_v)
FUNCTIONCALL_MEASURE(v_i1, JOIN(1, 0))
FUNCTIONCALL_MEASURE(v_i2, JOIN(2, 0))
FUNCTIONCALL_MEASURE(v_i3, JOIN(3, 0))
FUNCTIONCALL_MEASURE(v_i4, JOIN(4, 0))
FUNCTIONCALL_MEASURE(v_i5, JOIN(5, 0))
FUNCTIONCALL_MEASURE(v_i6, JOIN(6, 0))
FUNCTIONCALL_MEASURE(v_i7, JOIN(7, 0))
FUNCTIONCALL_MEASURE(v_i8, JOIN(8, 0))
FUNCTIONCALL_MEASURE(v_i9, JOIN(9, 0))
FUNCTIONCALL_MEASURE(v_i10, JOIN(10, 0))

struct runtime_stats functioncall_measure_multi(int (*func)(void), int runs) {
    int *runtimeArray = calloc(runs, sizeof(int));

    // Warm-up phase to stabilize CPU state.
    for (int i = 0; i < runs / 10; i++) {
        func();
    }
    for (int i = 0; i < runs; i++) {
        runtimeArray[i] = func();
    }
    struct runtime_stats stats = int_stats(runtimeArray, runs);
    free(runtimeArray);
    return stats;
}

static int (*int_measure_ptr[FNCALL_INT_ARGS_MAX + 1])(void) = {
    functioncall_measure_v_v,  functioncall_measure_v_i1,
    functioncall_measure_v_i2, functioncall_measure_v_i3,
    functioncall_measure_v_i4, functioncall_measure_v_i5,
    functioncall_measure_v_i6, functioncall_measure_v_i7,
    functioncall_measure_v_i8, functioncall_measure_v_i9,
    functioncall_measure_v_i10};

functioncall_results functioncall_measure_all(int runs) {
    functioncall_results f;
    f.calibration =
        functioncall_measure_multi(functioncall_measure_nothing, runs);
    f.i_v = functioncall_measure_multi(functioncall_measure_i_v, runs);
    for (int n = 0; n <= FNCALL_INT_ARGS_MAX; n++) {
        f.v_iN[n] = functioncall_measure_multi(int_measure_ptr[n], runs);
    }
    return f;
}

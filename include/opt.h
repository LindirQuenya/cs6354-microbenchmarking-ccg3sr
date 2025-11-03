#ifndef MICROBENCH_OPT_H
#define MICROBENCH_OPT_H

#define NOINLINE_NOUNROLL __attribute_noinline__ __attribute__((optimize("no-unroll-loops")))
#endif

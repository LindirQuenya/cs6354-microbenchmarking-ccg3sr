#ifndef MICROBENCH_OPT_H
#define MICROBENCH_OPT_H

#define NOINLINE __attribute__((__noinline__))
#define NOUNROLL                                                      \
    __attribute__((optimize("no-unroll-loops")))

#define NOCONSTPROP __attribute__((optimize("no-ipa-cp")))

#define __cpuid() asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx")
#define __nop() asm("nop")
#endif

#ifndef MICROBENCH_CACHE_H
#define MICROBENCH_CACHE_H

#define LINE_SIZE      (64)
#define L1d_CACHE_SIZE (32 * 1024)
#define L2_CACHE_SIZE  (256 * 1024)
#define L3_CACHE_SIZE  (8192 * 1024)

#define L1d_ASSOCIATIVITY (8) // TODO: Find associativity

#define L1d_NUM_SETS  (L1d_CACHE_SIZE / (LINE_SIZE * L1d_ASSOCIATIVITY))
#define L1d_NUM_LINES (L1d_CACHE_SIZE / LINE_SIZE)

// If these aren't marked volatile the compiler optimizes out the cache-filling
// operations.
extern volatile
    __attribute__((aligned(64))) unsigned char l1d_arr[L1d_CACHE_SIZE];
extern volatile
    __attribute__((aligned(64))) unsigned char l2_arr[L2_CACHE_SIZE];
extern volatile
    __attribute__((aligned(64))) unsigned char l3_arr[L3_CACHE_SIZE];
extern volatile
    __attribute__((aligned(64))) unsigned char target_cacheline[LINE_SIZE];
#endif
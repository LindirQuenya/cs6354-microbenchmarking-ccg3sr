#include "cache.h"

volatile __attribute__((aligned(64))) unsigned char l1d_arr[L1d_CACHE_SIZE];
volatile __attribute__((aligned(64))) unsigned char l2_arr[L2_CACHE_SIZE];
volatile __attribute__((aligned(64))) unsigned char l3_arr[L3_CACHE_SIZE];
volatile __attribute__((aligned(64))) unsigned char target_cacheline[LINE_SIZE];

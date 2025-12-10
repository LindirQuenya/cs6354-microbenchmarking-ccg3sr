#ifndef MICROBENCH_CPU_CARD_H
#define MICROBENCH_CPU_CARD_H

typedef struct {
	int function_call;
	int context_switch_syscall;
	int context_switch_pingpong;
} cpu_basic;

typedef struct {
	double fetch_throughput;
	double retire_throughput;
	double loads_per_cycle;
	double stores_per_cycle;
	int branch_mispredict_penalty;
	double int_alu_bandwidth;
	double int_mult_bandwidth;
	double int_div_bandwidth;
} cpu_pipeline;

typedef struct {
	int symbiotic_runtime;
	int contending_runtime;
} cpu_smt;

typedef struct {
	int l1i_latency;
	int l1d_latency;
	int l2_latency;
	int l3_latency;
} cpu_cache_latency;

typedef struct {
	double l1d_bandwidth;
	double l2_bandwidth;
	double l3_bandwidth;
} cpu_cache_bandwidth;

typedef struct {
	int dram_latency;
	double dram_bandwidth;
} cpu_main_memory;

typedef struct {
	cpu_basic basic;
	cpu_pipeline pipeline;
	cpu_smt smt;
	cpu_cache_latency cache_latency;
	cpu_cache_bandwidth cache_bandwidth;
	cpu_main_memory main_memory;
} cpu_card;

void displayCPUCard(cpu_card card);

#endif
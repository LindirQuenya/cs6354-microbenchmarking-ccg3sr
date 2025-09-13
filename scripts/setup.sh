#!/usr/bin/env bash
# We will schedule everything on CPU 1. Get ready.
# This whole thing is heavily inspired by:
#  - https://easyperf.net/blog/2019/08/02/Perf-measurement-environment-on-Linux
#  - https://llvm.org/docs/Benchmarking.html#linux

# Disable ASLR to make memory stuff consistent
echo 0 > /proc/sys/kernel/randomize_va_space

# Disable turbo boost to keep frequency scaling consistent
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo

# CPU 1 and CPU 5 are the same (SMT).
# You can discover this by looking at /sys/devices/system/cpu/cpu1/topology/thread_siblings_list
echo 0 > /sys/devices/system/cpu/cpu5/online

# Make power management be performance-focused rather than power-saving.
echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor


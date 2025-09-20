#!/usr/bin/env bash
# We will schedule everything on CPU 3. Get ready.
# This whole thing is heavily inspired by:
#  - https://easyperf.net/blog/2019/08/02/Perf-measurement-environment-on-Linux
#  - https://manuel.bernhardt.io/posts/2023-11-16-core-pinning/
#  - https://llvm.org/docs/Benchmarking.html#linux

# Disable in BIOS:
#  - Turbo boost
#  - Power save modes
#  - SMT (hyper-threading)

# Add to kernel parameters:
# nohz_full=3 isolcpus=domain,managed_irq,3 irqaffinity=0-2

# Disable ASLR to make memory stuff consistent
echo 0 > /proc/sys/kernel/randomize_va_space

# Disable turbo boost to keep frequency scaling consistent
# Disabled in BIOS
#echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo

# CPU 3 and CPU 7 are the same (SMT).
# You can discover this by looking at /sys/devices/system/cpu/cpu3/topology/thread_siblings_list
# Or lstopo will also show you it graphically.
# No longer needed, I've disabled hyper-threading.
#echo 0 > /sys/devices/system/cpu/cpu7/online

# Make power management be performance-focused rather than power-saving.
echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor

# Then use taskset -c 3 {command}

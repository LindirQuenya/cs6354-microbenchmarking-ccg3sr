#!/usr/bin/env bash
# Code to automatically compile and run all benchmarks
../make
# Run the benchmarks
../build/benchmark 100000

# Execute the Jupyter notebook and save output
jupyter nbconvert --to notebook --execute generate_cpu_card.ipynb
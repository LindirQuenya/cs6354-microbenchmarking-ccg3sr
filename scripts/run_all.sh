#!/usr/bin/env bash

# Move over to the script folder so that we can safely call this script from anywhere.
# Modified from: https://stackoverflow.com/a/246128
SCRIPT_DIR="$( cd -- "$( dirname -- "$(realpath -- "${BASH_SOURCE[0]}")" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Code to automatically compile and run all benchmarks
cd ..

make clean; make
# Run the benchmarks
./build/benchmark 1000000

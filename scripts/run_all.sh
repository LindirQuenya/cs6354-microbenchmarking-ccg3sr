#!/usr/bin/env bash

# Move over to the script folder so that we can safely call this script from anywhere.
# Modified from: https://stackoverflow.com/a/246128
SCRIPT_DIR="$( cd -- "$( dirname -- "$(realpath -- "${BASH_SOURCE[0]}")" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Code to automatically compile and run all benchmarks
cd ..

sudo apt install make git jupyter pip python3 python3.13-venv
if [[ ! -d venv ]]; then
  python3 -m venv venv
fi
source venv/bin/activate
pip install pandas matplotlib numpy

make
# Run the benchmarks
./build/benchmark 1000000

# Execute the Jupyter notebook and save output
cd notebooks
jupyter nbconvert --to notebook --execute generate_cpu_card.ipynb

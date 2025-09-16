#!/usr/bin/env bash
# Code to automatically compile and run all benchmarks
cd ..

sudo apt install make git jupyter pip python3 python3.13-venv
python3 -m venv venv
source venv/bin/activate
pip install pandas matplotlib numpy

make
# Run the benchmarks
./build/benchmark 100000

# Execute the Jupyter notebook and save output
cd notebooks
jupyter nbconvert --to notebook --execute generate_cpu_card.ipynb

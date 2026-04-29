#!/usr/bin/env bash
set -e
rm -f results.csv results_with_speedup.csv *.png

g++ -O2 -std=c++17 -o seq 1_sequential_benchmark.cpp
g++ -O2 -std=c++17 -pthread -o thr 2_threaded_benchmark.cpp
g++ -O2 -std=c++17 -fopenmp -o omp 3_openmp_benchmark.cpp

./seq
./thr
./omp

if command -v nvcc >/dev/null 2>&1; then
  nvcc -O2 -std=c++17 -o cuda_sort 4_cuda_benchmark.cu
  ./cuda_sort
else
  echo "nvcc not found; skipped CUDA benchmark. Run CUDA file on a CUDA-enabled machine."
fi

python3 plot_results.py

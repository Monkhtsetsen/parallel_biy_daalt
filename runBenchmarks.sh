#!/usr/bin/env bash
# ============================================================
#  Merge Sort — Benchmark runner
#  Бүх хувилбарыг compile хийж ажиллуулна
#  CUDA байхгүй бол автоматаар алгасна
# ============================================================
set -e
rm -f results.csv results_with_speedup.csv *.png

echo "==== Compile ===="
g++ -O2 -std=c++17 -o seq 1_sequential.cpp
g++ -O2 -std=c++17 -pthread -o thr 2_threaded.cpp
g++ -O2 -std=c++17 -fopenmp -o omp 3_openmp.cpp

echo ""
echo "==== 1. Sequential ===="
./seq

echo ""
echo "==== 2. std::thread ===="
./thr

echo ""
echo "==== 3. OpenMP ===="
./omp

echo ""
if command -v nvcc >/dev/null 2>&1; then
  echo "==== 4. CUDA ===="
  nvcc -O2 -std=c++17 -o cuda 4_cuda.cu && ./cuda || echo "  CUDA GPU олдсонгүй, алгасав."
else
  echo "==== 4. CUDA ==== (nvcc олдсонгүй, алгасав)"
fi

echo ""
echo "==== График үүсгэж байна... ===="
python3 plotresult.py

echo ""
echo "Дууслаа! results.csv болон *.png файлууд үүслээ."
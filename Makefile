# ============================================================
#  Merge Sort — Makefile
#  Бүгдийг хамтад нь build хийх: make all
#  Тус тусаар:                   make seq / thr / omp / cuda
#  Ажиллуулах:                   make run
#  Цэвэрлэх:                     make clean
# ============================================================

CXX      = g++
CXXFLAGS = -O2 -std=c++17
NVCC     = nvcc
NVFLAGS  = -O2

.PHONY: all seq thr omp cuda run clean

all: seq thr omp cuda

seq:
	$(CXX) $(CXXFLAGS) -o seq 1_sequential.cpp

thr:
	$(CXX) $(CXXFLAGS) -pthread -o thr 2_threaded.cpp

omp:
	$(CXX) $(CXXFLAGS) -fopenmp -o omp 3_openmp.cpp

cuda:
	$(NVCC) $(NVFLAGS) -o cuda 4_cuda.cu

run: all
	@echo ""
	@echo "==== 1. Sequential ===="
	@./seq
	@echo ""
	@echo "==== 2. std::thread ===="
	@./thr
	@echo ""
	@echo "==== 3. OpenMP ===="
	@./omp
	@echo ""
	@echo "==== 4. CUDA ===="
	@./cuda

clean:
	rm -f seq thr omp cuda

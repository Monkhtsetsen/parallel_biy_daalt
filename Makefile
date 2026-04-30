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
NVFLAGS  = -O2 -std=c++17

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

run: seq thr omp
	@echo ""
	@echo "==== 1. Sequential ===="
	@./seq
	@echo ""
	@echo "==== 2. std::thread ===="
	@./thr
	@echo ""
	@echo "==== 3. OpenMP ===="
	@./omp
	@if [ -f ./cuda ]; then \
		echo ""; \
		echo "==== 4. CUDA ===="; \
		./cuda; \
	else \
		echo ""; \
		echo "==== 4. CUDA ==== (binary олдсонгүй, make cuda гүйцэтгэнэ үү)"; \
	fi

clean:
	rm -f seq thr omp cuda results.csv results_with_speedup.csv *.png
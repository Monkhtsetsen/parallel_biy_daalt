// ============================================================
// Merge Sort Benchmark — 4. CUDA bottom-up version
// Output: appends benchmark results into results.csv
// Compile: nvcc -O2 -std=c++17 -o cuda_sort 4_cuda_benchmark.cu
// Run:     ./cuda_sort
// ============================================================
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cuda_runtime.h>

using namespace std;

#define CUDA_CHECK(call)                                                     \
    do {                                                                     \
        cudaError_t err = (call);                                            \
        if (err != cudaSuccess) {                                            \
            fprintf(stderr, "CUDA error at %s:%d — %s\n",                  \
                    __FILE__, __LINE__, cudaGetErrorString(err));            \
            exit(EXIT_FAILURE);                                              \
        }                                                                    \
    } while (0)

vector<int> makeRandomData(int n) {
    // Deterministic lightweight generator: same input as CPU versions.
    unsigned int x = 42u;
    vector<int> a(n);
    for (int &v : a) {
        x = 1664525u * x + 1013904223u;
        v = static_cast<int>(x % 1'000'001u);
    }
    return a;
}

long long estimatedOperations(int n) {
    if (n <= 1) return 0;
    return static_cast<long long>(llround(static_cast<double>(n) * log2(static_cast<double>(n))));
}

double achievableMOPS(int n, double totalMs) {
    if (totalMs <= 0.0) return 0.0;
    return (static_cast<double>(estimatedOperations(n)) / (totalMs / 1000.0)) / 1'000'000.0;
}

void ensureCsvHeader(const string &filename) {
    bool needHeader = true;
    FILE* check = fopen(filename.c_str(), "rb");
    if (check) {
        int c = fgetc(check);
        needHeader = (c == EOF);
        fclose(check);
    }
    if (needHeader) {
        FILE* out = fopen(filename.c_str(), "a");
        if (!out) {
            perror("Cannot open CSV file");
            exit(EXIT_FAILURE);
        }
        fprintf(out, "Algorithm,N,ExecutionTimeMs,H2DMs,KernelMs,D2HMs,TotalTimeMs,TransferredBytes,TotalOperations,AchievableMOPS,Correct,Workers,Notes\n");
        fclose(out);
    }
}

void appendResultCsv(
    const string &filename,
    const string &algorithm,
    int n,
    double executionMs,
    double h2dMs,
    double kernelMs,
    double d2hMs,
    double totalMs,
    long long transferredBytes,
    bool correct,
    const string &workers,
    const string &notes
) {
    ensureCsvHeader(filename);
    FILE* out = fopen(filename.c_str(), "a");
    if (!out) {
        perror("Cannot open CSV file");
        exit(EXIT_FAILURE);
    }
    fprintf(out, "%s,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%lld,%lld,%.6f,%s,%s,%s\n",
            algorithm.c_str(), n, executionMs, h2dMs, kernelMs, d2hMs, totalMs,
            transferredBytes, estimatedOperations(n), achievableMOPS(n, totalMs),
            correct ? "YES" : "NO", workers.c_str(), notes.c_str());
    fclose(out);
}

__global__ void mergeSortKernel(const int* src, int* dst, int n, int width) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int left = tid * 2 * width;
    if (left >= n) return;

    int mid = min(left + width, n);
    int right = min(left + 2 * width, n);

    int i = left;
    int j = mid;
    int k = left;

    while (i < mid && j < right) dst[k++] = (src[i] <= src[j]) ? src[i++] : src[j++];
    while (i < mid) dst[k++] = src[i++];
    while (j < right) dst[k++] = src[j++];
}

float elapsedMs(cudaEvent_t start, cudaEvent_t stop) {
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    return ms;
}

float copyH2D(int* d_arr, const vector<int>& h_arr) {
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));
    CUDA_CHECK(cudaMemcpy(d_arr, h_arr.data(), h_arr.size() * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = elapsedMs(start, stop);
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    return ms;
}

float copyD2H(vector<int>& h_arr, const int* d_arr) {
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));
    CUDA_CHECK(cudaMemcpy(h_arr.data(), d_arr, h_arr.size() * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = elapsedMs(start, stop);
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    return ms;
}

float runCudaMergeSort(int*& d_arr, int*& d_tmp, int n) {
    const int BLOCK_SIZE = 256;
    bool resultInArr = true;

    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));

    for (int width = 1; width < n; width *= 2) {
        int numMerges = (n + 2 * width - 1) / (2 * width);
        int gridSize = (numMerges + BLOCK_SIZE - 1) / BLOCK_SIZE;

        const int* src = resultInArr ? d_arr : d_tmp;
        int* dst = resultInArr ? d_tmp : d_arr;

        mergeSortKernel<<<gridSize, BLOCK_SIZE>>>(src, dst, n, width);
        CUDA_CHECK(cudaGetLastError());
        resultInArr = !resultInArr;
    }

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float kernelMs = elapsedMs(start, stop);

    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));

    if (!resultInArr) swap(d_arr, d_tmp);
    return kernelMs;
}

void benchmarkOne(int n, const string& csvFile) {
    vector<int> h_arr = makeRandomData(n);
    vector<int> expected = h_arr;
    sort(expected.begin(), expected.end());

    int *d_arr = nullptr, *d_tmp = nullptr;
    size_t bytes = static_cast<size_t>(n) * sizeof(int);
    CUDA_CHECK(cudaMalloc(&d_arr, bytes));
    CUDA_CHECK(cudaMalloc(&d_tmp, bytes));

    float h2dMs = copyH2D(d_arr, h_arr);
    float kernelMs = runCudaMergeSort(d_arr, d_tmp, n);
    float d2hMs = copyD2H(h_arr, d_arr);

    double totalMs = static_cast<double>(h2dMs + kernelMs + d2hMs);
    bool correct = (h_arr == expected);
    long long transferredBytes = static_cast<long long>(2LL * n * sizeof(int));

    appendResultCsv(csvFile, "CUDA", n, kernelMs, h2dMs, kernelMs, d2hMs, totalMs,
                    transferredBytes, correct, "block=256", "bottom_up_merge_sort");

    printf("  CUDA        | n = %9d | H2D: %8.3f ms | Kernel: %8.3f ms | D2H: %8.3f ms | Total: %9.3f ms | %s\n",
           n, h2dMs, kernelMs, d2hMs, totalMs, correct ? "OK" : "ERROR");

    CUDA_CHECK(cudaFree(d_arr));
    CUDA_CHECK(cudaFree(d_tmp));
}

int main() {
    const string csvFile = "results.csv";

    int device = 0;
    CUDA_CHECK(cudaSetDevice(device));
    cudaDeviceProp prop{};
    CUDA_CHECK(cudaGetDeviceProperties(&prop, device));

    cout << "====================================================\n";
    cout << "  Merge Sort Benchmark — CUDA\n";
    cout << "  GPU: " << prop.name
         << " | SM: " << prop.multiProcessorCount
         << " | Global memory: " << prop.totalGlobalMem / (1024 * 1024) << " MB\n";
    cout << "====================================================\n";

    benchmarkOne(10'000, csvFile);
    benchmarkOne(100'000, csvFile);
    benchmarkOne(1'000'000, csvFile);

    cout << "====================================================\n";
    cout << "Saved/appended to " << csvFile << "\n";
    return 0;
}

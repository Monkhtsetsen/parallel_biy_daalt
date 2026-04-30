// ============================================================
//  Merge Sort — 4. CUDA (GPU) хувилбар
//  Compile: nvcc -O2 -std=c++17 -o cuda 4_cuda.cu
//  Run:     ./cuda
//
//  Арга: Bottom-up iterative merge sort
//  - Эхний давталт: хэмжээ=1 блокуудыг нэгтгэнэ (2 болгоно)
//  - Дараа нь: хэмжээ=2 → 4 → 8 → ... → n
//  - Нэг GPU thread нэг merge операцийг хийнэ
//
//  CSV гаралт: results.csv (benchmark_utils.hpp-тэй нийцтэй)
// ============================================================

#include "benchmark_utils.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>
#include <cuda_runtime.h>

using namespace std;

// ----------------------------------------------------------
//  CUDA алдаа шалгах macro
// ----------------------------------------------------------
#define CUDA_CHECK(call)                                              \
    do {                                                              \
        cudaError_t err = (call);                                     \
        if (err != cudaSuccess) {                                     \
            fprintf(stderr, "CUDA error at %s:%d — %s\n",           \
                    __FILE__, __LINE__, cudaGetErrorString(err));     \
            exit(EXIT_FAILURE);                                       \
        }                                                             \
    } while (0)

// ----------------------------------------------------------
//  GPU kernel: нэг thread нэг sub-array хос нэгтгэнэ
//
//  arr   — эрэмбэлэх массив (device)
//  tmp   — түр санах ой (device)
//  n     — нийт элемент тоо
//  width — одоогийн нэгтгэлийн блокийн хагас хэмжээ
// ----------------------------------------------------------
__global__ void mergeSortKernel(int* arr, int* tmp, int n, int width) {
    // Энэ thread-ийн нэгтгэх блокийн эхлэл
    int tid   = blockIdx.x * blockDim.x + threadIdx.x;
    int left  = tid * 2 * width;

    if (left >= n) return;   // хязгаараас гарсан thread буцна

    int mid   = min(left + width - 1, n - 1);
    int right = min(left + 2 * width - 1, n - 1);

    // left..mid ба mid+1..right хэсгүүдийг нэгтгэнэ → tmp руу
    int i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) tmp[k++] = arr[i++];
        else                  tmp[k++] = arr[j++];
    }
    while (i <= mid)   tmp[k++] = arr[i++];
    while (j <= right) tmp[k++] = arr[j++];

    // tmp-ийн үр дүнг arr руу хуулна
    for (int x = left; x <= right; x++)
        arr[x] = tmp[x];
}

// ----------------------------------------------------------
//  CPU талын bottom-up CUDA merge sort дуудлага
// ----------------------------------------------------------
void mergeSortCUDA(int* d_arr, int* d_tmp, int n) {
    const int BLOCK_SIZE = 256;

    // width: 1, 2, 4, ..., n/2 давталт
    for (int width = 1; width < n; width *= 2) {
        // Хичнээн thread хэрэгтэй вэ?
        int numMerges = (n + 2 * width - 1) / (2 * width);
        int gridSize  = (numMerges + BLOCK_SIZE - 1) / BLOCK_SIZE;

        mergeSortKernel<<<gridSize, BLOCK_SIZE>>>(d_arr, d_tmp, n, width);
        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaDeviceSynchronize());
    }
}

// ----------------------------------------------------------
//  Benchmark нэг хэмжээ дээр ажиллуулж CSV-д бичнэ
//  H2D, Kernel, D2H хугацааг тусад нь хэмжинэ
// ----------------------------------------------------------
void benchmarkOne(int n, const string& csvFile) {
    // benchmark_utils-тай ижил random өгөгдөл үүсгэнэ
    vector<int> h_arr = makeRandomData(n);

    vector<int> expected = h_arr;
    sort(expected.begin(), expected.end());

    // Device санах ой хуваарилна
    int *d_arr, *d_tmp;
    CUDA_CHECK(cudaMalloc(&d_arr, n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_tmp, n * sizeof(int)));

    long long transferredBytes = 2LL * n * sizeof(int); // H2D + D2H

    // ---- Host → Device (H2D) хугацаа ----
    cudaEvent_t evH2D_s, evH2D_e;
    CUDA_CHECK(cudaEventCreate(&evH2D_s));
    CUDA_CHECK(cudaEventCreate(&evH2D_e));
    CUDA_CHECK(cudaEventRecord(evH2D_s));
    CUDA_CHECK(cudaMemcpy(d_arr, h_arr.data(), n * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaEventRecord(evH2D_e));
    CUDA_CHECK(cudaEventSynchronize(evH2D_e));
    float h2dMs = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&h2dMs, evH2D_s, evH2D_e));
    CUDA_CHECK(cudaEventDestroy(evH2D_s));
    CUDA_CHECK(cudaEventDestroy(evH2D_e));

    // ---- Kernel хугацаа ----
    cudaEvent_t evK_s, evK_e;
    CUDA_CHECK(cudaEventCreate(&evK_s));
    CUDA_CHECK(cudaEventCreate(&evK_e));
    CUDA_CHECK(cudaEventRecord(evK_s));
    mergeSortCUDA(d_arr, d_tmp, n);
    CUDA_CHECK(cudaEventRecord(evK_e));
    CUDA_CHECK(cudaEventSynchronize(evK_e));
    float kernelMs = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&kernelMs, evK_s, evK_e));
    CUDA_CHECK(cudaEventDestroy(evK_s));
    CUDA_CHECK(cudaEventDestroy(evK_e));

    // ---- Device → Host (D2H) хугацаа ----
    cudaEvent_t evD2H_s, evD2H_e;
    CUDA_CHECK(cudaEventCreate(&evD2H_s));
    CUDA_CHECK(cudaEventCreate(&evD2H_e));
    CUDA_CHECK(cudaEventRecord(evD2H_s));
    CUDA_CHECK(cudaMemcpy(h_arr.data(), d_arr, n * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaEventRecord(evD2H_e));
    CUDA_CHECK(cudaEventSynchronize(evD2H_e));
    float d2hMs = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&d2hMs, evD2H_s, evD2H_e));
    CUDA_CHECK(cudaEventDestroy(evD2H_s));
    CUDA_CHECK(cudaEventDestroy(evD2H_e));

    double totalMs = (double)h2dMs + (double)kernelMs + (double)d2hMs;

    bool correct = (h_arr == expected);

    // GPU нэр
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    string gpuName = prop.name;

    // CSV-д бичнэ (benchmark_utils-тай нийцтэй)
    appendResultCsv(
        csvFile,
        "CUDA",
        n,
        (double)kernelMs,   // ExecutionTimeMs = зөвхөн kernel
        (double)h2dMs,      // H2DMs
        (double)kernelMs,   // KernelMs
        (double)d2hMs,      // D2HMs
        totalMs,            // TotalTimeMs = H2D + Kernel + D2H
        transferredBytes,
        correct,
        "1",                // workers = 1 GPU
        "bottom_up_merge_sort_gpu=" + gpuName
    );

    printResult("CUDA", n, totalMs, correct,
        "H2D=" + to_string((int)h2dMs) + "ms"
        + " K=" + to_string((int)kernelMs) + "ms"
        + " D2H=" + to_string((int)d2hMs) + "ms");

    // Цэвэрлэх
    CUDA_CHECK(cudaFree(d_arr));
    CUDA_CHECK(cudaFree(d_tmp));
}

int main() {
    const string csvFile = "results.csv";

    // GPU мэдээлэл харуулах
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));

    cout << "====================================================\n";
    cout << "  Merge Sort Benchmark — CUDA (GPU параллел)\n";
    cout << "  GPU: " << prop.name
         << "  |  SM: " << prop.multiProcessorCount
         << "  |  Санах ой: " << prop.totalGlobalMem / (1024*1024) << " MB\n";
    cout << "====================================================\n";

    benchmarkOne(10'000,   csvFile);
    benchmarkOne(100'000,  csvFile);
    benchmarkOne(1'000'000, csvFile);

    cout << "====================================================\n";
    cout << "Saved/appended to " << csvFile << "\n";

    return 0;
}
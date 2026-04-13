// ============================================================
//  Merge Sort — 4. CUDA (GPU) хувилбар
//  Хэрэглээ: nvcc -O2 -o cuda 4_cuda.cu && ./cuda
//
//  Арга: Bottom-up iterative merge sort
//  - Эхний давталт: хэмжээ=1 блокуудыг нэгтгэнэ (2 болгоно)
//  - Дараа нь: хэмжээ=2 → 4 → 8 → ... → n
//  - Нэг GPU thread нэг merge операцийг хийнэ
// ============================================================
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>
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
//  Туршилт
// ----------------------------------------------------------
void runTest(int n) {
    mt19937 rng(42);
    uniform_int_distribution<int> dist(0, 1'000'000);
    vector<int> h_arr(n), expected(n);
    for (auto& x : h_arr) x = dist(rng);
    expected = h_arr;
    sort(expected.begin(), expected.end());

    // Device санах ой хуваарилна
    int *d_arr, *d_tmp;
    CUDA_CHECK(cudaMalloc(&d_arr, n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_tmp, n * sizeof(int)));

    // Host → Device
    CUDA_CHECK(cudaMemcpy(d_arr, h_arr.data(), n * sizeof(int), cudaMemcpyHostToDevice));

    // CUDA event-ээр хугацаа хэмжих
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));

    mergeSortCUDA(d_arr, d_tmp, n);

    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));

    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));

    // Device → Host
    CUDA_CHECK(cudaMemcpy(h_arr.data(), d_arr, n * sizeof(int), cudaMemcpyDeviceToHost));

    bool correct = (h_arr == expected);

    // GPU мэдээлэл
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));

    cout << fixed << setprecision(3);
    cout << "  n = " << setw(9) << n
         << "  |  Хугацаа: " << setw(10) << ms << " мс"
         << "  |  GPU: " << prop.name
         << "  |  " << (correct ? "✓ Зөв" : "✗ Алдаа") << "\n";

    // Цэвэрлэх
    CUDA_CHECK(cudaFree(d_arr));
    CUDA_CHECK(cudaFree(d_tmp));
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
}

int main() {
    // GPU мэдээлэл харуулах
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    cout << "====================================================\n";
    cout << "  Merge Sort — CUDA (GPU параллел)\n";
    cout << "  GPU: " << prop.name
         << "  |  SM: " << prop.multiProcessorCount
         << "  |  Санах ой: " << prop.totalGlobalMem / (1024*1024) << " MB\n";
    cout << "====================================================\n";
    runTest(10'000);
    runTest(100'000);
    runTest(1'000'000);
    cout << "====================================================\n";
    return 0;
}

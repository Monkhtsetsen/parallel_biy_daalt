// ============================================================
// Merge Sort Benchmark — 3. OpenMP version
// Output: appends benchmark results into results.csv
// Compile: g++ -O2 -std=c++17 -fopenmp -o omp 3_openmp_benchmark.cpp
// Run:     ./omp

#include "benchmark_utils.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <omp.h>

using namespace std;
using namespace chrono;

// Merge helper function
void mergeParts(vector<int>& arr, int left, int mid, int right) {
    vector<int> L(arr.begin() + left, arr.begin() + mid + 1);
    vector<int> R(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0;
    int j = 0;
    int k = left;

    while (i < (int)L.size() && j < (int)R.size()) {
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < (int)L.size()) {
        arr[k++] = L[i++];
    }

    while (j < (int)R.size()) {
        arr[k++] = R[j++];
    }
}

// Sequential merge sort
// Used when subarray is too small.
void mergeSortSeq(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    mergeSortSeq(arr, left, mid);
    mergeSortSeq(arr, mid + 1, right);

    mergeParts(arr, left, mid, right);
}

// OpenMP task-based parallel merge sort
//
// #pragma omp task:
//   Left and right recursive calls become independent tasks.
//
// #pragma omp taskwait:
//   Wait until both halves are sorted before merge.
//
// cutoff:
//   Small arrays are sorted sequentially to reduce task overhead.
const int OMP_CUTOFF = 4096;

void mergeSortOMP(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int size = right - left + 1;

    if (size <= OMP_CUTOFF) {
        mergeSortSeq(arr, left, right);
        return;
    }

    int mid = left + (right - left) / 2;

    #pragma omp task shared(arr) firstprivate(left, mid)
    {
        mergeSortOMP(arr, left, mid);
    }

    #pragma omp task shared(arr) firstprivate(mid, right)
    {
        mergeSortOMP(arr, mid + 1, right);
    }

    #pragma omp taskwait

    mergeParts(arr, left, mid, right);
}

double runOpenMP(vector<int>& arr, int threads) {
    if (arr.empty()) return 0.0;

    omp_set_num_threads(threads);

    auto start = high_resolution_clock::now();

    #pragma omp parallel
    {
        #pragma omp single
        {
            mergeSortOMP(arr, 0, (int)arr.size() - 1);
        }
    }

    auto end = high_resolution_clock::now();

    return duration<double, milli>(end - start).count();
}

// Benchmark one input size
void benchmarkOne(int n, const string& csvFile, int threads) {
    vector<int> arr = makeRandomData(n);

    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    double ms = runOpenMP(arr, threads);

    bool correct = (arr == expected);

    appendResultCsv(
        csvFile,
        "OpenMP",
        n,
        ms,
        0.0,
        0.0,
        0.0,
        ms,
        0,
        correct,
        to_string(threads),
        "task_based_merge_sort_cutoff=" + to_string(OMP_CUTOFF)
    );

    printResult(
        "OpenMP",
        n,
        ms,
        correct,
        "threads=" + to_string(threads)
    );
}

int main() {
    const string csvFile = "results.csv";

    int maxThreads = omp_get_max_threads();

    cout << "  Merge Sort Benchmark — OpenMP\n";
    cout << "  OpenMP max threads = " << maxThreads << "\n";
    cout << "  Cutoff = " << OMP_CUTOFF << "\n";

    benchmarkOne(10'000, csvFile, maxThreads);
    benchmarkOne(100'000, csvFile, maxThreads);
    benchmarkOne(1'000'000, csvFile, maxThreads);
    cout << "Saved/appended to " << csvFile << "\n";

    return 0;
}
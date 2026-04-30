// Merge Sort Benchmark — 2. std::thread version
// Output: appends benchmark results into results.csv
// Compile Linux/MSYS2: g++ -O2 -std=c++17 -pthread -o thr 2_threaded_benchmark.cpp
// Compile MSVC:        cl /EHsc /O2 2_threaded_benchmark.cpp
// Run:                 ./thr

#include "benchmark_utils.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;
using namespace chrono;

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

void mergeSortSeq(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    mergeSortSeq(arr, left, mid);
    mergeSortSeq(arr, mid + 1, right);

    mergeParts(arr, left, mid, right);
}

int computeMaxDepth() {
    unsigned int hw = thread::hardware_concurrency();

    if (hw == 0) {
        hw = 4;
    }

    int depth = (int)floor(log2((double)hw));

    // Жишээ:
    // hw = 8 бол depth = 3, хамгийн ихдээ 2^3 = 8 worker гэж үзнэ.
    if (depth < 1) {
        depth = 1;
    }

    // Хэт олон thread үүсгэхээс хамгаална.
    // 4 depth => ойролцоогоор 16 worker хүртэл.
    if (depth > 4) {
        depth = 4;
    }

    return depth;
}

void mergeSortThreaded(vector<int>& arr, int left, int right, int depth, int maxDepth) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    if (depth < maxDepth) {
        thread leftThread(
            [&arr, left, mid, depth, maxDepth]() {
                mergeSortThreaded(arr, left, mid, depth + 1, maxDepth);
            }
        );

        mergeSortThreaded(arr, mid + 1, right, depth + 1, maxDepth);

        // Баруун, зүүн тал хоёулаа sorted болсны дараа merge хийнэ.
        leftThread.join();
    } else {
        mergeSortSeq(arr, left, mid);
        mergeSortSeq(arr, mid + 1, right);
    }

    mergeParts(arr, left, mid, right);
}

double runThreaded(vector<int>& arr, int maxDepth) {
    if (arr.empty()) return 0.0;

    auto start = high_resolution_clock::now();

    mergeSortThreaded(arr, 0, (int)arr.size() - 1, 0, maxDepth);

    auto end = high_resolution_clock::now();

    return duration<double, milli>(end - start).count();
}

void benchmarkOne(int n, const string& csvFile, int maxDepth) {
    vector<int> arr = makeRandomData(n);

    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    double ms = runThreaded(arr, maxDepth);

    bool correct = (arr == expected);

    int maxWorkers = 1 << maxDepth;

    appendResultCsv(
        csvFile,
        "std_thread",
        n,
        ms,
        0.0,
        0.0,
        0.0,
        ms,
        0,
        correct,
        to_string(maxWorkers),
        "max_depth=" + to_string(maxDepth)
    );

    printResult(
        "std_thread",
        n,
        ms,
        correct,
        "workers<= " + to_string(maxWorkers)
    );
}

int main() {
    const string csvFile = "results.csv";

    int maxDepth = computeMaxDepth();
    unsigned int hw = thread::hardware_concurrency();

    cout << "  Merge Sort Benchmark — std::thread\n";
    cout << "  hardware_concurrency = " << hw
         << ", maxDepth = " << maxDepth << "\n";

    benchmarkOne(10'000, csvFile, maxDepth);
    benchmarkOne(100'000, csvFile, maxDepth);
    benchmarkOne(1'000'000, csvFile, maxDepth);

    cout << "Saved/appended to " << csvFile << "\n";

    return 0;
}
// ============================================================
// Merge Sort Benchmark — 3. OpenMP version
// Output: appends benchmark results into results.csv
// Compile: g++ -O2 -std=c++17 -fopenmp -o omp 3_openmp.cpp
// Run:     ./omp
// Merge Sort Benchmark — OpenMP version
// Compile:
// g++ -O2 -std=c++17 -fopenmp -o omp 3_openmp.cpp

#include "benchmark_utils.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <omp.h>

using namespace std;
using namespace chrono;

// 2 sorted hesgiig negtgeh function
void mergeParts(vector<int>& arr, int left, int mid, int right) {

    // Zuun heseg
    vector<int> L(arr.begin() + left, arr.begin() + mid + 1);

    // Baruun heseg
    vector<int> R(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0;
    int j = 0;
    int k = left;

    // Hoyr hesgiig haritsuulan negtgeh
    while (i < (int)L.size() && j < (int)R.size()) {
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    // Uldsen elementuudiig huulah
    while (i < (int)L.size()) {
        arr[k++] = L[i++];
    }

    while (j < (int)R.size()) {
        arr[k++] = R[j++];
    }
}

// Sequential merge sort
// Jijig heseg deer ashiglana
void mergeSortSeq(vector<int>& arr, int left, int right) {

    // Neg element bol recursion zogsono
    if (left >= right)
        return;

    // Dund heseg oloh
    int mid = left + (right - left) / 2;

    // Zuun hesgiig erembeleh
    mergeSortSeq(arr, left, mid);

    // Baruun hesgiig erembeleh
    mergeSortSeq(arr, mid + 1, right);

    // Hoyr hesgiig negtgeh
    mergeParts(arr, left, mid, right);
}

// Jijig array deer parallel task uusgehgui
const int OMP_CUTOFF = 4096;

// OpenMP task-based parallel merge sort
void mergeSortOMP(vector<int>& arr, int left, int right) {

    // Neg element bol recursion zogsono
    if (left >= right)
        return;

    // Odoogiin hesgiin hemjee
    int size = right - left + 1;

    // Jijig heseg bol sequential sort ashiglana
    if (size <= OMP_CUTOFF) {
        mergeSortSeq(arr, left, right);
        return;
    }

    // Dund heseg oloh
    int mid = left + (right - left) / 2;

    // Zuun hesgiig OpenMP task bolgon ajilluulah
    #pragma omp task shared(arr) firstprivate(left, mid)
    {
        mergeSortOMP(arr, left, mid);
    }

    // Baruun hesgiig OpenMP task bolgon ajilluulah
    #pragma omp task shared(arr) firstprivate(mid, right)
    {
        mergeSortOMP(arr, mid + 1, right);
    }

    // Hoyr task duusahig huleeh
    #pragma omp taskwait

    // Hoyr sorted hesgiig negtgeh
    mergeParts(arr, left, mid, right);
}

// OpenMP version-iin hugatsaag hemjih
double runOpenMP(vector<int>& arr, int threads) {

    if (arr.empty())
        return 0.0;

    // Ashiglah thread-iin too
    omp_set_num_threads(threads);

    auto start = high_resolution_clock::now();

    // OpenMP parallel region uusgeh
    #pragma omp parallel
    {
        // Recursion-iig neg thread ehluulne
        #pragma omp single
        {
            mergeSortOMP(arr, 0, (int)arr.size() - 1);
        }
    }

    auto end = high_resolution_clock::now();

    // Millisecond-eer butsaah
    return duration<double, milli>(end - start).count();
}

// Neg hemjeen deer benchmark hiih
void benchmarkOne(int n, const string& csvFile, int threads) {

    // Random ogogdol uusgeh
    vector<int> arr = makeRandomData(n);

    // Zov hariutai haritsuulah huvilbar
    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    // OpenMP merge sort ajilluulah
    double ms = runOpenMP(arr, threads);

    // Zov erembelsen eseh
    bool correct = (arr == expected);

    // CSV file ruu ur dung hadgalah
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

    // Console deer ur dung hevleh
    printResult(
        "OpenMP",
        n,
        ms,
        correct,
        "threads=" + to_string(threads)
    );
}

int main() {

    // Benchmark ur dung hadgalah file
    const string csvFile = "results.csv";

    // OpenMP-iin ashiglaj chadah hamgiin ih thread
    int maxThreads = omp_get_max_threads();

    cout << "  Merge Sort Benchmark — OpenMP\n";
    cout << "  OpenMP max threads = " << maxThreads << "\n";
    cout << "  Cutoff = " << OMP_CUTOFF << "\n";

    // Yanz buriin hemjeen deer benchmark hiih
    benchmarkOne(10'000, csvFile, maxThreads);
    benchmarkOne(100'000, csvFile, maxThreads);
    benchmarkOne(1'000'000, csvFile, maxThreads);

    cout << "Saved/appended to " << csvFile << "\n";

    return 0;
}
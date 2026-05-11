// Merge Sort Benchmark — 2. std::thread version
// Output: appends benchmark results into results.csv
// Compile Linux/MSYS2: g++ -O2 -std=c++17 -pthread -o thr 2_threaded_benchmark.cpp
// Compile MSVC:        cl /EHsc /O2 2_threaded_benchmark.cpp
// Run:                 ./thr

// Merge Sort Benchmark — std::thread version
// Compile Linux/MSYS2:
// g++ -O2 -std=c++17 -pthread -o thr 2_threaded_benchmark.cpp

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

// ----------------------------------------------------------
// 2 sorted hesgiig negtgeh function
// ----------------------------------------------------------
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

    // Zuun hesgiin uldsen elementuudiig huulah
    while (i < (int)L.size()) {
        arr[k++] = L[i++];
    }

    // Baruun hesgiin uldsen elementuudiig huulah
    while (j < (int)R.size()) {
        arr[k++] = R[j++];
    }
}

// ----------------------------------------------------------
// Sequential merge sort
// Parallel recursion duussanii daraa ashiglana
// ----------------------------------------------------------
void mergeSortSeq(
    vector<int>& arr,
    int left,
    int right
) {

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

// ----------------------------------------------------------
// Hardware deer undeslen recursion-iin gun tootsoh
// ----------------------------------------------------------
int computeMaxDepth() {

    // CPU deer baigaa hardware thread-iin too
    unsigned int hw = thread::hardware_concurrency();

    // Oldohgui bol default utga ashiglah
    if (hw == 0) {
        hw = 4;
    }

    // log2 ashiglan recursion-iin gun tootsoh
    int depth =
        (int)floor(log2((double)hw));

    // Hamgiin bagadaa 1 baina
    if (depth < 1) {
        depth = 1;
    }

    // Het olon thread uusgehees hamgaalna
    if (depth > 4) {
        depth = 4;
    }

    return depth;
}

// ----------------------------------------------------------
// std::thread ashiglasan parallel merge sort
// ----------------------------------------------------------
void mergeSortThreaded(
    vector<int>& arr,
    int left,
    int right,
    int depth,
    int maxDepth
) {

    // Neg element bol recursion zogsono
    if (left >= right)
        return;

    // Dund heseg oloh
    int mid = left + (right - left) / 2;

    // Max depth hureegui bol shine thread uusgeh
    if (depth < maxDepth) {

        // Zuun hesgiig shine thread deer ajilluulah
        thread leftThread(
            [&arr, left, mid, depth, maxDepth]() {

                mergeSortThreaded(
                    arr,
                    left,
                    mid,
                    depth + 1,
                    maxDepth
                );
            }
        );

        // Baruun hesgiig undsen thread deer ajilluulah
        mergeSortThreaded(
            arr,
            mid + 1,
            right,
            depth + 1,
            maxDepth
        );

        // Zuun thread duusahig huleeh
        leftThread.join();

    } else {

        // Max depth hursen bol sequential merge sort ashiglana
        mergeSortSeq(arr, left, mid);

        mergeSortSeq(arr, mid + 1, right);
    }

    // Hoyr hesgiig negtgeh
    mergeParts(arr, left, mid, right);
}

// ----------------------------------------------------------
// std::thread version-iin hugatsaag hemjih
// ----------------------------------------------------------
double runThreaded(
    vector<int>& arr,
    int maxDepth
) {

    if (arr.empty())
        return 0.0;

    // Hugatsaa hemjilt ehleh
    auto start =
        high_resolution_clock::now();

    // Parallel merge sort ajilluulah
    mergeSortThreaded(
        arr,
        0,
        (int)arr.size() - 1,
        0,
        maxDepth
    );

    // Hugatsaa hemjilt duusah
    auto end =
        high_resolution_clock::now();

    // Millisecond-eer butsaah
    return duration<double, milli>(
        end - start
    ).count();
}

// ----------------------------------------------------------
// Neg hemjeen deer benchmark hiih
// ----------------------------------------------------------
void benchmarkOne(
    int n,
    const string& csvFile,
    int maxDepth
) {

    // Random ogogdol uusgeh
    vector<int> arr = makeRandomData(n);

    // Zov hariutai haritsuulah huvilbar
    vector<int> expected = arr;

    // CPU standard sort ashiglan erembeleh
    sort(expected.begin(), expected.end());

    // std::thread merge sort ajilluulah
    double ms =
        runThreaded(arr, maxDepth);

    // Zov erembelsen eseh
    bool correct = (arr == expected);

    // Hamgiin ih bolomjit worker-iin too
    int maxWorkers = 1 << maxDepth;

    // CSV file ruu ur dung hadgalah
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

    // Console deer ur dung hevleh
    printResult(
        "std_thread",
        n,
        ms,
        correct,
        "workers<= " + to_string(maxWorkers)
    );
}

// ----------------------------------------------------------
// Main function
// ----------------------------------------------------------
int main() {

    // Benchmark ur dung hadgalah file
    const string csvFile = "results.csv";

    // Parallel recursion-iin gun
    int maxDepth = computeMaxDepth();

    // CPU hardware thread-iin too
    unsigned int hw =
        thread::hardware_concurrency();

    cout << "========================================\n";

    cout << " Merge Sort Benchmark — std::thread\n";

    cout << " hardware_concurrency = "
         << hw
         << ", maxDepth = "
         << maxDepth
         << "\n";

    cout << "========================================\n";

    // Yanz buriin hemjeen deer benchmark hiih
    benchmarkOne(
        10'000,
        csvFile,
        maxDepth
    );

    benchmarkOne(
        100'000,
        csvFile,
        maxDepth
    );

    benchmarkOne(
        1'000'000,
        csvFile,
        maxDepth
    );

    cout << "========================================\n";

    cout << "Saved/appended to "
         << csvFile
         << "\n";

    return 0;
}
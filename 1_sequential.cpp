// Merge Sort Benchmark — 1. Sequential version
// Output: appends benchmark results into results.csv
// Compile: g++ -O2 -std=c++17 -o seq 1_sequential.cpp
// Run:     ./seq
// Merge Sort Benchmark — 2. std::thread version
// Output: appends benchmark results into results.csv
// Compile Linux/MSYS2: g++ -O2 -std=c++17 -pthread -o thr 2_threaded_benchmark.cpp
// Compile MSVC:        cl /EHsc /O2 2_threaded_benchmark.cpp
// Run:                 ./thr
// Merge Sort Benchmark — Sequential version
// Compile:
// g++ -O2 -std=c++17 -o seq 1_sequential.cpp

#include "benchmark_utils.hpp"

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
// Recursive sequential merge sort
// ----------------------------------------------------------
void mergeSortSequential(
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
    mergeSortSequential(arr, left, mid);

    // Baruun hesgiig erembeleh
    mergeSortSequential(arr, mid + 1, right);

    // Hoyr hesgiig negtgeh
    mergeParts(arr, left, mid, right);
}

// ----------------------------------------------------------
// Sequential merge sort-iin hugatsaag hemjih
// ----------------------------------------------------------
double runSequential(vector<int>& arr) {

    // Hugatsaa hemjilt ehleh
    auto start = high_resolution_clock::now();

    // Merge sort ajilluulah
    mergeSortSequential(
        arr,
        0,
        (int)arr.size() - 1
    );

    // Hugatsaa hemjilt duusah
    auto end = high_resolution_clock::now();

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
    const string& csvFile
) {

    // Random ogogdol uusgeh
    vector<int> arr = makeRandomData(n);

    // Zov hariutai haritsuulah huvilbar
    vector<int> expected = arr;

    // CPU standard sort ashiglan erembeleh
    sort(expected.begin(), expected.end());

    // Sequential merge sort ajilluulah
    double ms = runSequential(arr);

    // Zov erembelsen eseh
    bool correct = (arr == expected);

    // CSV file ruu ur dung hadgalah
    appendResultCsv(
        csvFile,
        "Sequential",
        n,
        ms,
        0.0,
        0.0,
        0.0,
        ms,
        0,
        correct,
        "1",
        "recursive_merge_sort"
    );

    // Console deer ur dung hevleh
    printResult(
        "Sequential",
        n,
        ms,
        correct
    );
}

// ----------------------------------------------------------
// Main function
// ----------------------------------------------------------
int main() {

    // Benchmark ur dung hadgalah file
    const string csvFile = "results.csv";

    cout << "========================================\n";
    cout << " Merge Sort Benchmark — Sequential\n";
    cout << "========================================\n";

    // Yanz buriin hemjeen deer benchmark hiih
    benchmarkOne(10'000, csvFile);

    benchmarkOne(100'000, csvFile);

    benchmarkOne(1'000'000, csvFile);

    cout << "========================================\n";

    cout << "Saved/appended to "
         << csvFile
         << "\n";

    return 0;
}
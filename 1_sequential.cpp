// Merge Sort Benchmark — 1. Sequential version
// Output: appends benchmark results into results.csv
// Compile: g++ -O2 -std=c++17 -o seq 1_sequential.cpp
// Run:     ./seq
#include "benchmark_utils.hpp"

using namespace std;
using namespace chrono;

void mergeParts(vector<int>& arr, int left, int mid, int right) {
    vector<int> L(arr.begin() + left, arr.begin() + mid + 1);
    vector<int> R(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0, j = 0, k = left;
    while (i < (int)L.size() && j < (int)R.size()) {
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }
    while (i < (int)L.size()) arr[k++] = L[i++];
    while (j < (int)R.size()) arr[k++] = R[j++];
}

void mergeSortSequential(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortSequential(arr, left, mid);
    mergeSortSequential(arr, mid + 1, right);
    mergeParts(arr, left, mid, right);
}

double runSequential(vector<int>& arr) {
    auto start = high_resolution_clock::now();
    mergeSortSequential(arr, 0, (int)arr.size() - 1);
    auto end = high_resolution_clock::now();
    return duration<double, milli>(end - start).count();
}

void benchmarkOne(int n, const string& csvFile) {
    vector<int> arr = makeRandomData(n);
    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    double ms = runSequential(arr);
    bool correct = (arr == expected);

    appendResultCsv(csvFile, "Sequential", n, ms, 0.0, 0.0, 0.0, ms, 0, correct, "1", "recursive_merge_sort");
    printResult("Sequential", n, ms, correct);
}

int main() {
    const string csvFile = "results.csv";
    cout << "====================================================\n";
    cout << "  Merge Sort Benchmark — Sequential\n";
    cout << "====================================================\n";

    benchmarkOne(10'000, csvFile);
    benchmarkOne(100'000, csvFile);
    benchmarkOne(1'000'000, csvFile);

    cout << "====================================================\n";
    cout << "Saved/appended to " << csvFile << "\n";
    return 0;
}

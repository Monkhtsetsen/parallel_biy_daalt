// ============================================================
//  Merge Sort — 1. Дараалсан (Sequential) хувилбар
//  Хэрэглээ: g++ -O2 -o seq 1_sequential.cpp && ./seq
// ============================================================
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>

using namespace std;
using namespace chrono;

// ----------------------------------------------------------
//  Нэгтгэх туслах функц
// ----------------------------------------------------------
void merge(vector<int>& arr, int left, int mid, int right) {
    // Зүүн болон баруун хагас хуулбар
    vector<int> L(arr.begin() + left,  arr.begin() + mid + 1);
    vector<int> R(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0, j = 0, k = left;
    while (i < (int)L.size() && j < (int)R.size())
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < (int)L.size()) arr[k++] = L[i++];
    while (j < (int)R.size()) arr[k++] = R[j++];
}

// ----------------------------------------------------------
//  Рекурсив merge sort
// ----------------------------------------------------------
void mergeSort(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// ----------------------------------------------------------
//  Туршилт явуулах
// ----------------------------------------------------------
void runTest(int n) {
    // Санамсаргүй өгөгдөл үүсгэх
    mt19937 rng(42);
    uniform_int_distribution<int> dist(0, 1'000'000);
    vector<int> arr(n);
    for (auto& x : arr) x = dist(rng);

    // Зөв эрэмбэлсэн хуулбар (баталгаажуулалт)
    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    auto start = high_resolution_clock::now();
    mergeSort(arr, 0, n - 1);
    auto end   = high_resolution_clock::now();

    double ms = duration<double, milli>(end - start).count();

    // Зөвшөөрөл шалгах
    bool correct = (arr == expected);

    cout << fixed << setprecision(3);
    cout << "  n = " << setw(9) << n
         << "  |  Хугацаа: " << setw(10) << ms << " мс"
         << "  |  " << (correct ? "✓ Зөв" : "✗ Алдаа") << "\n";
}

int main() {
    cout << "====================================================\n";
    cout << "  Merge Sort — Дараалсан (Sequential)\n";
    cout << "====================================================\n";
    runTest(10'000);
    runTest(100'000);
    runTest(1'000'000);
    cout << "====================================================\n";
    return 0;
}

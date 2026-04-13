// ============================================================
//  Merge Sort — 3. OpenMP (Compiler directive) хувилбар
//  Хэрэглээ: g++ -O2 -fopenmp -o omp 3_openmp.cpp && ./omp
// ============================================================
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <omp.h>
#include <iomanip>

using namespace std;
using namespace chrono;

// ----------------------------------------------------------
//  Нэгтгэх туслах функц
// ----------------------------------------------------------
void merge(vector<int>& arr, int left, int mid, int right) {
    vector<int> L(arr.begin() + left,  arr.begin() + mid + 1);
    vector<int> R(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0, j = 0, k = left;
    while (i < (int)L.size() && j < (int)R.size())
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < (int)L.size()) arr[k++] = L[i++];
    while (j < (int)R.size()) arr[k++] = R[j++];
}

// ----------------------------------------------------------
//  Дараалсан merge sort (task-ийн суурь болгон)
// ----------------------------------------------------------
void mergeSortSeq(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortSeq(arr, left, mid);
    mergeSortSeq(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// ----------------------------------------------------------
//  OpenMP task-д суурилсан параллел merge sort
//
//  #pragma omp task — рекурсив дуудлагыг task болгон
//                     thread pool-д хуваарилна
//  #pragma omp taskwait — хоёр task дуустлаа хүлээнэ
//  cutoff: жижиг хэсгүүдийг task-аар задлах шаардлагагүй,
//          дараалсан хувилбарт шилжүүлснээр overhead буурна
// ----------------------------------------------------------
const int OMP_CUTOFF = 1024;   // энэ утгаас бага бол seq ашиглана

void mergeSortOMP(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int size = right - left + 1;
    if (size <= OMP_CUTOFF) {
        mergeSortSeq(arr, left, right);
        return;
    }

    int mid = left + (right - left) / 2;

    #pragma omp task shared(arr) firstprivate(left, mid)
    mergeSortOMP(arr, left, mid);

    #pragma omp task shared(arr) firstprivate(mid, right)
    mergeSortOMP(arr, mid + 1, right);

    #pragma omp taskwait   // хоёулаа дуустлаа хүлээнэ

    merge(arr, left, mid, right);
}

// ----------------------------------------------------------
//  Туршилт
// ----------------------------------------------------------
void runTest(int n, int threads) {
    mt19937 rng(42);
    uniform_int_distribution<int> dist(0, 1'000'000);
    vector<int> arr(n);
    for (auto& x : arr) x = dist(rng);

    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    omp_set_num_threads(threads);

    auto start = high_resolution_clock::now();

    // parallel region нэг удаа нээж, дотор task үүсгэнэ
    #pragma omp parallel
    {
        #pragma omp single nowait
        mergeSortOMP(arr, 0, n - 1);
    }

    auto end = high_resolution_clock::now();

    double ms    = duration<double, milli>(end - start).count();
    bool correct = (arr == expected);

    cout << fixed << setprecision(3);
    cout << "  n = " << setw(9) << n
         << "  |  Хугацаа: " << setw(10) << ms << " мс"
         << "  |  Thread: " << threads
         << "  |  " << (correct ? "✓ Зөв" : "✗ Алдаа") << "\n";
}

int main() {
    int maxThreads = omp_get_max_threads();
    cout << "====================================================\n";
    cout << "  Merge Sort — OpenMP (compiler directive параллел)\n";
    cout << "  OpenMP max thread: " << maxThreads << "\n";
    cout << "====================================================\n";
    runTest(10'000,    maxThreads);
    runTest(100'000,   maxThreads);
    runTest(1'000'000, maxThreads);
    cout << "====================================================\n";
    return 0;
}

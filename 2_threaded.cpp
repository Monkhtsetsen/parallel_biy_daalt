// ============================================================
//  Merge Sort — 2. std::thread (CPU олон цөм) хувилбар
//  Хэрэглээ: g++ -O2 -std=c++17 -pthread -o thr 2_threaded.cpp && ./thr
// ============================================================
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>
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
//  Дараалсан merge sort (thread-ийн суурь рекурс болгон)
// ----------------------------------------------------------
void mergeSortSeq(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortSeq(arr, left, mid);
    mergeSortSeq(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// ----------------------------------------------------------
//  Параллел merge sort
//
//  depth < MAX_DEPTH үед шинэ thread үүсгэнэ.
//  MAX_DEPTH = 3  →  2^3 = 8 thread хүртэл ашиглана
//  (thread нэмэгдэх тусам overhead ихэснэ, тиймээс хязгаарладаг)
// ----------------------------------------------------------
const int MAX_DEPTH = 3;   // thread тоо = 2^MAX_DEPTH

void mergeSortParallel(vector<int>& arr, int left, int right, int depth = 0) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    if (depth < MAX_DEPTH) {
        // Хоёр хагасыг тусдаа thread-д өгнө
        thread t([&]() {
            mergeSortParallel(arr, left, mid, depth + 1);
        });
        // Гол thread баруун хагасыг авна
        mergeSortParallel(arr, mid + 1, right, depth + 1);
        t.join();   // зүүн thread дуустлаа хүлээнэ
    } else {
        // Гүн хангалттай болсон — дараалсан хувилбарт шилжинэ
        mergeSortSeq(arr, left, mid);
        mergeSortSeq(arr, mid + 1, right);
    }

    merge(arr, left, mid, right);
}

// ----------------------------------------------------------
//  Туршилт
// ----------------------------------------------------------
void runTest(int n) {
    mt19937 rng(42);
    uniform_int_distribution<int> dist(0, 1'000'000);
    vector<int> arr(n);
    for (auto& x : arr) x = dist(rng);

    vector<int> expected = arr;
    sort(expected.begin(), expected.end());

    auto start = high_resolution_clock::now();
    mergeSortParallel(arr, 0, n - 1, 0);
    auto end   = high_resolution_clock::now();

    double ms  = duration<double, milli>(end - start).count();
    bool correct = (arr == expected);

    unsigned int hw = thread::hardware_concurrency();
    cout << fixed << setprecision(3);
    cout << "  n = " << setw(9) << n
         << "  |  Хугацаа: " << setw(10) << ms << " мс"
         << "  |  Thread: " << hw
         << "  |  " << (correct ? "✓ Зөв" : "✗ Алдаа") << "\n";
}

int main() {
    cout << "====================================================\n";
    cout << "  Merge Sort — std::thread (CPU параллел)\n";
    cout << "  Ашигласан thread: " << thread::hardware_concurrency() << " цөм\n";
    cout << "====================================================\n";
    runTest(10'000);
    runTest(100'000);
    runTest(1'000'000);
    cout << "====================================================\n";
    return 0;
}

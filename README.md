# Merge Sort — 4 хувилбарын гүйцэтгэлийн харьцуулалт

## Файлын бүтэц

```
merge_sort/
├── 1_sequential.cpp   — Дараалсан хувилбар
├── 2_threaded.cpp     — std::thread хувилбар
├── 3_openmp.cpp       — OpenMP хувилбар
├── 4_cuda.cu          — CUDA (GPU) хувилбар
└── Makefile
```

---

## 1. Дараалсан (Sequential) — `1_sequential.cpp`

### Алгоритмын логик
Merge sort нь **хуваа, эзэмш (divide and conquer)** зарчимд суурилна.

```
mergeSort(arr, left, right):
    mid = (left + right) / 2
    mergeSort(arr, left, mid)       ← зүүн хагасыг рекурсээр эрэмбэлнэ
    mergeSort(arr, mid+1, right)    ← баруун хагасыг рекурсээр эрэмбэлнэ
    merge(arr, left, mid, right)    ← нэгтгэнэ
```

**Цогцолбор байдал:**
- Хугацаа: O(n log n) — хамгийн муу, дундаж, хамгийн сайн бүгд адил
- Санах ой: O(n) — merge-д туслах массив хэрэгтэй

---

## 2. std::thread — `2_threaded.cpp`

### Стратеги: Рекурсив thread үүсгэлт

Рекурсийн мод дотор тодорхой гүн (`depth`) хүртэл **шинэ thread** үүсгэж, массивын хоёр хагасыг зэрэгцээ эрэмбэлнэ.

```
depth = 0 → 2 thread (root)
depth = 1 → 4 thread
depth = 2 → 8 thread
depth ≥ 3 → дараалсан хувилбарт шилжинэ (overhead хэмнэнэ)
```

```cpp
thread t([&]() { mergeSortParallel(arr, left,   mid,   depth+1); });
               mergeSortParallel(arr, mid+1, right, depth+1);
t.join();
```

**Анхаарах зүйл:**
- Зүүн ба баруун хагас нь `arr`-ийн **өөр өөр хэсгийг** ашиглах тул race condition байхгүй
- Жижиг массивт thread үүсгэх overhead нь хурдасгалаас их байдаг тул cutoff чухал

---

## 3. OpenMP — `3_openmp.cpp`

### Стратеги: `#pragma omp task`

OpenMP-ийн **task** механизмаар рекурсив дуудлага бүрийг thread pool-д хуваарилна.

```cpp
#pragma omp parallel          // thread pool нээнэ
{
    #pragma omp single nowait // зөвхөн нэг thread task үүсгэж эхлэнэ
    mergeSortOMP(arr, 0, n-1);
}

// mergeSortOMP доторх:
#pragma omp task              // зүүн хагасыг task болгоно
    mergeSortOMP(arr, left, mid);

#pragma omp task              // баруун хагасыг task болгоно
    mergeSortOMP(arr, mid+1, right);

#pragma omp taskwait          // хоёулаа дуустлаа хүлээнэ
```

**Оновчлол — CUTOFF:**
```cpp
const int OMP_CUTOFF = 1024;
if (size <= OMP_CUTOFF) {
    mergeSortSeq(arr, left, right);  // жижиг блокийг task болгохгүй
    return;
}
```
Cutoff байхгүй бол хэтэрхий олон жижиг task үүсэж, scheduler-ийн overhead давамгайлна.

**std::thread vs OpenMP:**
| | std::thread | OpenMP |
|---|---|---|
| Thread удирдлага | Гараар | Автомат (runtime) |
| Cutoff | depth-ээр | Хэмжээгээр |
| Уян хатан байдал | Өндөр | Дунд |
| Хялбар байдал | Бага | Өндөр |

---

## 4. CUDA — `4_cuda.cu`

### Стратеги: Bottom-up iterative merge sort on GPU

Рекурс ашигладаггүй. Оронд нь **давталтаар** блокийн хэмжээг 2 дахин нэмэгдүүлнэ.

```
Давталт 1 (width=1): [1|1] [1|1] [1|1] ... → бүх хосыг нэгтгэнэ → [2] [2] [2]
Давталт 2 (width=2): [2|2] [2|2] ...       → [4] [4] [4]
Давталт 3 (width=4): [4|4] [4|4] ...       → [8] [8]
...
Давталт k (width=n/2): [n/2|n/2]           → [n]  ← дууслаа
```

**Kernel бүтэц:**
```
нэг GPU thread = нэг merge операци
thread tid → left = tid * 2 * width
           → mid  = left + width - 1
           → right = left + 2*width - 1
```

```cuda
__global__ void mergeSortKernel(int* arr, int* tmp, int n, int width) {
    int tid   = blockIdx.x * blockDim.x + threadIdx.x;
    int left  = tid * 2 * width;
    ...
    // arr[left..mid] ба arr[mid+1..right] нэгтгэж tmp руу бичнэ
    // tmp → arr руу хуулна
}
```

**CUDA санах ойн загвар:**
```
Host (CPU RAM)                Device (GPU VRAM)
h_arr[]    ──cudaMemcpy──►   d_arr[]    ← kernel өөрчилнө
                              d_tmp[]    ← завсрын санах ой

cudaMemcpy (DeviceToHost) ◄── d_arr[]   (эрэмбэлсэн)
```

**Хугацаа хэмжих — CUDA Event:**
```cpp
cudaEventRecord(start);
mergeSortCUDA(d_arr, d_tmp, n);    // kernel + sync
cudaEventRecord(stop);
cudaEventElapsedTime(&ms, start, stop);  // GPU-ийн цаг
```

---

## Compile & Run

```bash
# Бүгдийг нэг дороос
make all
make run

# Тус тусаар
make seq   &&  ./seq
make thr   &&  ./thr
make omp   &&  ./omp
make cuda  &&  ./cuda

# Цэвэрлэх
make clean
```

---

## Гүйцэтгэлийн хүлээгдэж буй үр дүн (жишиг)

| Хувилбар | 10k | 100k | 1M |
|---|---|---|---|
| Sequential | ~1 мс | ~15 мс | ~180 мс |
| std::thread (8 thread) | ~1 мс | ~8 мс | ~80 мс |
| OpenMP (8 thread) | ~1 мс | ~7 мс | ~60 мс |
| CUDA (RTX 3060) | ~0.5 мс | ~2 мс | ~15 мс |

> ⚠️ Жижиг n (10k) дээр параллел хувилбарууд удаан байж болно — thread/kernel
> үүсгэх overhead нь параллелизмын ашгаас их байдаг.

---

## Гол ойлголтууд

| Ойлголт | Тайлбар |
|---|---|
| Thread overhead | Thread үүсгэх нь ~мкс зарцуулдаг — жижиг ажилд ашиггүй |
| Cutoff | Тодорхой хэмжээнээс доош sequential ашиглах нь оновчтой |
| Memory bandwidth | Merge sort нь санах ой их ашигладаг — GPU-ийн давуу тал bagасна |
| `taskwait` vs `join` | Утга нэг: хоёр task/thread дуустлаа хүлээнэ |
| Bottom-up CUDA | Рекурс kernel дуудалт GPU-д хүндрэлтэй тул iterative арга тохиромжтой |
# parallel_biy_daalt

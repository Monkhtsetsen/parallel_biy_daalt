#ifndef BENCHMARK_UTILS_HPP
#define BENCHMARK_UTILS_HPP
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
inline std::vector<int> makeRandomData(int n) {
    // Deterministic lightweight generator: same input for every algorithm.
    unsigned int x = 42u;
    std::vector<int> a(n);
    for (int &v : a) {
        x = 1664525u * x + 1013904223u;
        v = static_cast<int>(x % 1'000'001u);
    }
    return a;
}

inline long long estimatedOperations(int n) {
    if (n <= 1) return 0;
    return static_cast<long long>(std::llround(static_cast<double>(n) * std::log2(static_cast<double>(n))));
}

inline double achievableMOPS(int n, double totalMs) {
    if (totalMs <= 0.0) return 0.0;
    return (static_cast<double>(estimatedOperations(n)) / (totalMs / 1000.0)) / 1'000'000.0;
}

inline void ensureCsvHeader(const std::string &filename) {
    bool needHeader = true;
    FILE* check = std::fopen(filename.c_str(), "rb");
    if (check) {
        int c = std::fgetc(check);
        needHeader = (c == EOF);
        std::fclose(check);
    }

    if (needHeader) {
        FILE* out = std::fopen(filename.c_str(), "a");
        if (!out) {
            std::perror("Cannot open CSV file");
            return;
        }
        std::fprintf(out, "Algorithm,N,ExecutionTimeMs,H2DMs,KernelMs,D2HMs,TotalTimeMs,TransferredBytes,TotalOperations,AchievableMOPS,Correct,Workers,Notes\n");
        std::fclose(out);
    }
}

inline void appendResultCsv(
    const std::string &filename,
    const std::string &algorithm,
    int n,
    double executionMs,
    double h2dMs,
    double kernelMs,
    double d2hMs,
    double totalMs,
    long long transferredBytes,
    bool correct,
    const std::string &workers,
    const std::string &notes
) {
    ensureCsvHeader(filename);
    FILE* out = std::fopen(filename.c_str(), "a");
    if (!out) {
        std::perror("Cannot open CSV file");
        return;
    }
    std::fprintf(out, "%s,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%lld,%lld,%.6f,%s,%s,%s\n",
                 algorithm.c_str(), n, executionMs, h2dMs, kernelMs, d2hMs, totalMs,
                 transferredBytes, estimatedOperations(n), achievableMOPS(n, totalMs),
                 correct ? "YES" : "NO", workers.c_str(), notes.c_str());
    std::fclose(out);
}

inline void printResult(
    const std::string &algorithm,
    int n,
    double totalMs,
    bool correct,
    const std::string &extra = ""
) {
    std::printf("  %-12s | n = %9d | Total: %10.3f ms | %s",
                algorithm.c_str(), n, totalMs, correct ? "OK" : "ERROR");
    if (!extra.empty()) std::printf(" | %s", extra.c_str());
    std::printf("\n");
}

#endif

#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cstdint> // Added to safeguard uint64_t usage

struct RawTransaction {
    uint64_t src;
    uint64_t tgt;
    double val;
};

double traditional_query_throughput = 0.0;
double traditional_average_latency_ns = 0.0;

void run_traditional_benchmark(const std::vector<RawTransaction>& workload) {
    size_t total_records = workload.size();
    if (total_records == 0) return;
    
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, double>> legacy_matrix;
    legacy_matrix.reserve(8192);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < total_records; ++i) {
        legacy_matrix[workload[i].src][workload[i].tgt] += workload[i].val;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    if (total_ns > 0) {
        traditional_query_throughput = (static_cast<double>(total_records) / (total_ns / 1000000000.0)) / 1000000.0;
    }
    traditional_average_latency_ns = static_cast<double>(total_ns) / total_records;
}

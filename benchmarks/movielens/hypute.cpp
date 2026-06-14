#include <iostream>
#include <vector>
#include <chrono>
#include "hypute_stream.h"

struct RawTransaction {
    uint64_t src;
    uint64_t tgt;
    double val;
};

double hypute_query_throughput = 0.0;
double hypute_average_latency_ns = 0.0;

void run_hypute_benchmark(const std::vector<RawTransaction>& workload, const char* context) {
    size_t total_records = workload.size();
    if (total_records == 0) return;

    hypute_initialize(context);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < total_records; ++i) {
        hypute_process(workload[i].src, workload[i].tgt, workload[i].val);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    if (total_ns > 0) {
        hypute_query_throughput = (static_cast<double>(total_records) / (total_ns / 1000000000.0)) / 1000000.0;
    }
    hypute_average_latency_ns = static_cast<double>(total_ns) / total_records;

    hypute_shutdown();
}

#include <vector>
#include <chrono>
#include <cstdint>
#include "hypute_stream.h"

struct RawTransaction {
    uint64_t src;
    uint64_t tgt;
    double val;
};

double hypute_query_throughput = 0.0;
double hypute_average_latency_ns = 0.0;
std::size_t hypute_records_processed = 0;

static bool parse_transaction(const std::string& line, RawTransaction& transaction) {
    if (line.empty()) {
        return false;
    }

    if (line.find("::") != std::string::npos) {
        size_t p1 = line.find("::");
        size_t p2 = line.find("::", p1 + 2);
        size_t p3 = line.find("::", p2 + 2);

        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos) {
            return false;
        }

        transaction.src = std::stoull(line.substr(0, p1));
        transaction.tgt = std::stoull(line.substr(p1 + 2, p2 - (p1 + 2)));
        transaction.val = std::stod(line.substr(p2 + 2, p3 - (p2 + 2)));
        return true;
    }

    if (line.find(',') != std::string::npos) {
        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        size_t p3 = line.find(',', p2 + 1);

        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos) {
            return false;
        }

        try {
            transaction.src = std::stoull(line.substr(0, p1));
            transaction.tgt = std::stoull(line.substr(p1 + 1, p2 - (p1 + 1)));
            transaction.val = std::stod(line.substr(p2 + 1, p3 - (p2 + 1)));
            return true;
        } catch (...) {
            return false;
        }
    }

    return false;
}

bool run_hypute_benchmark(const std::vector<RawTransaction>& workload, const char* context) {
    hypute_records_processed = 0;

    hypute_initialize(context);

    auto start = std::chrono::high_resolution_clock::now();

    for (const RawTransaction& transaction : workload) {
        hypute_process(transaction.src, transaction.tgt, transaction.val);
        ++hypute_records_processed;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    if (hypute_records_processed == 0) {
        std::cerr << "[ERROR] Dataset processing batch yielded zero records.\n";
        hypute_shutdown();
        return false;
    }
    if (total_ns > 0) {
        hypute_query_throughput = (static_cast<double>(hypute_records_processed) / (total_ns / 1000000000.0)) / 1000000.0;
    }
    hypute_average_latency_ns = static_cast<double>(total_ns) / hypute_records_processed;

    hypute_shutdown();
    return true;
}

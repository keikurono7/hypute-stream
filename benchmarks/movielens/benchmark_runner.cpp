#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <algorithm> 
#include <random>    
#include <sstream>

struct RawTransaction {
    uint64_t src;
    uint64_t tgt;
    double val;
};

extern void run_traditional_benchmark(const std::vector<RawTransaction>& workload);
extern void run_hypute_benchmark(const std::vector<RawTransaction>& workload, const char* context);

extern double traditional_query_throughput;
extern double traditional_average_latency_ns;
extern double hypute_query_throughput;
extern double hypute_average_latency_ns;

// Programmatic Linux kernel telemetry retrieval hook
size_t get_current_rss_kb() {
    std::ifstream stream("/proc/self/status");
    std::string line;
    while (std::getline(stream, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            std::stringstream ss(line);
            std::string label;
            size_t value_kb = 0;
            ss >> label >> value_kb;
            return value_kb;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "[ERROR] Missing target execution mode.\n";
        std::cerr << "Usage: " << argv[0] << " <traditional|hypute> [context]\n";
        return 1;
    }

    std::string mode          = argv[1];
    const char* ctx           = (argc > 2) ? argv[2] : "DEFAULT_CTX";
    const char* dataset_arg   = (argc > 3) ? argv[3] : nullptr;

    std::string dataset;
    std::ifstream file;

    auto try_open = [&](const std::string& p) -> bool {
        file.open(p); if (file.is_open()) { dataset = p; return true; }
        return false;
    };

    if (dataset_arg) {
        try_open(dataset_arg);
    } else {
        try_open("ml-25m/ratings.csv") ||
        try_open("../benchmarks/movielens/ml-25m/ratings.csv") ||
        try_open("../../benchmarks/movielens/ml-25m/ratings.csv");
    }

    if (!file.is_open()) {
        std::cerr << "[ERROR] Dataset file not found or unreadable.\n";
        return 1;
    }

    // Derive label from dataset path
    std::string bench_label = "MOVIELENS 25M";
    if (dataset.find("amazon") != std::string::npos || dataset.find("Amazon") != std::string::npos)
        bench_label = "AMAZON REVIEWS'23 100M";
    else if (dataset.find("32m") != std::string::npos)
        bench_label = "MOVIELENS 32M+";

    std::cout << "[HAYAKO LOG] Ingesting evaluation records from: " << dataset << "...\n";
    std::vector<RawTransaction> workload_buffer;
    workload_buffer.reserve(dataset_arg ? 100000000 : 25000000);

    std::string line;
    if (std::getline(file, line)) { /* Skip CSV header row */ }

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        size_t p3 = line.find(',', p2 + 1);

        if (p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos) {
            RawTransaction tx;
            tx.src = std::stoull(line.substr(0, p1));
            tx.tgt = std::stoull(line.substr(p1 + 1, p2 - (p1 + 1)));
            tx.val = std::stod(line.substr(p2 + 1, p3 - (p2 + 1)));
            workload_buffer.push_back(tx);
        }
    }
    file.close();

    size_t record_count = workload_buffer.size();
    if (record_count == 0) {
        std::cerr << "[ERROR] Dataset processing stream yielded zero records.\n";
        return 1;
    }

    // Capture baseline memory immediately after data loading, but before engines execute allocations
    size_t rss_baseline_kb = get_current_rss_kb();

    std::cout << "\n========================================================\n";
    std::cout << "        " << bench_label << " STREAMING STATE MUTATION BENCHMARK \n";
    std::cout << "========================================================\n";
    std::cout << "[STATUS] Total Records Processed : " << record_count << " Interaction Rows\n";
    std::cout << "[STATUS] Isolated Profile Target : " << mode << "\n";
    std::cout << "========================================================\n\n";

    if (mode == "traditional") {
        std::cout << "[HAYAKO LOG] Running Traditional Phase 1 (Preserved Sorted)...\n";
        run_traditional_benchmark(workload_buffer);
        std::cout << "  Sorted Latency    : " << std::fixed << std::setprecision(2) << traditional_average_latency_ns << " ns\n";
        std::cout << "  Sorted Throughput : " << traditional_query_throughput << " M qps\n\n";

        std::cout << "[HAYAKO LOG] Randomizing stream event sequencing in-place...\n";
        std::mt19937 g(42); 
        std::shuffle(workload_buffer.begin(), workload_buffer.end(), g);

        std::cout << "[HAYAKO LOG] Running Traditional Phase 2 (High Entropy Randomized)...\n";
        run_traditional_benchmark(workload_buffer);
        std::cout << "  Random Latency    : " << std::fixed << std::setprecision(2) << traditional_average_latency_ns << " ns\n";
        std::cout << "  Random Throughput : " << traditional_query_throughput << " M qps\n";

    } else if (mode == "hypute") {
        std::cout << "[HAYAKO LOG] Running Hypute Phase 1 (Preserved Sorted)...\n";
        run_hypute_benchmark(workload_buffer, ctx);
        std::cout << "  Sorted Latency    : " << std::fixed << std::setprecision(2) << hypute_average_latency_ns << " ns\n";
        std::cout << "  Sorted Throughput : " << hypute_query_throughput << " M qps\n\n";

        std::cout << "[HAYAKO LOG] Randomizing stream event sequencing in-place...\n";
        std::mt19937 g(42); 
        std::shuffle(workload_buffer.begin(), workload_buffer.end(), g);

        std::cout << "[HAYAKO LOG] Running Hypute Phase 2 (High Entropy Randomized)...\n";
        run_hypute_benchmark(workload_buffer, ctx);
        std::cout << "  Random Latency    : " << std::fixed << std::setprecision(2) << hypute_average_latency_ns << " ns\n";
        std::cout << "  Random Throughput : " << hypute_query_throughput << " M qps\n";

    } else {
        std::cerr << "[ERROR] Invalid execution target choice: " << mode << "\n";
        return 1;
    }

    // Capture final allocation state before process teardown
    size_t rss_final_kb = get_current_rss_kb();
    long long rss_delta_kb = static_cast<long long>(rss_final_kb) - static_cast<long long>(rss_baseline_kb);

    // FIXED: Renamed fields to reflect accurate, honest system-level metrics
    std::cout << "\n========================================================\n";
    std::cout << " INTERNAL PROCESS TELEMETRY LOGS (RETAINED STATE)\n";
    std::cout << "========================================================\n";
    std::cout << "  Dataset RSS Baseline         : " << std::fixed << std::setprecision(2) << (rss_baseline_kb / 1024.0) << " MB\n";
    std::cout << "  Final Process VmRSS          : " << std::fixed << std::setprecision(2) << (rss_final_kb / 1024.0) << " MB\n";
    std::cout << "  Delta RSS (Memory Retained)  : " << std::fixed << std::setprecision(2) << (rss_delta_kb / 1024.0) << " MB\n";
    std::cout << "========================================================\n";
    
    return 0;
}

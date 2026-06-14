#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdint> // Added to fix the uint64_t compilation error

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

int main(int argc, char* argv[]) {
    const char* ctx = (argc > 1) ? argv[1] : "DEFAULT_CTX";
    
    std::string dataset = "ml-1m/ratings.dat";
    std::ifstream file(dataset);
    
    if (!file.is_open()) {
        dataset = "../benchmarks/movielens/ml-1m/ratings.dat";
        file.open(dataset);
    }
    if (!file.is_open()) {
        dataset = "../../benchmarks/movielens/ml-1m/ratings.dat";
        file.open(dataset);
    }

    if (!file.is_open()) {
        std::cerr << "[ERROR] MovieLens dataset asset files not found.\n";
        std::cerr << "Verify execution steps: benchmarks/movielens/dataset_download.sh\n";
        return 1;
    }

    std::cout << "[HAYAKO LOG] Ingesting evaluation records from: " << dataset << "...\n";

    std::vector<RawTransaction> workload_buffer;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        size_t p1 = line.find("::");
        size_t p2 = line.find("::", p1 + 2);
        size_t p3 = line.find("::", p2 + 2);

        if (p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos) {
            RawTransaction tx;
            tx.src = std::stoull(line.substr(0, p1));
            tx.tgt = std::stoull(line.substr(p1 + 2, p2 - (p1 + 2)));
            tx.val = std::stod(line.substr(p2 + 2, p3 - (p2 + 2)));
            workload_buffer.push_back(tx);
        }
    }
    file.close();

    size_t record_count = workload_buffer.size();
    if (record_count == 0) {
        std::cerr << "[ERROR] Dataset processing stream yielded zero records.\n";
        return 1;
    }

    std::cout << "[HAYAKO LOG] Ingest complete. Running symmetric mutation loops...\n";
    
    run_traditional_benchmark(workload_buffer);
    run_hypute_benchmark(workload_buffer, ctx);

    double throughput_gain = 0.0;
    if (traditional_query_throughput > 0.0) {
        throughput_gain = hypute_query_throughput / traditional_query_throughput;
    }

    double latency_reduction = 0.0;
    if (traditional_average_latency_ns > 0.0) {
        latency_reduction = (1.0 - (hypute_average_latency_ns / traditional_average_latency_ns)) * 100.0;
    }

    std::cout << "\n========================================================\n";
    std::cout << "        MOVIELENS 1M STREAMING STATE MUTATION BENCHMARK \n";
    std::cout << "========================================================\n\n";
    
    std::cout << "[STATUS] Total Records Processed : " << record_count << " Interaction Rows\n\n";
    
    std::cout << "Traditional Baseline:\n";
    std::cout << "  Latency:    " << std::fixed << std::setprecision(2) << traditional_average_latency_ns << " ns\n";
    std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << traditional_query_throughput << " M qps\n\n";

    std::cout << "Hypute Stream Runtime:\n";
    std::cout << "  Latency:    " << std::fixed << std::setprecision(2) << hypute_average_latency_ns << " ns\n";
    std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << hypute_query_throughput << " M qps\n\n";

    std::cout << "Comparative Metrics:\n";
    std::cout << "  Observed Throughput Ratio   : " << std::fixed << std::setprecision(2) << throughput_gain << "\n";
    std::cout << "  Observed Latency Difference : " << std::fixed << std::setprecision(2) << latency_reduction << "%\n";
    std::cout << "========================================================\n";

    return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <algorithm> 
#include <random>    

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

void print_output_grid(const std::string& mode_label, 
                       double trad_lat, double trad_tp, 
                       double hyp_lat, double hyp_tp) {
    double tp_ratio = trad_tp > 0.0 ? (hyp_tp / trad_tp) : 0.0;
    double lat_diff = trad_lat > 0.0 ? (((trad_lat - hyp_lat) / trad_lat) * 100.0) : 0.0;

    std::cout << "========================================================\n";
    std::cout << " DATASET EXECUTION MODE: " << mode_label << "\n";
    std::cout << "========================================================\n";
    std::cout << "Traditional Baseline:\n";
    std::cout << "  Latency:    " << std::fixed << std::setprecision(2) << trad_lat << " ns\n";
    std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << trad_tp << " M qps\n\n";
    std::cout << "Hypute Stream Runtime:\n";
    std::cout << "  Latency:    " << std::fixed << std::setprecision(2) << hyp_lat << " ns\n";
    std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << hyp_tp << " M qps\n\n";
    std::cout << "Comparative Metrics:\n";
    std::cout << "  Observed Throughput Ratio   : " << std::fixed << std::setprecision(2) << tp_ratio << "\n";
    std::cout << "  Observed Latency Difference : " << std::fixed << std::setprecision(2) << lat_diff << "%\n";
    std::cout << "========================================================\n\n";
}

int main(int argc, char* argv[]) {
    const char* ctx = (argc > 1) ? argv[1] : "DEFAULT_CTX";
    std::string dataset = "ml-25m/ratings.csv";
    std::ifstream file(dataset);
    
    if (!file.is_open()) { dataset = "../benchmarks/movielens/ml-25m/ratings.csv"; file.open(dataset); }
    if (!file.is_open()) { dataset = "../../benchmarks/movielens/ml-25m/ratings.csv"; file.open(dataset); }

    if (!file.is_open()) {
        std::cerr << "[ERROR] MovieLens 25M source file target unreadable.\n";
        return 1;
    }

    std::cout << "[HAYAKO LOG] Ingesting evaluation records from: " << dataset << "...\n";
    std::vector<RawTransaction> workload_buffer;
    workload_buffer.reserve(25000000);

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

    // Pass 1: Preserved Sorted Track Cycles
    std::cout << "[HAYAKO LOG] Running Phase 1: Preserved Sorted Track Cycles...\n";
    run_traditional_benchmark(workload_buffer);
    double trad_sorted_lat = traditional_average_latency_ns;
    double trad_sorted_tp  = traditional_query_throughput;

    run_hypute_benchmark(workload_buffer, ctx);
    double hyp_sorted_lat = hypute_average_latency_ns;
    double hyp_sorted_tp  = hypute_query_throughput;

    // Fixed: Shuffled completely in-place to avoid duplication and eliminate memory overhead bounds
    std::cout << "[HAYAKO LOG] Randomizing stream event sequencing to mimic real-time production traffic...\n";
    std::mt19937 g(42); 
    // Fixed: Typo resolved using standard distribution syntax
    std::uniform_int_distribution<size_t> dist(0, record_count - 1);
    std::shuffle(workload_buffer.begin(), workload_buffer.end(), g);

    // Pass 2: High Entropy Randomized Track Cycles
    std::cout << "[HAYAKO LOG] Running Phase 2: Randomized Track Cycles...\n";
    run_traditional_benchmark(workload_buffer);
    double trad_rand_lat = traditional_average_latency_ns;
    double trad_rand_tp  = traditional_query_throughput;

    run_hypute_benchmark(workload_buffer, ctx);
    double hyp_rand_lat = hypute_average_latency_ns;
    double hyp_rand_tp  = hypute_query_throughput;

    std::cout << "\n========================================================\n";
    std::cout << "        MOVIELENS 25M STREAMING STATE MUTATION BENCHMARK \n";
    std::cout << "========================================================\n\n";
    std::cout << "[STATUS] Total Records Processed : " << record_count << " Interaction Rows\n\n";

    print_output_grid("PRESERVED SORTED DATA", trad_sorted_lat, trad_sorted_tp, hyp_sorted_lat, hyp_sorted_tp);
    print_output_grid("HIGH ENTROPY RANDOMIZED", trad_rand_lat, trad_rand_tp, hyp_rand_lat, hyp_rand_tp);

    return 0;
}

#include <iostream>
#include <chrono>
#include <iomanip>
#include "hypute_stream.h"

constexpr int EVALUATION_CYCLES = 10000000;

int main(int argc, char* argv[]) {
    const char* ctx = (argc > 1) ? argv[1] : "DEFAULT_CTX";

    hypute_initialize(ctx);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < EVALUATION_CYCLES; ++i) {
        hypute_process(1001, 2002, 5.0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    double average_ns = static_cast<double>(elapsed_ns) / EVALUATION_CYCLES;
    double throughput_mops = (static_cast<double>(EVALUATION_CYCLES) / (elapsed_ns / 1000000000.0)) / 1000000.0;

    std::cout << "========================================================\n";
    std::cout << "                HYPUTE EVALUATION RESULTS               \n";
    std::cout << "========================================================\n\n";
    std::cout << "[STATUS] Execution Cycles     : " << EVALUATION_CYCLES << "\n";
    std::cout << "[STATUS] Execution Throughput : " << std::fixed << std::setprecision(2) << throughput_mops << " M executions/sec\n";
    std::cout << "[STATUS] Average Latency      : " << std::fixed << std::setprecision(2) << average_ns << " ns\n\n";
    std::cout << "========================================================\n";

    hypute_shutdown();
    return 0;
}

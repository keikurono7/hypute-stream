#include <iostream>
#include <chrono>
#include <iomanip>
#include "hypute_stream.h"

constexpr int EVALUATION_CYCLES = 10000000;

int main(int argc, char* argv[]) {

const char* ctx =
    (argc > 1)
        ? argv[1]
        : "DEFAULT_CTX";

hypute_initialize(ctx);

uint64_t b0 = 0x1000ULL;
uint64_t b1 = 0x20000ULL;
uint64_t b2 = 0x8ULL;

constexpr int ACTIVE_INPUTS = 3;

auto start =
    std::chrono::high_resolution_clock::now();

for (int i = 0; i < EVALUATION_CYCLES; ++i) {
    hypute_execute(b0, b1, b2);
}

auto end =
    std::chrono::high_resolution_clock::now();

auto elapsed_ns =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - start
    ).count();

double elapsed_ms =
    elapsed_ns / 1000000.0;

double average_ns =
    static_cast<double>(elapsed_ns) /
    EVALUATION_CYCLES;

double throughput_mops =
    (
        static_cast<double>(EVALUATION_CYCLES)
        /
        (elapsed_ns / 1000000000.0)
    )
    /
    1000000.0;

std::cout
    << "========================================================\n";
std::cout
    << "                HYPUTE EVALUATION RESULTS               \n";
std::cout
    << "========================================================\n\n";

std::cout
    << "[STATUS] Evaluation Cycles      : "
    << EVALUATION_CYCLES
    << "\n";

std::cout
    << "[STATUS] Active Inputs          : "
    << ACTIVE_INPUTS
    << "\n";

std::cout
    << "[STATUS] Total Runtime          : "
    << std::fixed
    << std::setprecision(2)
    << elapsed_ms
    << " ms\n";

std::cout
    << "[STATUS] Average Execution Time : "
    << std::fixed
    << std::setprecision(2)
    << average_ns
    << " ns\n";

std::cout
    << "[STATUS] Throughput            : "
    << std::fixed
    << std::setprecision(2)
    << throughput_mops
    << " M executions/sec\n\n";

std::cout
    << "========================================================\n";

hypute_shutdown();

return 0;

}

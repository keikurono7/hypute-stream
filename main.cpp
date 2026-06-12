#include <iostream>
#include <chrono>
#include <iomanip>
#include "hypute_stream.h"

constexpr int EVALUATION_CYCLES = 10000000;

int main(int argc, char* argv[]) {
const char* ctx = (argc > 1) ? argv[1] : "DEFAULT_CTX";

```
std::cout << "========================================================\n";
std::cout << "               HYPUTE EVALUATION ENVIRONMENT            \n";
std::cout << "========================================================\n";

hypute_initialize();

uint64_t b0 = 0x1000ULL;
uint64_t b1 = 0x20000ULL;
uint64_t b2 = 0x8ULL;

auto start = std::chrono::high_resolution_clock::now();

for (int i = 0; i < EVALUATION_CYCLES; ++i) {
    hypute_execute(b0, b1, b2, ctx);
}

auto end = std::chrono::high_resolution_clock::now();

auto elapsed =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - start
    ).count();

double average_metric =
    static_cast<double>(elapsed) / EVALUATION_CYCLES;

std::cout << "------------------- EVALUATION RESULTS -----------------\n";
std::cout << "[STATUS] Total Processing Time : "
          << elapsed / 1000000.0
          << " ms\n";

std::cout << "[STATUS] Average Execution Time : "
          << std::fixed
          << std::setprecision(4)
          << average_metric
          << " ns\n";

std::cout << "========================================================\n";

hypute_shutdown();

return 0;
```

}

#ifndef HYPUTE_STREAM_H
#define HYPUTE_STREAM_H

#include <cstdint>

extern "C" {
void hypute_initialize();
void hypute_execute(
uint64_t block_0,
uint64_t block_1,
uint64_t block_2,
const char* runtime_context
);
void hypute_shutdown();
}

#endif // HYPUTE_STREAM_H

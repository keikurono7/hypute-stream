#ifndef HYPUTE_STREAM_H
#define HYPUTE_STREAM_H

#include <cstdint>

extern "C" {
    /**
     * @brief Instantiates the execution environment layer.
     */
    void hypute_initialize(const char* runtime_context);

    /**
     * @brief Ingests raw interaction metrics. Processing and internal 
     * structural encoding occur entirely within the runtime binary target.
     */
    void hypute_process(uint64_t source_id, uint64_t target_id, double metric_value);

    /**
     * @brief Dissolves runtime allocations and releases target threads.
     */
    void hypute_shutdown();
}

#endif // HYPUTE_STREAM_H
cmake_minimum_required(VERSION 3.12)
project(HyputeStreamPreview CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -pthread -funroll-loops -ffast-math")

link_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 1. Quick Validation Target
add_executable(hypute_eval evaluation/main.cpp)
target_link_libraries(hypute_eval hypute_demo pthread)

# 2. MovieLens B2B Comparison Module Target
add_executable(benchmark_runner 
    benchmarks/movielens/traditional.cpp 
    benchmarks/movielens/hypute.cpp 
    benchmarks/movielens/benchmark_runner.cpp
)
target_link_libraries(benchmark_runner hypute_demo pthread)
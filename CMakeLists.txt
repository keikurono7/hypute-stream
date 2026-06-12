cmake_minimum_required(VERSION 3.10)

project(HyputeStreamPreview CXX)

set(CMAKE_CXX_STANDARD 17)

set(
CMAKE_CXX_FLAGS
"${CMAKE_CXX_FLAGS} -O3 -march=native -pthread -funroll-loops -ffast-math"
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
hypute_stream_bench
main.cpp
)

target_link_libraries(
hypute_stream_bench
hypute_demo
pthread
)

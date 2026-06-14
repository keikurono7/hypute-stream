# Hypute Stream Evaluation Suite

Hypute Stream is a runtime platform developed by Hayako for latency-sensitive and performance-critical environments where execution predictability, bounded resource consumption, and operational consistency are important.

This repository provides a public evaluation environment allowing engineering teams and infrastructure architects to:

* Validate deployment compatibility
* Evaluate runtime behavior
* Benchmark execution characteristics
* Compare runtime behavior against conventional data-structure approaches using publicly available datasets

This repository contains a limited evaluation environment and does not include components distributed as part of commercial deployments.

---

## Evaluation Profiles

### 1. Verification Runtime Sandbox

A lightweight execution harness used to validate environment compatibility and establish baseline execution characteristics.

The sandbox reports:

* Execution throughput
* Average execution latency
* Runtime stability across repeated cycles

### 2. Streaming State Mutation Benchmark

A comparative benchmark built on the GroupLens MovieLens 25M dataset.

The benchmark evaluates two execution models:

* Traditional nested associative-map processing
* Hypute Stream runtime processing

Two workload modes are executed.

#### Preserved Sorted Data

Events are processed using the original dataset ordering.

This workload benefits strongly from locality and cache-friendly access patterns.

#### High-Entropy Randomized Data

Events are randomized before execution to simulate asynchronous production traffic.

This workload reduces locality and increases memory-access entropy, creating a more demanding mutation environment.

---

## Prerequisites

Install:

* C++17 compatible compiler (GCC or Clang)
* CMake
* curl
* unzip

---

## Download Dataset

```bash
cd benchmarks/movielens
chmod +x dataset_download.sh
./dataset_download.sh
cd ../..
```

---

## Build

```bash
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

---

## Run Benchmark

Traditional baseline:

```bash
./benchmark_runner traditional
```

Hypute runtime:

```bash
./benchmark_runner hypute
```

---

## Reference Results

The following measurements were obtained using the public evaluation environment and are provided as reference values only.

Results will vary depending on processor architecture, memory configuration, compiler version, operating system scheduling behavior, and workload characteristics.

### Preserved Sorted Dataset

| Metric     | Traditional | Hypute Stream |
| ---------- | ----------- | ------------- |
| Throughput | 13.62 M qps | 5.58 M qps    |
| Latency    | 73.40 ns    | 179.25 ns     |

### High-Entropy Randomized Dataset

| Metric     | Traditional | Hypute Stream |
| ---------- | ----------- | ------------- |
| Throughput | 1.52 M qps  | 5.53 M qps    |
| Latency    | 659.07 ns   | 180.90 ns     |

### Memory Characteristics

| Metric                     | Traditional | Hypute Stream |
| -------------------------- | ----------- | ------------- |
| Dataset RSS Baseline       | 575.98 MB   | 575.95 MB     |
| Final Retained RSS         | 1634.78 MB  | 576.25 MB     |
| Retained Allocation Delta  | 1058.80 MB  | 0.30 MB       |
| Peak Resident Memory (RSS) | 1637 MB     | 1148 MB       |

---

## Interpretation

These measurements demonstrate two distinct operational behaviors.

### Ordered Access Patterns

When interaction records remain highly ordered and cache-local, the traditional associative-map implementation achieves higher mutation throughput and lower latency.

### High-Entropy Access Patterns

When interaction ordering is randomized and locality decreases:

* The traditional associative-map implementation exhibits substantial throughput degradation and latency growth.
* Hypute Stream maintains comparatively stable throughput and latency characteristics.
* Peak memory consumption during execution was lower for Hypute Stream in the evaluated workload.

These observations are workload-specific and should be independently validated within the target deployment environment.

---

## Reproducibility

All benchmark workloads are derived from publicly available datasets and can be executed locally using the commands documented above.

Organizations evaluating the platform are encouraged to:

* Run benchmarks on their own hardware
* Compare compiler configurations
* Evaluate different processor architectures
* Validate behavior using their own representative workloads

---

## Enterprise Evaluation

This repository is intended for compatibility assessment, performance exploration, and deployment validation.

Commercial deployments may differ from the public evaluation environment.

For enterprise evaluations, pilot programs, or commercial discussions, please contact the Hayako team.

© 2026 Hayako. All rights reserved.

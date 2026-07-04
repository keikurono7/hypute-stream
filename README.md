# Hypute Stream — Public Evaluation Suite

Hypute Stream is a runtime platform by [Hayako](https://hayako.io) built for latency-sensitive environments where execution predictability, bounded memory consumption, and throughput stability under entropy matter.

This repository provides a public benchmark environment so engineering teams can validate deployment compatibility and compare runtime behavior against conventional approaches using real-world public datasets.

[![Hypute MovieLens Benchmark](https://github.com/hayako-org/hypute-stream/actions/workflows/movielens.yml/badge.svg)](https://github.com/hayako-org/hypute-stream/actions/workflows/movielens.yml)
[![Hypute Amazon Reviews 100M Benchmark](https://github.com/hayako-org/hypute-stream/actions/workflows/amazon-100m.yml/badge.svg)](https://github.com/hayako-org/hypute-stream/actions/workflows/amazon-100m.yml)

---

## Benchmark Results

Two datasets were used to evaluate behavior at different scales and entropy levels.

### MovieLens 25M — Streaming State Mutation

> GroupLens MovieLens 25M · 25,000,095 interaction rows

| Workload | Engine | Latency | Throughput |
|---|---|---|---|
| Preserved Sorted | Traditional | 73.40 ns | 13.62 M qps |
| Preserved Sorted | **Hypute Stream** | 179.25 ns | 5.58 M qps |
| High-Entropy Randomized | Traditional | 659.07 ns | 1.52 M qps |
| High-Entropy Randomized | **Hypute Stream** | **180.90 ns** | **5.53 M qps** |

**Memory retained after execution:**

| Engine | RSS Baseline | Retained Delta | Peak RSS |
|---|---|---|---|
| Traditional | 575.98 MB | +1,058.80 MB | 1,637 MB |
| **Hypute Stream** | 575.95 MB | **+0.30 MB** | 1,148 MB |

---

### Amazon Reviews '23 — 100M Streaming State Mutation

> UCSD Amazon Reviews 2023 · 100,000,000 interaction rows · [GitHub Actions run](https://github.com/hayako-org/hypute-stream/actions/runs/27936025854)

| Workload | Engine | Latency | Throughput |
|---|---|---|---|
| Preserved Sorted | Traditional | 337.39 ns | 2.96 M qps |
| Preserved Sorted | **Hypute Stream** | **155.48 ns** | **6.43 M qps** |
| High-Entropy Randomized | Traditional | 909.73 ns | 1.10 M qps |
| High-Entropy Randomized | **Hypute Stream** | **156.22 ns** | **6.40 M qps** |

**Memory retained after execution:**

| Engine | RSS Baseline | Retained Delta | Peak RSS |
|---|---|---|---|
| Traditional | 2,292.21 MB | +8,821.86 MB | ~11,115 MB |
| **Hypute Stream** | 2,292.22 MB | **+0.23 MB** | ~2,292 MB |

---

## Key Observations

**Throughput stability under entropy.** Traditional associative-map throughput collapses 63–73% when access order is randomized (sorted → random: 13.62→1.52 M qps at 25M; 2.96→1.10 M qps at 100M). Hypute Stream latency and throughput remain essentially flat across both workload modes.

**Scale inversion.** At 25M records, traditional achieves lower latency on sorted access. At 100M records, Hypute Stream is faster in both modes — the hash-map overhead overtakes locality gains as working set size grows.

**Memory retention.** After processing 100M records, the traditional engine retained ~8.8 GB of heap (accumulated map state). Hypute Stream retained 0.23 MB. This gap scales with dataset size and has direct implications for long-running or multi-tenant deployments.

All measurements are workload-specific. Results will vary by hardware, compiler, and OS scheduling. Run the benchmarks on your own infrastructure to validate.

---

## Running the Benchmarks

### MovieLens 25M

```bash
# Download dataset
cd benchmarks/movielens && ./dataset_download.sh && cd ../..

# Build
mkdir -p build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

# Run
./build/benchmark_runner traditional
./build/benchmark_runner hypute
```

### Amazon Reviews 100M

```bash
# Generate dataset (~100M rows streamed from UCSD public files)
mkdir -p benchmarks/amazon_reviews23/generated
python3 benchmarks/amazon_reviews23/generate_dataset.py \
  benchmarks/amazon_reviews23/generated/amazon_reviews23.csv 100000000

# Run (reuse the same binary)
./build/benchmark_runner traditional DEFAULT_CTX \
  benchmarks/amazon_reviews23/generated/amazon_reviews23.csv

./build/benchmark_runner hypute DEFAULT_CTX \
  benchmarks/amazon_reviews23/generated/amazon_reviews23.csv
```

### Via GitHub Actions

Both benchmarks are available as `workflow_dispatch` workflows under the **Actions** tab — no local setup required.

---

## Prerequisites

- C++17 compiler (GCC ≥ 9 or Clang ≥ 10)
- CMake ≥ 3.12
- Python 3 (for Amazon Reviews dataset generation)
- curl + unzip (for MovieLens download)

---

## Repository Layout

```
benchmarks/
  movielens/          # 25M benchmark (dataset download + runner)
  amazon_reviews23/   # 100M benchmark (dataset generator + runner)
evaluation/           # Lightweight environment verification harness
hypute_stream.h       # Public evaluation API header
```

---

## About

This repository is a limited public evaluation environment. It does not include components distributed in commercial deployments.

For enterprise evaluations, pilot programs, or commercial discussions, contact the [Hayako](https://hayako.io) team.

© 2026 Hayako. All rights reserved.

# 2-Level Cache Simulator

> A physically-indexed, physically-tagged cache simulator I developed for my CSN 506 - Advanced Computer Architecture course (2022)

After spending weeks learning about cache architectures in class, I wanted to build something that would help me visualize how different cache configurations actually perform in real-world scenarios. This simulator is the result of that exploration.

## 🤔 Why This Project?

Cache performance is critical to overall system performance, but it's difficult to experiment with different cache configurations on physical hardware. This simulator allows you to:

- See how different replacement policies affect hit rates
- Understand the performance implications of various cache parameters
- Visualize the trade-offs between L1 and L2 cache configurations
- Analyze memory access patterns from real-world applications

## 📌 Features

- **Two-level cache hierarchy** (L1 + L2) with realistic modeling
- **Multiple replacement policies**: LRU, FIFO, and Random
- **Write-back with write allocate strategy** for efficient memory operations
- **USIMM trace file support** for real-world application benchmarking
- **Detailed statistics**: Hit rates, miss rates, writebacks, and access counts
- **Configurable parameters**: Easily adjust cache size, associativity, and block size

## ⚙️ Technical Specifications

The simulator is configured with the following default parameters, which you can modify in the code:

| Cache  | Size  | Associativity | Block Size | Index Bits | Tag Bits |
|--------|-------|--------------|------------|------------|----------|
| **L1** | 64KB  | 8-way        | 64B        | 7          | 19       |
| **L2** | 256KB | 16-way       | 64B        | 8          | 18       |

## 🏗️ Repository Structure

I've organized the code to be modular and easy to navigate:

```
cache-controller/
├── include/              # Header files
│   ├── cache.h           # Cache structures and interfaces
│   ├── controller.h      # Main cache controller logic
│   ├── replacement.h     # Replacement policy implementations
│   ├── trace_parser.h    # For parsing memory access traces
│   └── write_buffer.h    # Write buffer implementation
├── src/                  # Implementation files
│   ├── cache.cpp
│   ├── controller.cpp
│   ├── main.cpp          # Program entry point
│   ├── replacement.cpp
│   ├── trace_parser.cpp
│   └── write_buffer.cpp
├── traces/               # Sample memory access traces
├── Makefile              # Build configuration
└── README.md             # This file
```

## 🚀 Building & Running

Super simple to get started:

```bash
# Compile everything
make clean && make

# Run with a trace file
./bin/cache_simulator traces/gcc.trace

# Enhanced mode (with write buffer)
./bin/cache_simulator -e traces/gcc.trace
```

You can also customize the simulation parameters by modifying the constants in `include/cache.h`.

## 📊 Sample Results

Here's what the output looks like when running with the GCC trace file:

```
Final Statistics:
===== L1 Cache Statistics =====
Total accesses: 515,683
Hits: 496,132 (96.21%)
Misses: 19,551 (3.79%)
Hit rate: 96.21%
Miss rate: 3.79%
Write-backs: 843
==========================
===== L2 Cache Statistics =====
Total accesses: 20,394
Hits: 10,571 (51.83%)
Misses: 9,823 (48.17%)
Hit rate: 51.83%
Miss rate: 48.17%
Write-backs: 255
==========================
Memory accesses: 9,326
```

I found it interesting how much the L1 hit rate affects overall performance. Even small tweaks to the replacement policy can make a noticeable difference!

## 🔍 What I Learned

Building this simulator deepened my understanding of:
- How different replacement policies perform under various workloads
- The delicate balance between cache size and associativity
- Write-back vs. write-through strategies and their implications
- The importance of locality in memory access patterns


## 📄 License

This project is released under the MIT License - feel free to use and modify as needed!

---

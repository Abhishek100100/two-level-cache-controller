# 2-Level Cache Simulator

> A physically-indexed, physically-tagged cache simulator developed for CSN 506 - Advanced Computer Architecture course (2022)

This project implements a two-level cache simulator that was assigned as part of my course requirements. It models a realistic cache hierarchy and allows for analysis of different cache configurations.

## 💻 Features

- **Two-level cache hierarchy** (L1 + L2)
- **Multiple replacement policies**: LRU, FIFO, and Random
- **Write-back with write allocate strategy**
- **USIMM trace file support** for memory access pattern analysis
- **Detailed statistics**: Hit rates, miss rates, and writebacks
- **Configurable parameters**: Cache size, associativity, and block size

## 💻 Technical Specifications

The simulator uses the following default parameters:

| Cache  | Size  | Associativity | Block Size | Index Bits | Tag Bits |
|--------|-------|--------------|------------|------------|----------|
| **L1** | 64KB  | 8-way        | 64B        | 7          | 19       |
| **L2** | 256KB | 16-way       | 64B        | 8          | 18       |

## 💻 Repository Structure

```
cache-controller/
├── include/              # Header files
│   ├── cache.h           # Cache structures
│   ├── controller.h      # Cache controller
│   ├── replacement.h     # Replacement policies
│   ├── trace_parser.h    # Trace parsing
│   └── write_buffer.h    # Write buffer
├── src/                  # Implementation
│   ├── cache.cpp
│   ├── controller.cpp
│   ├── main.cpp
│   ├── replacement.cpp
│   ├── trace_parser.cpp
│   └── write_buffer.cpp
├── traces/               # Sample traces
├── Makefile              # Build system
└── README.md             # This file
```

## 💻 Building & Running

```bash
# Compile
make clean && make

# Run with trace file
./bin/cache_simulator traces/gcc.trace

# Enhanced mode (with write buffer)
./bin/cache_simulator -e traces/gcc.trace
```

## 📊 Sample Results

Output when running with the GCC trace file:

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

## 💻 Assignment Context

This simulator was developed to demonstrate an understanding of:
- Multi-level cache hierarchies
- Various cache replacement algorithms
- Memory access pattern analysis
- Performance implications of different cache configurations

The code fulfills the course requirements for implementing a functional cache simulator that handles realistic memory access patterns.

## 💻 Usage Notes

- The trace files should follow the USIMM format
- Default configuration can be modified in the header files
- The simulator is intended for educational purposes only

---

*Created as a course project for CSN 506 - Advanced Computer Architecture (2022) at IIT Roorkee*

```markdown
# Comparative Analysis of SSSP Algorithms: Dijkstra, Bellman-Ford, and BMSSP

This repository features a comprehensive performance benchmark of three Single-Source Shortest Path (SSSP) algorithms on real-world large-scale road networks. The study evaluates **Dijkstra’s Algorithm**, **Bellman-Ford**, and a modern **BMSSP/DMMSY** variant across 135 independent test runs.

## 🚀 Project Overview

The goal of this project is to analyze algorithmic efficiency in terms of **runtime**, **memory consumption**, and **scalability** using directed graph datasets with over 1.5 million nodes.

### Key Highlights:
- **Datasets:** DIMACS Road Networks (NY, COL, NE).
- **Large-Scale Testing:** Up to 1.5M vertices and 3.9M edges.
- **Environment:** WSL (Ubuntu 24.04), C++17 compiled with `-O2` optimization.
- **Precision:** Memory tracking via Linux `getrusage()` and `ru_maxrss`.

## 📊 Key Findings

* **Dijkstra:** Remains the gold standard for road networks, offering the best balance of speed and memory efficiency.
* **Bellman-Ford:** Highly inefficient for large-scale graphs (e.g., **34.24s** average vs **0.23s** on the NE dataset compared to Dijkstra).
* **BMSSP:** Competitive runtime matching Dijkstra, but at a **1.85x higher memory cost** due to auxiliary workspace structures.

## 🛠️ Implementation Details

To ensure a fair and transparent comparison:
- **Custom-built:** Dijkstra and Bellman-Ford were implemented from scratch to ensure precise control over data structures and memory tracking.
- **Adapted:** The BMSSP/DMMSY analysis uses a modified version of the reference implementation to ensure compatibility with our CSR graph structure and modern benchmarking tools.
- **Validation:** All implementations were cross-validated to ensure identical shortest-path distance outputs across all datasets.

## 📂 Repository Structure

```text
├── src/                    # C++ source codes (Dijkstra, Bellman-Ford, BMSSP)
├── reports/                # Final academic analysis report (PDF)
├── results/                # Performance plots (.png) and raw data (.xlsx)
│   ├── Runtime_Comparison.png
│   ├── Memory_Usage.png
│   ├── Scalability_Curve.png
│   ├── Efficiency_Scatter.png
│   └── Source_Stability_NE.png
└── README.md               # Project documentation
```

## Algorithms

| Algorithm | Complexity | Source |
|---|---|---|
| Dijkstra | O((V+E) log V) | Implemented from scratch |
| Bellman-Ford | O(V·E) | Implemented from scratch |
| BMSSP/DMMSY | O(m log²/³ n) | Adapted from [danalec/DMMSY-SSSP](https://github.com/danalec/DMMSY-SSSP) |

## Datasets

Download from the [9th DIMACS Implementation Challenge](http://users.diag.uniroma1.it/challenge9/download.shtml):

- `USA-road-d.NY.gr` — New York City (264,346 nodes, 733,846 edges)
- `USA-road-d.COL.gr` — Colorado (435,666 nodes, 1,057,066 edges)
- `USA-road-d.NE.gr` — Northeast USA (1,524,453 nodes, 3,897,636 edges)

Place the downloaded `.gr` files in the same directory as the source files before compiling.

## Requirements

- Linux or WSL (Windows Subsystem for Linux)
- g++ with C++17 support
- gcc (for compiling the DMMSY C library files)

## Build & Run

### Dijkstra

```bash
g++ -O2 -std=c++17 -o dijkstra src/djkstra_bellek.cpp
./dijkstra
```

### Bellman-Ford

```bash
g++ -O2 -std=c++17 -o bellmanford src/bellman-ford.cpp
./bellmanford
```

### BMSSP/DMMSY

```bash
g++ -O2 -std=c++17 -o dmmsy src/dmssy.cpp \
    src/dmmsy_lib/common.c \
    src/dmmsy_lib/dmmsy_opt.c \
    src/dmmsy_lib/dmmsy_res.c
./dmmsy
```

## Changing Source Node and Dataset

To change the dataset or source node, open the corresponding `.cpp` file and modify these two lines near the bottom of `main()`:

```cpp
string filename = "USA-road-d.NE.gr";  // change dataset here
int src = 1372007;                      // change source node here (0-based)
```

Then recompile and run.

## Output Format

Each program prints:

- Runtime (seconds)
- Graph storage memory (MB)
- Algorithm working memory (MB)
- Estimated total memory (MB)
- OS peak memory reference (MB)

## 📊 Results
Full test results (135 runs) are available in [`results/SSSP_test_results_professional.xlsx`](./results/SSSP_test_results_summary.xlsx).

## 📈 Performance Visuals

### Scalability Analysis
The following curve illustrates how algorithms respond as graph sizes grow from 264K (NY) to 1.5M (NE) nodes.
![Scalability Curve](./results/Scalability_Curve.png)

### Source Stability Analysis
This box plot confirms the operational consistency of Dijkstra and BMSSP regardless of the starting node, contrasted with Bellman-Ford's high sensitivity to source location.
![Source Stability](./results/Source_Stability_NE.png)

## 📄 Full Report
For a detailed discussion on the "Time-Memory Trade-off" and theoretical vs. practical observations, please refer to the [Full Analysis Report](./reports/SSSP_Report.pdf).

## 🔗 References
1. 9th DIMACS Implementation Challenge - Shortest Paths.
2. Duan et al. "Breaking the Sorting Barrier for Directed Single-Source Shortest Paths," STOC 2025.
3. [DMMSY-SSSP Reference Implementation Repository](https://github.com/danalec/DMMSY-SSSP).

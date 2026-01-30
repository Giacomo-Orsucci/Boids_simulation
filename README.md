# Boids Simulation - Parallelization performance Analysis 

This project implements a flocking behavior simulation based on the **Boids** algorithm in C++, using **SFML** for graphical visualization.

The main goal is to analyze and compare the performance of different optimization techniques and memory layouts, specifically focusing on:
*   **AOS (Array of Structures)** vs **SOA (Structure of Arrays)**.
*   Parallelization using **OpenMP**.
*   Vectorization using **SIMD (AVX2)** instructions.
*   Padding effects on parallelization.

The sequential version is the AOS one, setting the number of threads equal to 1.

## Requirements

*   C++ Compiler (C++17/20 support)
*   CMake
*   OpenMP Library
*   SFML Library (handled automatically via CMake FetchContent)
*   CPU with AVX2 support (for SIMD versions)
*   Python 3 & Pandas/Matplotlib (for benchmarking scripts)


## Project Structure
The codebase is divided into several variants to facilitate benchmarking:

*   `AOS`: Baseline implementation using Array of Structures.
*   `AOS_parallel_SIMD`: Optimized AOS version using OpenMP, padding, and memory alignment to support vectorization.
*   `SOA`: Implementation using Structure of Arrays.
*   `SOA_parallel_SIMD`: The strictly optimized version. It combines the cache-friendly SOA layout, OpenMP, and explicit branchless logic for efficient SIMD usage other than memory alignment and, optionally, padding.

Each version is indipendent, resulting in a little redundant code but perfectly adaptable. In detail:

*   `src`: contains all the header files (all with inside the config struct used to pass the execution parameters injected via python) for the different implementations.

*   `scripts`: contains `run_benchmark.py` to execute the benchmarks interested and `stats_plot_script.py` to plot the results obtained and written in `.csv` files with the name specified in `run_benchmark.py`. 


All the files are higly commented to allow the maximum comprehension and possibility of adaptation.


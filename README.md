Cache & Memory Simulation Collection
====================================

Overview
--------
This repository collects several small cache and memory-system simulators and analysis tools used for education and experimentation. It contains hand-implemented set-associative and direct-mapped cache simulators (C++), unit tests, and Python analysis utilities for reuse-distance and working-set analysis.

Top-level layout (what's in this repo)
-------------------------------------
- `cache sim/` — multiple cache simulator variants (C++ source, tests, small drivers)
  - `4-way cache/` — 4-way set-associative cache implementation and tests
  - `direct-way/` — direct-mapped cache implementation and tests
- `memory system simulator/` — a separate memory system simulator (C++ + Python)
- `python/` — small helpers and scripts (config loader, runners)
- `data/` — example configs and traces

Contributing and usage
----------------------
- Read each component's README (each subfolder has a focused README) for build/run instructions.
- Build C++ components with CMake or `g++ -std=c++17`.
- Python analysis lives in each `python/analysis` folder; use a `venv` and install `requirements.txt` (if present).

Author
------
Joshua Carter — First-year Computer Engineering student at Concordia University, Montreal. This repository was prepared as a learning project and set of examples for cache and memory system simulation.

- Per-folder README.md files added to document build/run instructions.
- A `requirements.txt` placed in the Python analysis directory to pin dependencies.

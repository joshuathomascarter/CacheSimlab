Cache & Memory Simulation Collection
====================================

Overview
--------
This repository collects several small cache and memory-system simulators and analysis tools used for education and experimentation. It contains hand-implemented set-associative and direct-mapped cache simulators (C++), unit tests, and Python analysis utilities for reuse-distance and working-set analysis.

Suggested repository name and short description
----------------------------------------------
- Repository name: `CacheSimLab`
- Short description: "Educational cache and memory simulators in C++ with Python analysis. Features set-associative and direct-mapped caches, LRU replacement, working-set and reuse-distance tools. Includes unit tests, build scripts, and sample traces for learning computer architecture concepts."

Organization and repo strategy
------------------------------
I recommend keeping everything together in a single repository with clear subfolders for each independent component. They share a common theme (cache simulation & analysis) and will benefit from a single README, CI, and release management. If any component grows into a standalone project (many files, separate release cadence), you can split it into its own repo later.

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

License
-------
Add a license file (MIT is common for educational repos). If you want, I can add a `LICENSE` file.

Contact / Author
----------------
Add an author section or your GitHub handle here.

Author
------
Joshua Carter — First-year Computer Engineering student at Concordia University, Montreal. This repository was prepared as a learning project and set of examples for cache and memory system simulation.

Files added by the assistant
---------------------------
- Per-folder README.md files added to document build/run instructions.
- A `requirements.txt` placed in the Python analysis directory to pin dependencies.

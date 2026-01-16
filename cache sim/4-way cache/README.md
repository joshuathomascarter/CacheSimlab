4-way Set-Associative Cache
===========================

Overview
--------
This folder contains a didactic implementation of a set-associative cache with LRU replacement. It includes a small unit test harness and Python analysis scripts.

What's included
---------------
- `c++/set_associative_cache.h` and `c++/set_associative_cache.cpp` — Cache implementation
- `c++/lru_tracker.h`, `c++/cache_set.h` — small helpers
- `tests c++/test_set_associative.cpp` — test harness and test cases
- `python/analysis/working_set.py` and `reuse_distance.py` — analysis utilities

Build & run (C++)
-----------------
From this folder run:

```sh
# Using CMake (recommended)
mkdir build && cd build
cmake ..
make
./test_cache
```

Or manually:
```sh
g++ -std=c++17 -I"./include" "./tests c++/test_set_associative.cpp" "./c++/set_associative_cache.cpp" -o test_cache
./test_cache
```

This compiles the tests and runs the test suite. Tests print per-test output and a pass/fail summary.

Python analysis
---------------
The analysis scripts are under `python/analysis`. Create a venv and install dependencies:

```sh
python -m venv .venv
source .venv/bin/activate
python -m pip install -r python/analysis/requirements.txt
python python/analysis/working_set.py
python python/analysis/reuse_distance.py
```

Notes
-----
- The C++ code targets C++17 and uses only the standard library; it should compile with `g++` or `clang++`.
- The Python scripts generate PNG plots (saved to the working directory). Modify scripts to point to real trace files.

Direct-Mapped Cache
===================

Overview
--------
This folder contains a didactic implementation of a direct-mapped cache. It includes a unit test harness and Python trace generation scripts.

What's included
---------------
- `c++/direct_mapped_cache.h` and `c++/direct_mapped_cache.cpp` — Cache implementation
- `c++/cache_line.h` — Cache line structure
- `c++/test_direct_mapped.cpp` — test harness and test cases
- `python/random_trace.py` and `sequential_trace.py` — trace generation utilities

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
g++ -std=c++17 -I"./include" "./c++/test_direct_mapped.cpp" "./c++/direct_mapped_cache.cpp" -o test_cache
./test_cache
```

This compiles the tests and runs the test suite. Tests print per-test output and a pass/fail summary.

Python trace generation
-----------------------
The trace scripts are under `python/`. They generate example access patterns:

```sh
python python/random_trace.py
python python/sequential_trace.py
```

Notes
-----
- The C++ code targets C++17 and uses only the standard library; it should compile with `g++` or `clang++`.
- Modify trace scripts to integrate with the cache simulator for custom evaluations.
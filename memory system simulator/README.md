Memory System Simulator
========================

Overview
--------
This folder contains a memory system simulator with configuration-driven execution, statistics collection, and Python utilities for setup and logging.

What's included
---------------
- `c++/main.cpp`, `c++/statistics.cpp`, `c++/statistics.h`, `c++/types.h`, `c++/config.h` — Core simulator implementation
- `python/config_loader.py`, `python/logger.py`, `python/run_simulator.py`, `python/test_config.py` — Python helpers for configuration and execution
- `data/config.json`, `data/high_perf_config.json` — Example configuration files

Build & run (C++)
-----------------
From this folder run:

```sh
g++ -std=c++17 c++/main.cpp c++/statistics.cpp -o memory_sim
./memory_sim
```

Or use the Python runner:
```sh
python python/run_simulator.py
```

Python utilities
----------------
- `config_loader.py` — Load and validate JSON configurations
- `logger.py` — Logging utilities
- `test_config.py` — Test configuration parsing
- `run_simulator.py` — Wrapper to run the C++ simulator with Python setup

Notes
-----
- Configurations are in JSON format; modify `data/` files for different scenarios.
- Statistics are collected during simulation runs.
- Integrate with the cache simulators in sibling folders for full system evaluation.
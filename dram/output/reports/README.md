# DRAM Model (single-bank) — day -4

This folder contains a small, easy-to-read DRAM single-bank timing model used for Day -4 of the memory architect plan.

Structure:
- `headers/` — public headers: `dram_timing.h`, `dram_bank.h`
- `cpp/` — implementation: `dram_bank.cpp`
- `tests/` — a small test binary `test_dram_bank.cpp`
- `python/` — `dram_timing_viz.py` for quick latency plots

Quick compile & run (from workspace root):
```bash
g++ -std=c++17 -Idram/headers -O2 -o dram/tests/test_dram_bank dram/tests/test_dram_bank.cpp dram/cpp/dram_bank.cpp
./dram/tests/test_dram_bank
```

The model is intentionally minimal: ROW_SIZE is 8 KiB, and timing values are in abstract cycles.

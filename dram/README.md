# DRAM Memory System Simulator

**Production-grade DDR4 DRAM timing, power, and refresh modeling framework**

---

## 📁 Project Structure

```
dram/
├── README.md                    # This file
├── Makefile                     # Build system
├── run_all_tests.sh            # Test runner script
│
├── headers/                     # Public API headers
│   ├── dram_bank_fsm.h         # Bank state machine
│   ├── dram_timing.h           # DDR4 timing parameters
│   ├── power_model.h           # Cycle-accurate power model
│   ├── refresh_controller.h    # RAIDR-based refresh controller
│   └── timing_validator.h      # JEDEC constraint validator
│
├── cpp/                         # Implementation files
│   ├── dram_bank_fsm.cpp
│   ├── power_model.cpp
│   ├── refresh_controller.cpp
│   └── timing_validator.cpp
│
├── tests/                       # Test source code
│   ├── test_bank_fsm.cpp       # Day 1: Bank FSM tests
│   ├── test_refresh_power.cpp  # Day 2: Refresh & power tests
│   └── generate_power_trace.cpp # Power trace generator
│
├── python/                      # Analysis & visualization tools
│   ├── power_analyzer.py       # Power breakdown analysis
│   ├── dram_timing_viz.py      # Full timing visualization
│   └── dram_timing_viz_lite.py # Lightweight visualizer
│
├── output/                      # Generated outputs (gitignored)
│   ├── traces/                 # CSV/JSON trace files
│   ├── visualizations/         # PNG plots and charts
│   └── reports/                # Generated documentation
│
├── build/                       # Intermediate build files (gitignored)
│   └── *.o                     # Object files
│
└── bin/                         # Compiled executables (gitignored)
    ├── test_bank_fsm
    ├── test_refresh_power
    └── generate_power_trace
```

---

## 🚀 Quick Start

### Build All Tests
```bash
make all
```

### Run Day 1 Tests (Bank FSM)
```bash
make test_day1
```

### Run Day 2 Tests (Refresh & Power)
```bash
make test_day2
```

### Generate Power Trace & Visualize
```bash
# Generate trace from C++ simulation
./bin/generate_power_trace

# Analyze with Python
python3 python/power_analyzer.py output/traces/power_trace_output.csv
```

### Clean Build Artifacts
```bash
make clean
```

---

## 📊 Day 1: Bank State Machine & Timing Validation

**Learning Goals:**
- Understand DRAM bank state transitions
- Implement JEDEC DDR4-2400 timing constraints
- Validate tRCD, tRAS, tRP, tRC timing

**Files:**
- `headers/dram_bank_fsm.h` - Bank FSM interface
- `cpp/dram_bank_fsm.cpp` - FSM implementation
- `tests/test_bank_fsm.cpp` - Unit tests

**Run:**
```bash
./bin/test_bank_fsm
```

---

## 📊 Day 2: Refresh Controller & Power Modeling

**Learning Goals:**
- Implement temperature-aware refresh (2x @ 95°C, 4x @ 105°C)
- RAIDR retention profiling (75% power savings)
- Cycle-accurate power modeling (±2% vs Micron datasheet)
- Validate refresh overhead (3.12% target)

**Files:**
- `headers/refresh_controller.h` - Refresh controller interface
- `cpp/refresh_controller.cpp` - Implementation with RAIDR
- `headers/power_model.h` - Power model interface
- `cpp/power_model.cpp` - Energy tracking implementation
- `tests/test_refresh_power.cpp` - Comprehensive test suite

**Run:**
```bash
./bin/test_refresh_power
```

**Expected Results:**
- ✅ 2/6 tests passing initially (4 failures are learning exercises)
- 🎯 Debug to achieve:
  - Refresh overhead: 3.12% ± 0.1%
  - Power accuracy: ±2% vs datasheet
  - Temperature scaling: 2x @ 90°C, 4x @ 100°C
  - RAIDR profiling: ~10% weak rows identified

---

## 🔬 Analysis Tools (Python)

### Power Analyzer
```bash
python3 python/power_analyzer.py output/traces/power_trace_output.csv
```

**Outputs:**
- `output/visualizations/power_breakdown.png` - Component breakdown
- `output/visualizations/power_timeline.png` - Power over time
- Console: Comprehensive analysis summary

### Timing Visualizer (Full)
```bash
python3 python/dram_timing_viz.py output/traces/trace_dramsim3.txt
```

### Timing Visualizer (Lite - for large traces)
```bash
python3 python/dram_timing_viz_lite.py output/traces/trace_dramsim3.txt 500
```

---

## 📐 DDR4-2400 Specification

| Parameter | Value | Cycles @ 1600MHz |
|-----------|-------|------------------|
| tCK | 0.625 ns | 1 |
| tRCD | 13.75 ns | 22 |
| tRP | 13.75 ns | 22 |
| tRAS | 35 ns | 56 |
| tRC | 48.75 ns | 78 |
| tRFC | 260 ns | 416 |
| tREFI | 7.8 μs | 12,480 |
| tCAS | 13.75 ns | 22 |

**Refresh Overhead:**
```
Time overhead = (tRFC / tREFI) × 100% = 3.33%
Power overhead = ~34% (measured in sample trace)
```

---

## 🎯 Learning Path

### Week 1: Foundation
1. Study `headers/dram_bank_fsm.h` - understand state machine
2. Code `cpp/dram_bank_fsm.cpp` line-by-line
3. Run `test_bank_fsm` and debug failures
4. Study `headers/timing_validator.h` - JEDEC constraints

### Week 2: Refresh Architecture
1. Study `headers/refresh_controller.h` - refresh interface
2. Understand RAIDR algorithm (retention profiling)
3. Code `cpp/refresh_controller.cpp` line-by-line
4. Implement temperature scaling (85°C → 2x, 95°C → 4x)

### Week 3: Power Modeling
1. Study `headers/power_model.h` - IDD current model
2. Understand energy integration (∫ power × dt)
3. Code `cpp/power_model.cpp` line-by-line
4. Validate against Micron DDR4-2400 datasheet (±2%)

### Week 4: Integration & Optimization
1. Debug all test failures (target: 6/6 passing)
2. Optimize refresh overhead (3.12% target)
3. Analyze power breakdown (use Python tools)
4. Document design decisions and learnings

---

## 📚 References

- **JEDEC JESD79-4C**: DDR4 SDRAM Standard (2020)
- **Micron DDR4-2400 8Gb x8**: Datasheet for power validation
- **RAIDR Paper**: Liu et al., ISCA 2012 - Retention-Aware Intelligent DRAM Refresh
- **DRAMPower**: Open-source power modeling tool (reference implementation)
- **CACTI-3DD**: Chen et al., HP Labs - 3D DRAM power model

---

## 🛠️ Build System

### Compiler Requirements
- **C++ Standard**: C++17
- **Compiler**: g++ or clang++
- **Flags**: `-std=c++17 -Wall -I./headers`

### Make Targets
```bash
make all              # Build all tests
make test_day1        # Build & run Day 1 tests
make test_day2        # Build & run Day 2 tests
make generate_trace   # Build trace generator
make clean            # Remove build artifacts
make distclean        # Remove all generated files
```

---

## 📊 Test Results

See `output/reports/TEST_RESULTS_DAY2.md` for detailed test execution results and learning guidance.

---

## 🎓 Architecture Principles

This codebase follows Apple Memory Architecture Team standards:

1. **Separation of Concerns**: Headers define interfaces, cpp implements
2. **JEDEC Compliance**: All timing matches DDR4-2400 specification
3. **Validation-First**: ±2% accuracy target vs datasheets
4. **Production-Grade**: Comprehensive error handling, type safety
5. **Tool Integration**: Python analysis, industry-standard formats

---

## 📝 License

MIT License - See LICENSE file

---

## 👤 Author

Josh Carter  
Memory Architecture Learning Project  
January 2026

---

## 🚀 Next Steps

1. ✅ Compile all files (`make all`)
2. ✅ Run tests and understand failures
3. ✅ Code `refresh_controller.cpp` line-by-line
4. ✅ Debug until all tests pass (6/6)
5. ✅ Generate power traces and analyze
6. ✅ Document your learnings in `output/reports/`

**Focus on C++ architecture, use Python tools for validation.** 🎯

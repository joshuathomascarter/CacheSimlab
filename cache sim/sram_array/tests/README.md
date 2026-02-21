# SRAM Controller Testing Framework

Complete validation infrastructure for cross-validating C++ behavioral model against Verilog RTL using Verilator.

## 🏗️ Architecture Overview

```
┌────────────────────────────────────────────────────────────────┐
│                    TESTING ARCHITECTURE                         │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────┐         ┌──────────────────┐            │
│  │  C++ Golden Model│         │  Verilog RTL     │            │
│  │  (Software)      │         │  (Hardware)      │            │
│  │                  │         │                  │            │
│  │  • 4-bank SRAM   │         │  • sram_array.v  │            │
│  │  • Byte-wide banks│         │  • sram_controller.v│       │
│  │  • Read: 2.5ns   │         │  • 3-stage pipeline│         │
│  │  • Write: 3.0ns  │         │  • Registered outputs│       │
│  └────────┬─────────┘         └────────┬─────────┘            │
│           │                            │                       │
│           │         COMPARISON         │                       │
│           └────────────┬───────────────┘                       │
│                        │                                       │
│              ┌─────────▼──────────┐                           │
│              │  tb_sram_controller│                           │
│              │  (Verilator-based) │                           │
│              │                     │                           │
│              │  1. Generate vectors│                           │
│              │  2. Run both models │                           │
│              │  3. Compare results │                           │
│              │  4. Report mismatches│                          │
│              └─────────┬──────────┘                           │
│                        │                                       │
│              ┌─────────▼──────────┐                           │
│              │  cross_validate.py │                           │
│              │  (Orchestrator)    │                           │
│              │                     │                           │
│              │  • Builds testbench │                           │
│              │  • Runs simulation  │                           │
│              │  • Parses results   │                           │
│              │  • Generates reports│                           │
│              └────────────────────┘                           │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

## 📁 File Structure

```
tests/
├── Makefile                    # Build automation
├── tb_sram_controller.cpp      # Main testbench (Verilator)
├── tb_sram_controller.sv       # SystemVerilog testbench (alternative)
└── README.md                   # This file

scripts/
└── cross_validate.py           # Python orchestrator

rtl/
├── sram_array.v                # Core 4-bank SRAM array
├── sram_controller.v           # Controller wrapper
└sram_pkg.sv                  # Package definitions
```

## 🚀 Quick Start

### Prerequisites

```bash
# macOS
brew install verilator

# Linux
sudo apt install verilator

# Verify installation
verilator --version  # Should be ≥ 4.0
```

### Option 1: Full RTL Validation (Recommended)

```bash
# From tests/ directory
make run_rtl
```

This will:
1. Build Verilator testbench (`obj_dir_rtl/Vsram_controller`)
2. Generate 500+ test vectors (sequential, random, corner cases, bursts)
3. Run C++ golden model
4. Run Verilog RTL simulation
5. Compare results
6. Report mismatches

### Option 2: Using Python Orchestrator

```bash
# From scripts/ directory
python3 cross_validate.py

# Force rebuild
python3 cross_validate.py --build-rtl

# C++ model only (no RTL)
python3 cross_validate.py --no-rtl
```

### Option 3: Standalone Golden Model Test

```bash
# From tests/ directory
make run_standalone
```

Tests the C++ behavioral model only (no RTL comparison).

## 📊 Test Coverage

The testbench generates diverse patterns:

### 1. Sequential Patterns
- **Sequential Writes**: 64 consecutive addresses
- **Sequential Reads**: 64 consecutive addresses
- Tests address decoder linearity

### 2. Random Patterns
- **256 random operations** (50% write, 50% read)
- Tests general functionality
- Stress-tests bank selection

### 3. Corner Cases
- **Boundary addresses**: `0x0000`, `0x0001`, `0x7FFF`, `0x8000`, `0xFFFE`, `0xFFFF`
- Each address gets write → read back
- Tests edge conditions in address decoder

### 4. Burst Patterns
- **10 bursts × 8 operations**
- Write burst → Read burst
- Tests pipeline behavior and bank parallelism

**Total: ~500 operations**

## 🎯 Validation Criteria

### ✅ PASS Criteria

```
Data Matches:     500/500 ✓ PERFECT
Timing Matches:   487/500 (>97%)
Avg Latency Diff: <10% (Jim Keller's rule)
```

### ⚠️ WARNING Criteria

```
Data Matches:     500/500 ✓
Avg Latency Diff: 10-15% (acceptable but review timing)
```

### ❌ FAIL Criteria

```
Data Matches:     <500/500 (ANY mismatch = CRITICAL BUG)
```

## 🔍 How It Works

### 1. Test Vector Generation

```cpp
TestVectorGenerator gen(42);  // Reproducible seed
gen.generate_sequential_writes(64);
gen.generate_sequential_reads(64);
gen.generate_random_pattern(256);
gen.generate_corner_cases();
gen.generate_burst_pattern(8, 10);
```

### 2. C++ Golden Model

```cpp
class SRAMGoldenModel {
    uint8_t bank0[65536];  // Byte-wide bank
    uint8_t bank1[65536];
    uint8_t bank2[65536];
    uint8_t bank3[65536];
    
    AccessResult read(uint16_t addr) {
        // Reassemble 32-bit word from 4 banks
        uint32_t data = (bank3[addr] << 24) |
                        (bank2[addr] << 16) |
                        (bank1[addr] << 8)  |
                        (bank0[addr] << 0);
        return {data, 2.5ns latency};
    }
    
    AccessResult write(uint16_t addr, uint32_t data) {
        // Split into bytes (one per bank)
        bank0[addr] = (data >> 0)  & 0xFF;
        bank1[addr] = (data >> 8)  & 0xFF;
        bank2[addr] = (data >> 16) & 0xFF;
        bank3[addr] = (data >> 24) & 0xFF;
        return {data, 3.0ns latency};
    }
};
```

### 3. Verilator RTL Simulation

```cpp
#ifdef VERILATOR
class RTLSimulator {
    Vsram_controller* dut;  // Generated by Verilator
    
    AccessResult read(uint16_t addr) {
        dut->rd_addr = addr;
        dut->rd_en = 1;
        tick();  // Clock edge
        dut->rd_en = 0;
        
        // Wait for rd_valid (3-cycle pipeline)
        while (!dut->rd_valid) tick();
        
        return {dut->rd_data, latency};
    }
};
#endif
```

### 4. Result Comparison

```cpp
for (each test vector) {
    AccessResult golden = golden_model.run(vector);
    AccessResult rtl    = rtl_simulator.run(vector);
    
    // Data must match EXACTLY
    if (golden.data != rtl.data) {
        ERROR("Data mismatch!");
    }
    
    // Timing allows 10-15% variance
    double diff_percent = abs(golden.latency - rtl.latency) / golden.latency * 100;
    if (diff_percent < 10%) {
        PASS("Within Jim Keller's rule");
    }
}
```

## 📈 Example Output

```
════════════════════════════════════════════════════════════════
SRAM CONTROLLER TESTBENCH
════════════════════════════════════════════════════════════════

[1/4] Generating Test Vectors...
  ✓ Generated 502 test vectors

[2/4] Running C++ Golden Model...
  ✓ Golden model simulated 502 operations

[3/4] Running Verilog RTL Simulation...
  ✓ RTL simulation completed (502 operations)

[4/4] Comparing Golden Model vs RTL...

════════════════════════════════════════════════════════════════
CROSS-VALIDATION REPORT: C++ Golden Model vs Verilog RTL
════════════════════════════════════════════════════════════════

TEST RESULTS:
  Total Operations:      502
  Data Matches:          502/502 ✓ PERFECT
  Hit Matches:           502/502
  Timing Matches (<15%): 487/502

TIMING ANALYSIS:
  Average Latency Diff:  4.23%
  Max Latency Diff:      8.91%

VERDICT:
  ✓ ALL TESTS PASSED
  ✓ RTL matches C++ golden model perfectly!
  ✓ Timing within Jim Keller's 10% rule

════════════════════════════════════════════════════════════════

✓ Validation complete - RTL ready for integration!
```

## 🐛 Debugging Failures

### Data Mismatch Example

```
MISMATCHES FOUND (3):
  [1] Cycle 42: Data mismatch at addr 0x0100:
      expected 0xDEADBEEF, got 0xDEADBE00
  [2] Cycle 89: Data mismatch at addr 0x7FFF:
      expected 0xAAAAAAAA, got 0x00000000
  [3] Cycle 156: Data mismatch at addr 0x8000:
      expected 0x12345678, got 0x12340000
```

**Root Cause Analysis:**
- Pattern: Last byte is always `0x00`
- Hypothesis: `bank3` (bits 31:24) not writing correctly
- Check: [sram_array.v](../rtl/sram_array.v#L95) write logic for bank3

### Timing Mismatch Example

```
TIMING ANALYSIS:
  Average Latency Diff:  18.5%  ⚠️ exceeds 10%!
  Max Latency Diff:      25.3%
```

**Root Cause Analysis:**
- RTL pipeline may have extra stages
- Check: [sram_array.v](../rtl/sram_array.v#L150) registered vs combinational outputs
- Compare: C++ model assumes 3 cycles, RTL may be 4 cycles

## 🏗️ Build Artifacts

```
obj_dir_rtl/                    # Verilator build directory
├── Vsram_controller           # Executable testbench
├── Vsram_controller.cpp        # Generated C++ wrapper
├── Vsram_controller.h          # Generated header
├── Vsram_controller__Syms.h    # Symbol table
└── *.o                         # Object files

test_golden_model               # Standalone C++ executable
*.vcd                           # Waveform dumps (if enabled)
cross_validation_report.json    # Machine-readable results
```

## 🎓 Key Concepts

### Why Two Golden Models?

You might notice:
1. **Full behavioral model**: `sram_behavioral_model.cpp` (separate file)
2. **Lightweight copy**: `SRAMGoldenModel` (in testbench)

**Reason:**
- Full model has statistics, exports, self-tests → heavyweight
- Testbench model is minimal → just read/write → lightweight
- Both use SAME algorithm → should produce identical results

### Verilator `-DVERILATOR` Flag

```cpp
#ifdef VERILATOR
    // Use actual RTL
    RTLSimulator rtl;
#else
    // Mock RTL (for quick testing)
    MockRTL rtl;
#endif
```

**When building:**
```bash
# WITH Verilator (real RTL):
verilator --cc *.v --exe tb_sram_controller.cpp -CFLAGS "-DVERILATOR"

# WITHOUT Verilator (mock RTL):
g++ -o test_standalone tb_sram_controller.cpp
```

### Pipeline Timing

```
Cycle 0:  rd_en=1, rd_addr=0x1000  │ Request issued
Cycle 1:  address latched          │ Pipeline stage 1
Cycle 2:  array read                │ Pipeline stage 2
Cycle 3:  rd_valid=1, rd_data=0x...│ Data ready ✓
```

C++ model predicts: **2.5ns** (< 1 cycle at 8ns period)
RTL actual: **~3 cycles = 24ns**
Difference: **~860%** → **NORMAL FOR PIPELINED RTL!**

**Solution:** Testbench waits for `rd_valid` signal, measures actual cycles.

## 📚 Related Documentation

- [sram_array.v RTL](../rtl/sram_array.v) - Core 4-bank implementation
- [sram_controller.v RTL](../rtl/sram_controller.v) - Controller wrapper
- [cross_validate.py](../scripts/cross_validate.py) - Python orchestrator
- [Main README](../README.md) - Project overview

## 🎯 CI/CD Integration

```yaml
# .github/workflows/rtl_validation.yml
- name: Build and Test RTL
  run: |
    cd tests
    make run_rtl || exit 1

- name: Parse Results
  run: |
    python3 scripts/cross_validate.py
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
      echo "RTL validation FAILED!"
      exit 1
    fi
```

## 🤝 Contributing

When modifying RTL:
1. Run `make run_rtl` to verify no regressions
2. If failures occur, check [Debugging section](#-debugging-failures)
3. Update tests if adding new features
4. Ensure all tests pass before committing

## 📞 Support

For questions:
- **RTL bugs**: Check waveforms (enable `VCD` in testbench)
- **Test failures**: Review mismatch log
- **Build issues**: Verify Verilator installation
- **Performance**: Run `make run_standalone` for quick iteration

---

**Last Updated**: February 2026  
**Apple Memory Architect Path** - Day 2 Software Validation

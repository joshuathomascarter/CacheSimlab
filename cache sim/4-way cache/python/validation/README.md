# Cache Simulator Validation Suite

Complete validation pipeline for testing your C++ 4-way cache simulator.

## 📋 Overview

This suite validates that your C++ cache simulator produces **correct results** by:

1. **Generating a predictable test trace** with known hit/miss patterns
2. **Hand-calculating expected results** using reference implementation
3. **Running your C++ simulator** against the same trace
4. **Comparing outputs** to verify correctness

## 🗂️ Files

### Python Scripts

- **`generate_trace.py`** — Creates controlled access pattern
  - Fills cache, tests hits, tests LRU eviction
  - Outputs: `test_data/trace.txt`

- **`manual_trace.py`** — Hand-traces cache behavior
  - Simulates your cache step-by-step
  - Generates ground truth: `test_data/expected_results.txt`

- **`parse_cpp_output.py`** — Extracts results from C++ binary
  - Runs `test_cache` with trace file
  - Parses and formats output: `test_data/cpp_results.txt`

- **`validate.py`** — Compares expected vs actual
  - Checks every access (hit/miss)
  - Generates detailed report: `test_data/validation_report.txt`

### Shell Script

- **`run_validation.sh`** — Master orchestrator
  - Runs all 4 Python scripts in sequence
  - Handles compilation if needed
  - Displays final pass/fail

## 🚀 Quick Start

### Option 1: Run Everything Automatically

```bash
cd /Users/joshcarter/Desktop/memory_sim/cache\ sim/4-way\ cache/python/validation
./run_validation.sh
```

### Option 2: Run Individual Steps

```bash
# Step 1: Generate trace
python3 generate_trace.py

# Step 2: Calculate expected results
python3 manual_trace.py

# Step 3: Run C++ simulator and parse output
python3 parse_cpp_output.py test_data/trace.txt ../test_cache

# Step 4: Validate
python3 validate.py
```

## 📊 Example Output

```
🔍 STEP 1: Generating test trace...
✅ Generated 15 addresses to test_data/trace.txt

Access 1: 0x0000 (Block 0)
Access 2: 0x0040 (Block 1)
...

✅ Expected results written to test_data/expected_results.txt

⚙️  STEP 3: Running C++ cache simulator...

✔️  STEP 4: Comparing expected vs actual results...

VALIDATION REPORT
════════════════════════════════════════════════════════════
✅ ALL TESTS PASSED

Summary:
  Expected hits:   10
  Actual hits:     10
  Expected misses: 5
  Actual misses:   5
  Expected rate:   66.67%
  Actual rate:     66.67%

No mismatches found. ✅
════════════════════════════════════════════════════════════
```

## 📂 Test Data Files

Generated in `test_data/`:

- **`trace.txt`** — Input addresses (hex format)
  ```
  0x0000
  0x0040
  0x0080
  ...
  ```

- **`expected_results.txt`** — Ground truth results
  ```
  #    Address   Block  Set  Way  Tag  Result
  1    0x0000    0      0    0    0    MISS
  2    0x0040    1      0    1    0    MISS
  3    0x0000    0      0    0    0    HIT
  ...
  ```

- **`cpp_results.txt`** — C++ simulator output (same format)

- **`validation_report.txt`** — Detailed comparison

## 🔧 Customizing Test Patterns

Edit `generate_trace.py` to create different access patterns:

```python
# Add more addresses to trace list
trace.append(0x0200)  # New access
trace.append(0x0000)  # Re-access (should hit)
```

Then re-run:
```bash
python3 manual_trace.py
python3 parse_cpp_output.py test_data/trace.txt ../test_cache
python3 validate.py
```

## ⚙️ How It Works

### Test Trace Generation

The default trace tests:
1. **Fills cache** (4 accesses to different blocks)
2. **Tests HIT** (re-access first block)
3. **Tests LRU eviction** (access 5th block, evicts LRU)
4. **Verifies eviction** (re-access evicted block)
5. **Complex patterns** (multiple evictions, state tracking)

### Manual Trace

Reference implementation that:
- Maintains cache state (tags, valid bits)
- Tracks LRU counters
- Classifies each access as HIT/MISS
- Records which way holds which tag

### C++ Integration

Your C++ binary should:
- Accept trace file as command-line argument
- Output results in predictable format
- Print summary statistics

**Note:** If your binary outputs different format, edit `parse_cpp_output.py` to parse it.

## 🐛 Troubleshooting

### "Could not find test_cache binary"

Compile your C++ code first:
```bash
cd ../  # Go to 4-way cache directory
cmake -B build
cmake --build build
```

Then re-run validation:
```bash
python3 parse_cpp_output.py test_data/trace.txt build/test_cache
```

### "Could not parse line"

Your C++ output format may differ. Edit `parse_cpp_output.py` to match your format:
```python
# Look for your actual output pattern:
if 'HIT' in line.upper():
    hits += 1
```

### Mismatches in validation

Check the detailed report:
```bash
cat test_data/validation_report.txt
```

Common issues:
- Off-by-one in cache indexing
- Incorrect LRU replacement logic
- Address decoding bug

## 📝 Cache Configuration (Reference)

- **Cache size:** 256 bytes
- **Block size:** 64 bytes
- **Associativity:** 4-way
- **Sets:** 1 (256 / 64 / 4)

Address decoding:
- Offset bits: 6 (log₂ 64)
- Index bits: 0 (log₂ 1)
- Tag bits: remaining

## ✅ Validation Success Criteria

All tests pass when:
- ✅ Hit count matches expected
- ✅ Miss count matches expected
- ✅ Hit rate matches expected
- ✅ Every access (HIT/MISS) matches expected
- ✅ All 15 accesses verified

## 📚 Next Steps

After validation passes:
1. **Add more test patterns** to `generate_trace.py`
2. **Test edge cases** (all misses, all hits, cache thrashing)
3. **Performance profiling** (trace from real workload)
4. **Move to DRAM validation** (similar pipeline for DRAM simulator)

---

**Happy validating! 🎉**

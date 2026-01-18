# Quick Reference: Cache Validation Pipeline

## 📋 Cheat Sheet

### Run Everything
```bash
cd "/Users/joshcarter/Desktop/memory_sim/cache sim/4-way cache/python/validation"
./run_validation.sh
```

### Generate Trace Only
```bash
python3 generate_trace.py
# Output: test_data/trace.txt (15 addresses)
```

### Calculate Expected Results
```bash
python3 manual_trace.py
# Output: test_data/expected_results.txt
# Summary: 1 HIT, 14 MISSES, 6.67% hit rate
```

### Parse C++ Output
```bash
python3 parse_cpp_output.py test_data/trace.txt [optional_binary_path]
# Output: test_data/cpp_results.txt
```

### Validate
```bash
python3 validate.py
# Output: test_data/validation_report.txt
# Shows: PASSED ✅ or FAILED ❌
```

---

## 🎯 Expected Results (Reference)

```
Access #  Address  Block  Result
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1         0x0000   0      MISS
2         0x0040   1      MISS
3         0x0080   2      MISS
4         0x00C0   3      MISS
5         0x0000   0      HIT  ← Re-access
6         0x0100   4      MISS ← Evicts Block 1
7         0x0040   1      MISS ← Block 1 reloaded
8         0x0080   2      MISS
9         0x00C0   3      MISS
10        0x0140   5      MISS ← Evicts Block 4
11        0x0180   6      MISS ← Evicts Block 2
12        0x01C0   7      MISS ← Evicts Block 3
13        0x0000   0      MISS ← Block 0 evicted
14        0x0040   1      MISS ← Block 1 evicted
15        0x0100   4      MISS ← Block 4 evicted

TOTALS: 1 HIT, 14 MISSES, 6.67% HIT RATE
```

---

## 🔍 Cache Configuration

- **Total Size:** 256 bytes
- **Block Size:** 64 bytes (6-bit offset)
- **Associativity:** 4-way
- **Number of Sets:** 1 (0-bit index)
- **Tag Bits:** Remaining (addr >> 6)

### Address Decoding Example
```
Address: 0x0000 = 0b000000000000
Offset:  [5:0]   = 0b000000 = 0
Index:   (none)  = 0
Tag:     [11:6]  = 0b000000 = 0
Block:   addr >> 6 = 0

Address: 0x0100 = 0b000100000000
Offset:  [5:0]   = 0b000000 = 0
Index:   (none)  = 0
Tag:     [11:6]  = 0b000100 = 4
Block:   addr >> 6 = 4
```

---

## 📂 File Locations

```
validation/
├── generate_trace.py          # Create test pattern
├── manual_trace.py            # Calculate ground truth
├── parse_cpp_output.py        # Extract C++ results
├── validate.py                # Compare
├── run_validation.sh          # Run all
├── README.md                  # Full documentation
├── PHASE3_SUMMARY.md          # Implementation details
├── QUICK_REFERENCE.md         # This file
└── test_data/
    ├── trace.txt              # Generated (15 addresses)
    ├── expected_results.txt    # Ground truth
    ├── cpp_results.txt         # C++ output
    └── validation_report.txt   # Comparison
```

---

## ✅ Validation Checklist

- [ ] `trace.txt` generated (15 addresses)
- [ ] `expected_results.txt` generated (1 HIT, 14 MISS)
- [ ] C++ binary compiled (`test_cache`)
- [ ] C++ binary runs with trace file argument
- [ ] C++ outputs results in parseable format
- [ ] `cpp_results.txt` generated
- [ ] `validation_report.txt` shows ✅ PASSED

---

## 🚨 Common Issues

| Problem | Solution |
|---------|----------|
| Binary not found | Compile: `cmake -B build && cmake --build build` |
| Parse error | Edit `parse_cpp_output.py` to match your output format |
| Wrong hit rate | Check address decoding and LRU logic in C++ |
| Off-by-one errors | Verify way indexing (0-3 vs 1-4) |
| Cache state mismatch | Check which block is in which way |

---

## 🎓 What Each Access Tests

| Access | Purpose | Expected |
|--------|---------|----------|
| 1-4 | Fill cache (4 ways) | All MISS |
| 5 | Re-access first | HIT |
| 6 | Evict LRU | MISS |
| 7 | Reload evicted | MISS |
| 8-9 | More accesses | MISS |
| 10-12 | Multiple evictions | MISS |
| 13-15 | Verify evictions | MISS |

---

## 🔧 Customizing

### Add More Addresses to Test
Edit `generate_trace.py`:
```python
trace.append(0x0200)  # New address
trace.append(0x0000)  # Re-access (should HIT if in cache)
```

Then re-run:
```bash
python3 generate_trace.py && python3 manual_trace.py
```

### Check Intermediate Files
```bash
cat test_data/trace.txt              # See addresses
cat test_data/expected_results.txt   # See ground truth
cat test_data/validation_report.txt  # See comparison
```

---

## 📊 Expected Flow

```
generate_trace.py
    ↓ (creates trace.txt)
manual_trace.py
    ↓ (creates expected_results.txt)
parse_cpp_output.py
    ↓ (runs C++ simulator, creates cpp_results.txt)
validate.py
    ↓ (compares, creates validation_report.txt)
    ↓
✅ PASSED or ❌ FAILED
```

---

**Ready to validate? Run:**
```bash
./run_validation.sh
```

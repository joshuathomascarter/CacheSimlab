# JOSH CARTER — MEMORY ARCHITECT MASTER PLAN
## February 8, 2026 → Apple Internship Application (October 2026)
## 10 Hours/Day | Every File Referenced | Every Minute Accounted For

---

# ARCHITECTURAL PHILOSOPHY: ONE PROJECT, NOT FORTY-THREE

This plan looks like 43 separate projects. It's not. It's **ONE growing codebase** from Day 1.

Everything lives in a single repository: `memory-architect`

```
memory-architect/         ← ONE repo, ONE CMake build system
├── CMakeLists.txt        ← Top-level build (add modules as you build them)
├── src/
│   ├── core/             ← Types, config loader, trace reader (Day 1)
│   ├── cache/            ← Direct-mapped → 4-way → MSHR → prefetch (Days 1-6)
│   ├── dram/             ← Bank FSM → scheduler → power → multi-channel (Days 1-2, Month 2)
│   ├── coherence/        ← MESI → MOESI (Days 3-4)
│   ├── hierarchy/        ← Connects cache + dram + coherence (Day 7-8)
│   ├── tlb/              ← TLB + page walker (Day 20)
│   ├── noc/              ← Ring → mesh (Month 2 Week 8)
│   ├── soc/              ← CPU+GPU+NPU traffic + QoS (Month 3)
│   ├── reliability/      ← ECC, scrubbing (Month 2 Week 7)
│   ├── security/         ← RowHammer defenses (Month 4)
│   ├── emerging/         ← STT-MRAM, PCM, hybrid (Month 5)
│   ├── compression/      ← BDI, bandwidth compression (Month 6)
│   ├── cxl/              ← CXL pooling + fabric (Month 8)
│   └── pim/              ← Processing-in-memory (Month 3 Week 11)
├── rtl/
│   ├── cache_controller.sv       ← Day 9: direct-mapped
│   ├── set_assoc_cache.sv        ← Day 15: 4-way
│   ├── mem_controller.sv         ← Day 16: basic
│   ├── mem_controller_ddr5.sv    ← Month 9: DDR5 + formal
│   ├── fpga/                     ← PYNQ Z2 synthesis projects
│   └── formal/                   ← SVA properties + SymbiYosys
├── python/
│   ├── bindings/                 ← pybind11 (Month 7)
│   ├── dashboard/                ← Visualization (Month 7)
│   ├── analysis/                 ← All analysis scripts
│   └── pynq/                     ← PYNQ Z2 Jupyter notebooks + overlays
├── tests/                        ← ALL tests, organized by module
├── docs/                         ← All reports and analysis documents
└── .github/workflows/ci.yml     ← CI from Month 7 onward
```

**The rule:** Every day's work is `git commit` to this repo. Every module is a new subdirectory under `src/` or `rtl/`. The Month 7 "MemSim" project is just adding a unified API + Python bindings + dashboard to what's already been growing for 6 months. You don't "start" MemSim in Month 7 — you've been building it since Day 1.

**PYNQ Z2 FPGA Board:** When your board arrives, you'll deploy your RTL to real hardware. Running your cache controller on an actual FPGA and comparing against your C++ sim IN HARDWARE is worth more than any amount of Verilator simulation. Details in Month 3 Week 12 and Month 9.

---

# LANGUAGE SPLIT TARGET

```
Language           Lines        %      Role
─────────────────────────────────────────────────────────────────
C++                25,000-30,000  55%   Core simulation engine, all models
SystemVerilog      6,000-9,000    17%   RTL designs + FPGA deployment
Python             8,000-10,000   19%   Analysis, pybind11 bindings, PYNQ notebooks, dashboard
Tcl/Constraints    500-1,000      2%    FPGA synthesis constraints (Vivado XDC files)
Makefile/CMake     500-800        2%    Build system
Markdown           2,000-3,000    5%    Reports, documentation
─────────────────────────────────────────────────────────────────
Total              ~42,000-54,000
```

The SystemVerilog percentage is higher than a typical architecture student because of your PYNQ Z2 FPGA work. That's intentional — Apple hires architects who can also write RTL.

---

# WHAT YOU'VE ALREADY BUILT (Your Foundation)

## ✅ Completed Projects — Detailed Inventory

### 1. Direct-Mapped Cache Simulator
**Location:** `/cache sim/direct-way/`
- `direct_mapped_cache.cpp` + `.h` — Full cache with tag/index/offset parsing
- `cache_line.h` — Valid bit, tag, data storage
- `test_direct_mapped.cpp` — Test harness with hit/miss verification
- `random_trace.py`, `sequential_trace.py` — Trace generators
- **What you proved:** You understand address decomposition, cache lookup, conflict misses
- **Gap:** No write policy (write-back vs write-through), no dirty bit tracking

### 2. 4-Way Set-Associative Cache
**Location:** `/cache sim/4-way cache/`
- `set_associative_cache.cpp/.h` — 4-way with configurable sets
- `cache_set.h` — Set abstraction with way selection
- `lru_tracker.h` — LRU eviction tracking
- `eviction_policies.cpp/.hpp` — LRU, FIFO, Random policies implemented
- `test_eviction_policies.cpp` — Comparative eviction testing
- `eviction_policies.py` — Python cross-validation
- `reuse_distance.py` — Stack distance analysis
- `working_set.py` — Working set size estimation
- `validate.py` — C++ vs Python cross-validation
- **What you proved:** Associativity tradeoffs, eviction policy impact, cross-language validation
- **Gap:** No MSHR (Miss Status Holding Register), no non-blocking behavior, no prefetching

### 3. SRAM Behavioral Model + RTL
**Location:** `/cache sim/sram_array/`
- `sram_behavioral_model.cpp/.h` — Cycle-accurate SRAM model
- `sram_array.v` — Verilog SRAM array
- `sram_array.sv` — SystemVerilog version
- `sram_array_explicit.v` — Gate-level explicit version
- `sram_controller.sv/.v` — Controller with read/write FSM
- `tb_sram_controller.cpp` — C++ testbench
- `tb_sram_controller.sv` — SystemVerilog testbench
- `cross_validate.py` — RTL vs behavioral cross-validation
- `sram_test_vectors.csv` — Golden test vectors
- **What you proved:** RTL design, FSM design, behavioral-to-RTL validation flow
- **Gap:** No timing model (setup/hold), no power estimation in RTL, no multi-port

### 4. DRAM Simulator (Most Advanced Project)
**Location:** `/dram/`
- `dram_bank_fsm.cpp/.h` — Bank state machine (IDLE→ACTIVATING→ACTIVE→PRECHARGING→REFRESHING)
- `memory_channel.cpp/.h` — Channel-level coordination
- `memory_scheduler.cpp/.h` — FCFS + FR-FCFS scheduling
- `power_model.cpp/.h` — IDD current-based power calculation
- `refresh_controller.cpp/.h` — Refresh scheduling (per-bank, all-bank)
- `timing_validator.cpp/.h` — tRCD, tCAS, tRP, tRFC enforcement
- `test_bank_fsm.cpp` — Bank FSM verification
- `test_refresh_power.cpp` — Refresh power analysis
- `test_scheduling.cpp` — Scheduler comparison
- `test_3d_stacking.cpp` — 3D/HBM modeling
- `example_hybrid_usage.cpp` — Hybrid scheduler demo
- `generate_power_trace.cpp` — Power trace generation
- `dram_timing_viz.py` — Timing diagram visualization
- `power_analyzer.py` — Power breakdown analysis
- `scheduling_analyzer.py` — Scheduling policy comparison
- `HYBRID_SCHEDULER_GUIDE.md` — Documentation
- **What you proved:** Full DRAM subsystem understanding, timing constraints, power modeling, scheduling tradeoffs
- **Gap:** No address mapping policies, no QoS/fairness, no multi-channel, no HBM channel architecture

### 5. Interview Prep Materials
**Location:** `/interview questions/`
- `dram/question1-5_deep_dive.md` — 5 DRAM architecture deep dives
- `sram/sram_comprehensive_deep_dive.md` — SRAM physics + architecture reference
- **Status:** DRAM questions not yet completed (do these first)

---

# PHASE 1: FOUNDATION COMPLETION
## February 8 – February 28, 2026 (21 days, 210 hours)

---

## DAY 1 — February 8 (Saturday)
### Theme: Complete DRAM Interview Questions 1-3

**WHY THIS DAY MATTERS:**
These questions force you to articulate what you built. You coded the DRAM simulator but can you EXPLAIN every design decision? An Apple interviewer will ask "walk me through your bank FSM" and you need to answer in 60 seconds, not fumble.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-8:30 | 30 min | **Morning Review** | Re-read your `dram_bank_fsm.cpp`. Draw the FSM by hand on paper without looking at code. Can you get all 5 states and transitions? |
| 8:30-10:00 | 90 min | **Question 1 Deep Dive** | Open `question1_deep_dive.md`. Write your answer. Reference your actual code: "In my implementation at line X of `dram_bank_fsm.cpp`, I handled this by..." |
| 10:00-10:15 | 15 min | Break | |
| 10:15-11:45 | 90 min | **Question 2 Deep Dive** | Same approach. Cross-reference with `memory_scheduler.cpp`. Trace through a FR-FCFS scheduling decision with a concrete example: 3 requests, 2 row hitow miss. |
| 11:45-12:30 | 45 min | **Lunch + Walk** | Think about Question 3 while walking. No screens. |
| 12:30-14:00 | 90 min | **Question 3 Deep Dive** | Reference `refresh_controller.cpp`. Calculate: for 8Gb DRAM with 8192 rows, tREFI = 7.8μs, tRFC = 350ns — what % of bandwidth is lost to refresh? |
| 14:00-14:15 | 15 min | Break | |
| 14:15-15:45 | 90 min | **Code Enhancement** | Add address mapping to your DRAM sim. Your current code doesn't decompose addresses into row/bank/column. Add this to `memory_channel.cpp`: |
| | | | - `void decode_address(uint64_t addr, int& channel, int& rank, int& bank, int& row, int& col)` |
| | | | - Support 3 schemes: row-bank-column, bank-row-column, row-interleaved |
| 15:45-16:00 | 15 min | Break | |
| 16:00-17:00 | 60 min | **Test Your Enhancement** | Write test in `tests/` that sends 1000 sequential addresses through each mapping scheme. Measure row buffer hit rate for each. Sequential access should show ~95% hit rate for row-bank-column but ~12% for bank-row-column. |
| 17:00-18:00 | 60 min | **Theory Notebook** | Handwrite (not type) answers to: |
| | | | 1. Why does address mapping affect performance? (Draw the bit fields) |
| | | | 2. What is the row buffer hit rate formula? |
| | | | 3. Why does bank interleaving help streaming workloads? |
| | | | 4. Draw timing diagram: back-to-back reads to same row vs different rows |

**Day 1 Deliverables:**
- [ ] Questions 1-3 answered with code references
- [ ] Address mapping added to DRAM sim (3 schemes)
- [ ] Test showing hit rate difference between schemes
- [ ] 4 handwritten notebook pages

---

## DAY 2 — February 9 (Sunday)
### Theme: Complete DRAM Interview Questions 4-5 + QoS

**WHY THIS DAY MATTERS:**
Questions 4-5 cover power and 3D stacking — the two hottest topics in memory right now. Apple's M-series chips use LPDDR with aggressive power management. HBM is everywhere in AI. You need to speak fluently about both.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-8:30 | 30 min | **Morning Review** | Re-read your `power_model.cpp`. List all IDD parameters you modeled. Can you explain IDD0 vs IDD4R vs IDD5 without looking? |
| 8:30-10:00 | 90 min | **Question 4 Deep Dive** | Power modeling. Reference `power_model.cpp` and `generate_power_trace.cpp`. Calculate power for: idle DRAM (IDD2N), streaming read (IDD4R), refresh burst (IDD5). Show the ratio. |
| 10:00-10:15 | 15 min | Break | |
| 10:15-11:45 | 90 min | **Question 5 Deep Dive** | 3D stacking. Reference `test_3d_stacking.cpp`. Compare: DDR5 (1 channel, 2 ranks) vs HBM3 (16 channels, pseudo-channels). Calculate bandwidth: DDR5 = 4800MT/s × 8B = 38.4 GB/s. HBM3 = 16 channels × 2 pseudo × 3.2GT/s × 32B = 3.28 TB/s. |
| 11:45-12:30 | 45 min | **Lunch + Walk** | |
| 12:30-14:30 | 120 min | **Build: QoS Memory Controller** | Your scheduler treats all requests equally. Real controllers prioritize. Add to `memory_scheduler.cpp`: |
| | | | - Request priority levels: CRITICAL (LLC miss), HIGH (prefetch), LOW (writeback) |
| | | | - `struct MemoryRequest { ...; Priority priority; int source_id; }` |
| | | | - Starvation prevention: if LOW priority request waits > 100 cycles, promote to HIGH |
| | | | - Per-source fairness: track bandwidth per source_id, throttle sources exceeding fair share |
| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Test QoS** | Create test: 4 cores, Core 0 = streaming (row hits), Core 1-3 = random. Without QoS, Core 0 hogs bandwidth. With QoS, each core gets ~25% ± 5%. Measure and print results. |
| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Write Report** | In `output/reports/`, create `QOS_ANALYSIS.md`: |
| | | | - Table: bandwidth per core with/without QoS |
| | | | - Graph data (even text-based): latency distribution per core |
| | | | - Design decision: why you chose the starvation threshold |
| 17:30-18:00 | 30 min | **Theory Notebook** | Handwrite: |
| | | | 1. Draw HBM3 stack (base die + 8 DRAM dies + TSVs) |
| | | | 2. Why is HBM bandwidth so much higher than DDR? |
| | | | 3. What is thermal throttling and why does 3D stacking make it worse? |

**Day 2 Deliverables:**
- [ ] Questions 4-5 answered with code references
- [ ] QoS scheduling added to DRAM sim
- [ ] QoS test showing fairness improvement
- [ ] QOS_ANALYSIS.md report
- [ ] 3 handwritten notebook pages

---

## DAY 3 — February 10 (Monday)
### Theme: Cache Coherence — MESI Protocol Simulator

**WHY THIS DAY MATTERS:**
Your caches work independently. Real systems have 4-16 cores sharing data. Without coherence, Core 0 writes to address X, Core 1 reads stale data from its cache. This is THE fundamental multicore problem. Apple's M4 has 12 cores — coherence is critical. Every memory architect interview asks about this.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:00 | 60 min | **Theory: MESI Protocol** | Study these 4 states until you can draw the full state diagram from memory: |
| | | | - **M (Modified):** Only copy, dirty. Must write back before anyone else reads. |
| | | | - **E (Exclusive):** Only copy, clean. Can silently transition to M on write. |
| | | | - **S (Shared):** Multiple copies exist, all clean. Must invalidate others to write. |
| | | | - **I (Invalid):** Not in cache. Must snoop/request on any access. |
| | | | Draw ALL transitions: PrRd, PrWr, BusRd, BusRdX, BusUpgr, Flush |
| 9:00-9:15 | 15 min | Break | |
| 9:15-11:15 | 120 min | **Build: MESI Cache Line** | Create new project: `/cache sim/coherence/` |

```cpp
// New file structure:
// coherence/
//   include/
//     mesi_cache.h
//     coherence_bus.h  
//     coherence_controller.h
//   cpp/
//     mesi_cache.cpp
//     coherence_bus.cpp
//     coherence_controller.cpp
//   tests/
//     test_mesi.cpp
//   Makefile

// mesi_cache.h — Design this:
enum class MESIState { MODIFIED, EXCLUSIVE, SHARED, INVALID };

struct CoherentCacheLine {
    bool valid;
    uint64_t tag;
    MESIState state;
    uint64_t data;  // simplified to single word
    bool dirty() const { return state == MESIState::MODIFIED; }
};

class MESICache {
    int cache_id;
    int num_sets;
    int associativity;
    std::vector<std::vector<CoherentCacheLine>> sets;
    
    // Returns: hit/miss, old state, new state, bus transaction needed
    struct AccessResult {
        bool hit;
        MESIState old_state;
        MESIState new_state;
        BusTransaction bus_txn;  // NONE, BUS_RD, BUS_RDX, BUS_UPGR, FLUSH
    };
    
    AccessResult read(uint64_t addr);
    AccessResult write(uint64_t addr);
    void snoop(BusTransaction txn, uint64_t addr);  // React to other caches
};
```

| 11:15-11:30 | 15 min | Break | |
| 11:30-12:15 | 45 min | **Lunch** | |
| 12:15-14:15 | 120 min | **Build: Coherence Bus** | The bus connects all caches. When one cache misses, it broadcasts on the bus. Other caches snoop and respond. |

```cpp
// coherence_bus.h — Design this:
enum class BusTransaction { NONE, BUS_RD, BUS_RDX, BUS_UPGR, FLUSH, DATA };

class CoherenceBus {
    std::vector<MESICache*> caches;  // All connected caches
    int total_bus_transactions = 0;
    int total_invalidations = 0;
    int total_data_transfers = 0;  // Cache-to-cache transfers
    
    // Core i issues a bus transaction
    // Bus asks all OTHER caches to snoop
    // Returns: did any cache supply data? (intervention)
    bool broadcast(int source_cache_id, BusTransaction txn, uint64_t addr);
    
    void print_stats();
};
```

| 14:15-14:30 | 15 min | Break | |
| 14:30-16:30 | 120 min | **Build: State Transitions** | Implement the FULL MESI state machine. This is the core logic: |

```
// MESI Transition Table (implement ALL of these):
//
// Current State | Event      | Next State | Bus Action
// --------------|------------|------------|------------
// I             | PrRd       | E or S     | BusRd (E if no other copy, S if shared)
// I             | PrWr       | M          | BusRdX (read + invalidate others)
// E             | PrRd       | E          | None (silent hit)
// E             | PrWr       | M          | None (silent upgrade!)
// S             | PrRd       | S          | None (hit)
// S             | PrWr       | M          | BusUpgr (invalidate others, no data needed)
// M             | PrRd       | M          | None (hit)
// M             | PrWr       | M          | None (hit)
//
// SNOOP responses (when OTHER cache does something):
// E             | BusRd      | S          | Supply data (intervention)
// E             | BusRdX     | I          | Supply data + invalidate self
// S             | BusRd      | S          | Memory supplies data (or shared intervention)
// S             | BusRdX     | I          | Invalidate self
// S             | BusUpgr    | I          | Invalidate self
// M             | BusRd      | S          | Flush data to bus + memory (writeback)
// M             | BusRdX     | I          | Flush data to requestor + invalidate self
```

| 16:30-16:45 | 15 min | Break | |
| 16:45-18:00 | 75 min | **Test: Classic Coherence Scenarios** | |

```cpp
// test_mesi.cpp — Test these EXACT scenarios:

// Scenario 1: Read sharing
// Core 0 reads addr 0x1000 → Miss → BusRd → E
// Core 1 reads addr 0x1000 → Miss → BusRd → Core 0: E→S, Core 1: I→S
// Verify: Both in S state, 2 bus transactions

// Scenario 2: Write invalidation  
// Core 0 reads addr 0x2000 → E
// Core 1 reads addr 0x2000 → Both S
// Core 0 writes addr 0x2000 → BusUpgr → Core 0: S→M, Core 1: S→I
// Core 1 reads addr 0x2000 → BusRd → Core 0: M→S (flush), Core 1: I→S
// Verify: 4 bus transactions, 1 flush, 1 invalidation

// Scenario 3: False sharing (CRITICAL interview topic)
// Core 0 writes addr 0x3000 → M
// Core 1 writes addr 0x3004 → BusRdX → Core 0 flushes, goes I. Core 1: M
// Core 0 writes addr 0x3000 → BusRdX → Core 1 flushes, goes I. Core 0: M
// This THRASHES even though they write DIFFERENT words!
// Verify: Ping-pong pattern, 3+ bus transactions for 3 writes

// Scenario 4: Producer-consumer
// Core 0 writes addr 0x4000 (produce) → M
// Core 1 reads addr 0x4000 (consume) → Core 0: M→S, flush. Core 1: S
// Repeat 100 times. Count total bus transactions.
// Verify: 200 bus transactions (100 BusRdX + 100 BusRd)
```

**Day 3 Deliverables:**
- [ ] MESI state diagram drawn by hand (photograph it)
- [ ] `mesi_cache.cpp` — Full MESI state machine
- [ ] `coherence_bus.cpp` — Bus with snooping
- [ ] All 4 test scenarios passing
- [ ] Print: total bus transactions, invalidations, data transfers per scenario

---

## DAY 4 — February 11 (Tuesday)
### Theme: Cache Coherence Analysis + False Sharing Deep Dive

**WHY THIS DAY MATTERS:**
Yesterday you built the mechanics. Today you analyze the PERFORMANCE implications. This is where architecture thinking matters — not "does it work?" but "how fast is it and what are the bottlenecks?"

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Coherence Traffic Analysis** | Add metrics to your MESI simulator: |
| | | | - Bus utilization: what % of cycles is the bus busy? |
| | | | - Invalidation rate: invalidations per 1000 accesses |
| | | | - Data sharing degree: avg number of sharers per line |
| | | | - Write-back count: how many dirty evictions? |
| 9:30-9:45 | 15 min | Break | |
| 9:45-11:45 | 120 min | **False Sharing Experiment** | Build a detailed false sharing test: |

```cpp
// Create this workload generator:
struct CacheLineAccess {
    int core_id;
    enum Type { READ, WRITE } type;
    uint64_t addr;
};

// Workload 1: TRUE sharing (same word)
// All cores read/write addr 0x1000
// Expected: high coherence traffic, unavoidable

// Workload 2: FALSE sharing (different words, same cache line)  
// Core 0: writes 0x1000 (bytes 0-7 of cache line)
// Core 1: writes 0x1008 (bytes 8-15 of same cache line)
// Core 2: writes 0x1010 (bytes 16-23 of same cache line)  
// Core 3: writes 0x1018 (bytes 24-31 of same cache line)
// Expected: SAME coherence traffic as true sharing! That's the problem.

// Workload 3: No sharing (different cache lines)
// Core 0: writes 0x1000
// Core 1: writes 0x1040 (next cache line, assuming 64B lines)
// Core 2: writes 0x1080
// Core 3: writes 0x10C0
// Expected: ZERO coherence traffic

// Run 10,000 accesses of each. Compare:
// | Workload      | Bus Txns | Invalidations | Avg Latency |
// |---------------|----------|---------------|-------------|
// | True sharing  | ???      | ???           | ???         |
// | False sharing | ???      | ???           | ???         |
// | No sharing    | ???      | ???           | ???         |
```

| 11:45-12:30 | 45 min | **Lunch** | |
| 12:30-14:00 | 90 min | **Build: MOESI Extension** | Apple uses MOESI (adds Owned state). Extend your simulator: |
| | | | - **O (Owned):** Dirty data, but shared. Owner must supply data on snoop, not memory. |
| | | | - Why? Avoids writing back to memory when sharing dirty data. Saves memory bandwidth. |
| | | | - Add O state to your FSM. When M receives BusRd: M→O (not M→S with writeback). |
| | | | - The O copy is responsible for supplying data to future BusRd requests. |
| 14:00-14:15 | 15 min | Break | |
| 14:15-15:45 | 90 min | **MOESI vs MESI Comparison** | Run same workloads on both. Measure: |
| | | | - Memory writebacks: MOESI should have FEWER (O state avoids writeback) |
| | | | - Bus data transfers: Should be SAME |
| | | | - Memory bandwidth saved: Calculate in bytes |
| | | | - Create table comparing both protocols |
| 15:45-16:00 | 15 min | Break | |
| 16:00-17:00 | 60 min | **Write: Coherence Analysis Report** | Create `coherence/COHERENCE_ANALYSIS.md`: |
| | | | - False sharing results table |
| | | | - MESI vs MOESI comparison |
| | | | - "If I were designing Apple M5's coherence, I'd choose MOESI because..." |
| | | | - Diagram: when does each protocol win? |
| 17:00-18:00 | 60 min | **Theory Notebook** | Handwrite: |
| | | | 1. Full MESI state diagram from memory (no cheating) |
| | | | 2. Why false sharing is worse than true sharing (hint: it's avoidable) |
| | | | 3. Directory-based coherence: why snooping doesn't scale beyond ~8 cores |
| | | | 4. What is a snoop filter? Why does Apple need one? |

**Day 4 Deliverables:**
- [ ] False sharing experiment with quantified results
- [ ] MOESI extension working
- [ ] MESI vs MOESI comparison table
- [ ] `COHERENCE_ANALYSIS.md` report
- [ ] 4 handwritten notebook pages

---

## DAY 5 — February 12 (Wednesday)
### Theme: Non-Blocking Cache + MSHRs

**WHY THIS DAY MATTERS:**
Your 4-way cache stalls on every miss. Real caches allow hits during misses (hit-under-miss). This requires MSHRs (Miss Status Holding Registers) — a queue that tracks outstanding misses. Apple's cores can handle 10+ outstanding misses simultaneously. This is what separates a textbook cache from a real one.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:00 | 60 min | **Theory: Why Non-Blocking?** | Study the performance impact: |
| | | | - Blocking cache: miss → stall pipeline for ~100 cycles (DRAM latency) |
| | | | - Non-blocking: miss → record in MSHR → keep servicing hits |
| | | | - Hit-under-miss: 1 outstanding miss allowed |  
| | | | - Miss-under-miss: N outstanding misses (N = MSHR count) |
| | | | - Modern cores: 10-16 MSHRs per L1, 20-32 per L2 |
| 9:00-9:15 | 15 min | Break | |
| 9:15-11:15 | 120 min | **Build: MSHR Structure** | Add to your 4-way cache: |

```cpp
// New file: /cache sim/4-way cache/include/mshr.h

struct MSHREntry {
    bool valid;
    uint64_t addr;          // Address of outstanding miss
    uint64_t block_addr;    // Cache-line aligned address
    int target_set;         // Which set this will fill
    int target_way;         // Which way (chosen by eviction policy)
    uint64_t cycle_issued;  // When was miss sent to memory?
    uint64_t cycle_expected; // When will data arrive? (cycle_issued + memory_latency)
    
    // Secondary misses: other requests to SAME cache line
    struct SecondaryTarget {
        uint64_t addr;       // Exact address (may differ within same line)
        bool is_write;
    };
    std::vector<SecondaryTarget> secondary_targets;
};

class MSHRFile {
    std::vector<MSHREntry> entries;
    int capacity;  // e.g., 8 MSHRs
    
    // Allocate MSHR for new miss. Returns false if all MSHRs full (structural stall)
    bool allocate(uint64_t addr, int set, int way, uint64_t current_cycle, uint64_t latency);
    
    // Check if address already has an MSHR (secondary miss — piggyback, don't allocate new)
    MSHREntry* lookup(uint64_t block_addr);
    
    // Check if any MSHRs have completed (data arrived)
    std::vector<MSHREntry> check_completions(uint64_t current_cycle);
    
    bool is_full() const;
    int occupancy() const;
};
```

| 11:15-11:30 | 15 min | Break | |
| 11:30-12:15 | 45 min | **Lunch** | |
| 12:15-14:15 | 120 min | **Integrate MSHRs into Cache** | Modify your `set_associative_cache.cpp`: |
| | | | - On miss: check MSHR for same block → if found, add secondary target |
| | | | - On miss: if no existing MSHR, allocate new one → if MSHRs full, STALL |
| | | | - Every cycle: check completions → fill cache line → wake up all secondary targets |
| | | | - Track: total stalls due to MSHR full (structural hazard) |
| 14:15-14:30 | 15 min | Break | |
| 14:30-16:00 | 90 min | **Performance Experiment** | Compare blocking vs non-blocking: |

```cpp
// Test workload: strided access with temporal reuse
// for (int i = 0; i < 1000; i++)
//   for (int j = 0; j < 8; j++)  
//     access(base + j * CACHE_LINE_SIZE + i * STRIDE)
//
// Blocking cache: every miss stalls for 100 cycles
// Non-blocking (8 MSHRs): can overlap up to 8 misses
//
// Measure:
// | Config           | Total Cycles | Speedup | MSHR Full Stalls |
// |------------------|-------------|---------|------------------|
// | Blocking         | ???         | 1.0x    | N/A              |
// | 2 MSHRs          | ???         | ???     | ???              |
// | 4 MSHRs          | ???         | ???     | ???              |
// | 8 MSHRs          | ???         | ???     | ???              |
// | 16 MSHRs         | ???         | ???     | ???              |
// | 32 MSHRs         | ???         | ???     | ???              |
//
// Expected: diminishing returns after 8-10 MSHRs (memory bandwidth saturates)
```

| 16:00-16:15 | 15 min | Break | |
| 16:15-17:15 | 60 min | **Secondary Miss Analysis** | Add tracking for: |
| | | | - Primary misses (first access to a block) |
| | | | - Secondary misses (subsequent accesses to same block while MSHR active) |
| | | | - Secondary misses are FREE — they piggyback on the primary miss |
| | | | - Workloads with high spatial locality → many secondary misses → big speedup |
| 17:15-18:00 | 45 min | **Theory Notebook** | Handwrite: |
| | | | 1. Draw MSHR pipeline: miss → allocate → wait → complete → fill → wake |
| | | | 2. Why is "MSHR full" a structural hazard? What happens architecturally? |
| | | | 3. Why 16 MSHRs? Why not 256? (Area cost, diminishing returns, memory bandwidth limit) |

**Day 5 Deliverables:**
- [ ] MSHR structure implemented
- [ ] Non-blocking cache integrated with 4-way cache
- [ ] Performance comparison: blocking vs 2/4/8/16/32 MSHRs
- [ ] Secondary miss tracking and analysis
- [ ] 3 handwritten notebook pages

---

## DAY 6 — February 13 (Thursday)
### Theme: Prefetching — The Key to Memory Performance

**WHY THIS DAY MATTERS:**
Without prefetching, the CPU waits 100+ cycles for every cache miss. With prefetching, data arrives BEFORE the CPU needs it. Apple's cores use sophisticated prefetchers. This is one of the most active research areas in memory architecture and a common interview topic.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:00 | 60 min | **Theory: Prefetch Concepts** | Study: |
| | | | - **Timeliness:** Prefetch too early = evicted before use. Too late = no help. |
| | | | - **Accuracy:** Prefetch wrong address = pollute cache, waste bandwidth. |
| | | | - **Coverage:** What % of misses does prefetcher eliminate? |
| | | | - **Bandwidth overhead:** Useless prefetches consume memory bandwidth. |
| | | | Formula: `Speedup = 1 / (1 - coverage × timeliness_rate)` for memory-bound code |
| 9:00-9:15 | 15 min | Break | |
| 9:15-11:15 | 120 min | **Build: Prefetcher Module** | Create `/cache sim/4-way cache/include/prefetcher.h`: |

```cpp
// Prefetcher interface — all prefetchers implement this
class Prefetcher {
public:
    virtual std::vector<uint64_t> on_access(uint64_t addr, bool is_miss) = 0;
    virtual std::string name() const = 0;
    
    // Metrics
    int prefetches_issued = 0;
    int prefetches_useful = 0;    // Actually used before eviction
    int prefetches_useless = 0;   // Evicted without being accessed
    int prefetches_late = 0;      // Address was accessed before prefetch arrived
    
    double accuracy() const { return (double)prefetches_useful / prefetches_issued; }
    double coverage() const;  // useful / (useful + remaining_misses)
};

// 1. Next-Line Prefetcher (simplest)
class NextLinePrefetcher : public Prefetcher {
    int degree;  // How many lines ahead to prefetch (1, 2, 4)
    std::vector<uint64_t> on_access(uint64_t addr, bool is_miss) override {
        std::vector<uint64_t> prefetches;
        if (is_miss) {
            for (int i = 1; i <= degree; i++)
                prefetches.push_back(addr + i * CACHE_LINE_SIZE);
        }
        return prefetches;
    }
};

// 2. Stride Prefetcher (detects strided access patterns)
class StridePrefetcher : public Prefetcher {
    struct StrideEntry {
        uint64_t last_addr;
        int64_t stride;       // Can be negative
        int confidence;       // 0-3, prefetch only if >= 2
        bool trained;
    };
    std::unordered_map<uint64_t, StrideEntry> table;  // Indexed by PC (instruction address)
    
    // On access: compute stride = addr - last_addr
    // If stride matches previous: confidence++
    // If stride differs: confidence--, update stride
    // If confidence >= 2: prefetch addr + stride, addr + 2*stride, ...
};

// 3. Markov Prefetcher (learns miss address correlations)
class MarkovPrefetcher : public Prefetcher {
    // Table: miss_addr → {next_addr_1: count, next_addr_2: count, ...}
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, int>> correlation_table;
    uint64_t last_miss_addr;
    
    // On miss at addr X: 
    //   1. Record correlation: last_miss_addr → X
    //   2. Look up X in table → prefetch top-N correlated addresses
    //   3. Update last_miss_addr = X
};
```

| 11:15-11:30 | 15 min | Break | |
| 11:30-12:15 | 45 min | **Lunch** | |
| 12:15-14:15 | 120 min | **Build: Stride Prefetcher (Full Implementation)** | This is the most important one. Implement completely with: |
| | | | - PC-indexed stride detection table (256 entries) |
| | | | - Confidence counter (0-3 saturating) |
| | | | - Prefetch degree: 1 at confidence 2, up to 4 at confidence 3 |
| | | | - Prefetch into a separate prefetch buffer (not directly into cache) |
| | | | - On hit in prefetch buffer: move to cache, mark as "useful" |
| 14:15-14:30 | 15 min | Break | |
| 14:30-16:30 | 120 min | **Prefetcher Comparison Experiment** | Create 4 workloads, test all 3 prefetchers: |

```
Workload A: Sequential scan (for i in 0..N: access(base + i*8))
  Expected winner: Next-line (simple pattern, 100% accuracy)

Workload B: Strided access (for i in 0..N: access(base + i*256))  
  Expected winner: Stride prefetcher (next-line can't reach)

Workload C: Linked list traversal (random pointer chasing)
  Expected winner: Markov (IF trained), otherwise NONE work

Workload D: Matrix column access (stride = row_size)
  Expected winner: Stride prefetcher

Results table:
| Workload | No Prefetch | Next-Line | Stride | Markov |
|          | Miss Rate   | Miss Rate | Miss Rate | Miss Rate |
|----------|-------------|-----------|-----------|-----------|
| A        | ???%        | ???%      | ???%      | ???%      |
| B        | ???%        | ???%      | ???%      | ???%      |
| C        | ???%        | ???%      | ???%      | ???%      |
| D        | ???%        | ???%      | ???%      | ???%      |

Also measure: accuracy, coverage, bandwidth overhead for each
```

| 16:30-16:45 | 15 min | Break | |
| 16:45-18:00 | 75 min | **Theory + Write-up** | |
| | | | 30 min: Write `PREFETCH_ANALYSIS.md` with all results |
| | | | 45 min: Handwrite in notebook: |
| | | | 1. Why can't prefetchers help pointer chasing? (Dependent loads) |
| | | | 2. Draw stride prefetcher FSM: No-Prediction → Transient → Steady → No-Prediction |
| | | | 3. What is prefetch timeliness? Draw timeline showing early/on-time/late |
| | | | 4. Apple A-series uses a "next-line prefetcher on L1, stride on L2" — why different levels? |

**Day 6 Deliverables:**
- [ ] Three prefetchers implemented (next-line, stride, Markov)
- [ ] Prefetch buffer (separate from cache)
- [ ] 4-workload comparison with accuracy/coverage/bandwidth metrics
- [ ] `PREFETCH_ANALYSIS.md`
- [ ] 4 handwritten notebook pages

---

## DAY 7 — February 14 (Friday)
### Theme: Full Memory Hierarchy Simulator (L1 → L2 → L3 → DRAM)

**WHY THIS DAY MATTERS:**
You've built individual pieces: caches, DRAM, coherence, prefetching. Today you connect them into a COMPLETE memory hierarchy. This is the project that will make Apple interviewers say "wait, you built WHAT as a Year 1 student?"

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Design: Hierarchy Architecture** | Plan before coding. Draw on paper: |

```
Architecture to build:

Core 0          Core 1          Core 2          Core 3
  |               |               |               |
[L1-I] [L1-D]  [L1-I] [L1-D]  [L1-I] [L1-D]  [L1-I] [L1-D]
  32KB  32KB     32KB  32KB     32KB  32KB     32KB  32KB
  8-way 8-way    8-way 8-way    8-way 8-way    8-way 8-way
  4-cyc 4-cyc    4-cyc 4-cyc    4-cyc 4-cyc    4-cyc 4-cyc
     \   /          \   /          \   /          \   /
    [L2 Unified]  [L2 Unified]  [L2 Unified]  [L2 Unified]
      256KB         256KB         256KB         256KB
      8-way         8-way         8-way         8-way  
      12-cyc        12-cyc        12-cyc        12-cyc
         \            |            |            /
          \           |            |           /
           +---------[L3 Shared]-----------+
                      8MB
                      16-way
                      36 cycles
                      Inclusive (contains copies of L1/L2 data)
                         |
                   [Memory Controller]
                   (your DRAM sim!)
                      ~100 cycles
                         |
                   [DRAM: DDR4-3200]
                   2 channels, 2 ranks, 8 banks
```

| | | | **Parameters to hardcode initially:** |
| | | | L1: 32KB, 8-way, 64B lines, 4-cycle hit, 8 MSHRs |
| | | | L2: 256KB, 8-way, 64B lines, 12-cycle hit, 16 MSHRs |
| | | | L3: 8MB, 16-way, 64B lines, 36-cycle hit, 32 MSHRs |
| | | | DRAM: 100-cycle base latency (use your existing sim) |

| 9:30-9:45 | 15 min | Break | |
| 9:45-11:45 | 120 min | **Build: MemoryHierarchy Class** | |

```cpp
// New project: /memory_hierarchy/
// 
// include/
//   memory_hierarchy.h
//   cache_level.h
// cpp/
//   memory_hierarchy.cpp
//   cache_level.cpp
// tests/
//   test_hierarchy.cpp
//   workload_generator.cpp

// memory_hierarchy.h
class MemoryHierarchy {
    struct CacheConfig {
        int size_bytes;
        int associativity;
        int line_size;
        int hit_latency;
        int mshr_count;
        bool inclusive;        // Does this level contain copies of higher levels?
        Prefetcher* prefetcher;  // Optional prefetcher at this level
    };
    
    std::vector<CacheLevel> levels;  // L1, L2, L3
    MemoryChannel* dram;             // Your existing DRAM sim
    CoherenceBus* bus;               // Your MESI/MOESI bus (for L3 and below)
    
    struct AccessResult {
        int latency_cycles;
        int level_serviced;  // 1=L1 hit, 2=L2 hit, 3=L3 hit, 4=DRAM
        bool prefetch_hit;   // Was this serviced by a prefetch?
    };
    
    AccessResult access(int core_id, uint64_t addr, bool is_write);
    
    // Stats per level
    struct LevelStats {
        uint64_t hits, misses;
        double hit_rate() const { return (double)hits / (hits + misses); }
        uint64_t total_latency;
        double avg_latency() const { return (double)total_latency / (hits + misses); }
    };
    std::vector<LevelStats> stats;
    
    void print_hierarchy_stats();  // Print full hierarchy summary
};
```

| 11:45-12:30 | 45 min | **Lunch** | |
| 12:30-14:30 | 120 min | **Implement Access Path** | The critical logic: |

```
access(core_id, addr, is_write):
  1. Check L1[core_id]
     - Hit? Return (data, 4 cycles)
     - Miss? Allocate MSHR, continue to L2
     
  2. Check L2[core_id]  
     - Hit? Fill L1, return (data, 4 + 12 = 16 cycles)
     - Miss? Allocate MSHR, continue to L3
     
  3. Check L3 (shared)
     - Hit? Fill L2, fill L1, return (data, 4 + 12 + 36 = 52 cycles)
     - Miss? Send to DRAM controller
     
  4. DRAM access
     - Use your memory_scheduler to process
     - Fill L3, L2, L1
     - Return (data, 4 + 12 + 36 + ~100 = ~152 cycles)

  For INCLUSIVE L3:
  - When L3 evicts a line, must ALSO invalidate from L1/L2 (back-invalidation)
  - This is expensive but guarantees L3 is superset of L1+L2
```

| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Workload Generator + First Results** | |

```cpp
// workload_generator.cpp
enum class WorkloadType {
    SEQUENTIAL_SCAN,     // Stream through memory
    RANDOM_ACCESS,       // Uniform random addresses
    WORKING_SET,         // Repeated access to fixed-size working set
    PRODUCER_CONSUMER,   // Core 0 writes, Core 1 reads
    MATRIX_MULTIPLY,     // Row-major × column-major (tests spatial locality)
};

// Generate 100K accesses for each workload
// Run through hierarchy
// Print per-level stats:

// Expected output:
// ┌─────────────────────────────────────────────────────────┐
// │ Workload: SEQUENTIAL_SCAN (100K accesses)               │
// ├─────────┬──────────┬──────────┬──────────┬──────────────┤
// │ Level   │ Accesses │ Hits     │ Hit Rate │ Avg Latency  │
// ├─────────┼──────────┼──────────┼──────────┼──────────────┤
// │ L1 (32K)│ 100000   │ 87500   │ 87.5%    │ 4.0 cyc      │
// │ L2(256K)│ 12500    │ 12400   │ 99.2%    │ 12.0 cyc     │
// │ L3 (8M) │ 100      │ 98      │ 98.0%    │ 36.0 cyc     │
// │ DRAM    │ 2        │ N/A     │ N/A      │ 105.3 cyc    │
// ├─────────┼──────────┼──────────┼──────────┼──────────────┤
// │ Overall │          │          │          │ 5.6 cyc      │
// └─────────┴──────────┴──────────┴──────────┴──────────────┘
```

| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Working Set Size Sweep** | This is the KEY experiment: |

```
Vary working set size, measure average access latency:

Working Set Size    Fits In    Expected Avg Latency
─────────────────────────────────────────────────────
4 KB               L1         ~4 cycles
16 KB              L1         ~4 cycles
32 KB              L1 edge    ~5 cycles (some misses)
64 KB              L2         ~8 cycles
256 KB             L2 edge    ~12 cycles
512 KB             L3         ~18 cycles
4 MB               L3         ~25 cycles
8 MB               L3 edge    ~35 cycles
32 MB              DRAM       ~60+ cycles

Plot this (even as text): it should show STAIRCASE pattern
with jumps at each cache boundary. This is the most important
graph in computer architecture.
```

| 17:30-18:00 | 30 min | **Theory Notebook** | Handwrite: |
| | | | 1. Draw the latency staircase from your results |
| | | | 2. Why is the L1→L2 jump smaller than L3→DRAM? |
| | | | 3. What is inclusion and why does Apple use it? |
| | | | 4. If you could add 1MB to any level, where would you add it? Why? |

**Day 7 Deliverables:**
- [ ] Full memory hierarchy simulator (L1→L2→L3→DRAM)
- [ ] Connected to your existing DRAM sim
- [ ] 5 workloads tested with per-level stats
- [ ] Working set size sweep showing latency staircase
- [ ] Architecture diagram (hand-drawn, photograph)
- [ ] 4 handwritten notebook pages

---

## DAY 8 — February 15 (Saturday)
### Theme: Memory Hierarchy Analysis + Optimization

**WHY THIS DAY MATTERS:**
Yesterday you built it. Today you become the architect — analyzing results, finding bottlenecks, making design decisions. This is what Jim Keller does: looks at data, finds the bottleneck, proposes a solution, quantifies the improvement.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Bottleneck Analysis** | Run all 5 workloads from Day 7. For each, identify: |
| | | | - Which level has the lowest hit rate? |
| | | | - Where is most latency spent? (L3 miss penalty dominates even if rare) |
| | | | - Calculate: AMAT = L1_hit_time + L1_miss_rate × (L2_hit_time + L2_miss_rate × (L3_hit_time + L3_miss_rate × DRAM_time)) |
| | | | - Verify your simulator's average latency matches AMAT formula |
| 9:30-9:45 | 15 min | Break | |
| 9:45-11:45 | 120 min | **Design Space Exploration** | Sweep these parameters: |

```
Experiment 1: L1 size (8KB, 16KB, 32KB, 64KB, 128KB)
  - Keep L2=256KB, L3=8MB
  - Plot: AMAT vs L1 size for each workload
  - Find: point of diminishing returns

Experiment 2: L2 associativity (2-way, 4-way, 8-way, 16-way)
  - Keep sizes fixed
  - Measure: conflict miss reduction
  - Find: where more ways stop helping

Experiment 3: L3 size (2MB, 4MB, 8MB, 16MB, 32MB)
  - Key for shared workloads
  - Measure: DRAM access reduction per MB added
  - Calculate: cost-effectiveness (what does each extra MB buy you?)

Experiment 4: Cache line size (32B, 64B, 128B)
  - Tradeoff: spatial locality vs waste
  - Measure: miss rate AND bandwidth consumed
  - 128B lines = fewer misses but 2x bandwidth per miss

Create a massive results table. This IS your architecture analysis.
```

| 11:45-12:30 | 45 min | **Lunch** | |
| 12:30-14:00 | 90 min | **Add Prefetcher to Hierarchy** | Connect your stride prefetcher from Day 6 to L2: |
| | | | - L2 prefetcher observes L1 misses |
| | | | - Prefetches into L2 (not L1 — avoid L1 pollution) |
| | | | - If prefetched data is accessed: promote to L1 |
| | | | - Re-run all workloads with prefetcher enabled |
| | | | - Measure: AMAT improvement, bandwidth overhead, accuracy |
| 14:00-14:15 | 15 min | Break | |
| 14:15-15:45 | 90 min | **Connect Coherence** | Connect your MESI bus to L3: |
| | | | - Multi-core workloads now generate coherence traffic |
| | | | - L3 acts as snoop filter (checks before broadcasting to all L1s) |
| | | | - Measure: bus traffic with and without snoop filter |
| | | | - Run producer-consumer workload with 4 cores |
| 15:45-16:00 | 15 min | Break | |
| 16:00-17:30 | 90 min | **Write: Full Architecture Report** | Create `memory_hierarchy/ARCHITECTURE_REPORT.md`: |
| | | | - System diagram with all parameters |
| | | | - AMAT analysis for each workload |
| | | | - Design space exploration results (all 4 experiments) |
| | | | - Prefetcher impact analysis |
| | | | - Coherence overhead analysis |
| | | | - **"My recommended configuration for a 4-core mobile SoC"** with justification |
| | | | - Compare your config to Apple M4 (published specs) |
| 17:30-18:00 | 30 min | **Theory Notebook** | Handwrite: |
| | | | 1. AMAT equation — expand for 3 levels |
| | | | 2. Why is L1 always small? (Cycle time constraint) |
| | | | 3. Power breakdown: which level consumes most energy per access? |

**Day 8 Deliverables:**
- [ ] AMAT verification (formula matches simulation)
- [ ] 4 design space exploration experiments with tables
- [ ] Prefetcher integrated into hierarchy
- [ ] Coherence connected to L3
- [ ] `ARCHITECTURE_REPORT.md` — full analysis
- [ ] 3 handwritten notebook pages

---

## DAY 9 — February 16 (Sunday) 
### Theme: RTL — Write Your Cache in SystemVerilog

**WHY THIS DAY MATTERS:**
You have behavioral simulators in C++. Apple interviews ask: "Can you also write RTL?" Your SRAM controller RTL exists but it's basic. Today you write a synthesizable cache controller in SystemVerilog. This bridges software and hardware — the exact skill Apple needs.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Design: Cache Controller FSM** | Design on paper first: |

```
States:
  IDLE        — Waiting for request
  TAG_CHECK   — Compare tag, check valid bit (1 cycle)
  HIT         — Data available, return to pipeline (1 cycle)  
  MISS        — Allocate MSHR, evict if needed
  WRITEBACK   — Write dirty evicted line to next level
  REFILL      — Waiting for data from next level
  REFILL_DONE — Write new line into array, transition to HIT

Inputs:
  req_valid, req_addr[31:0], req_write, req_wdata[63:0]
  
Outputs:
  resp_valid, resp_data[63:0], resp_hit
  mem_req_valid, mem_req_addr[31:0], mem_req_write, mem_req_wdata[511:0]
  mem_resp_ready

Internal:
  Tag RAM: 256 entries × (tag_width + valid + dirty) bits
  Data RAM: 256 entries × 512 bits (64 bytes per line)
  LRU state: 256 sets × (associativity-1) bits
```

| 9:30-9:45 | 15 min | Break | |
| 9:45-11:45 | 120 min | **Build: Direct-Mapped Cache RTL** | Start simple — direct mapped: |

```systemverilog
// filepath: /memory_hierarchy/rtl/cache_controller.sv

module cache_controller #(
    parameter CACHE_SIZE = 8192,     // 8KB
    parameter LINE_SIZE  = 64,       // 64 bytes
    parameter ADDR_WIDTH = 32
)(
    input  logic clk, rst_n,
    
    // CPU interface
    input  logic        cpu_req_valid,
    input  logic [31:0] cpu_req_addr,
    input  logic        cpu_req_write,
    input  logic [63:0] cpu_req_wdata,
    output logic        cpu_resp_valid,
    output logic [63:0] cpu_resp_data,
    output logic        cpu_resp_hit,
    output logic        cpu_stall,
    
    // Memory interface (to next level)
    output logic        mem_req_valid,
    output logic [31:0] mem_req_addr,
    output logic        mem_req_write,
    output logic [511:0] mem_req_wdata,  // Full cache line
    input  logic        mem_resp_valid,
    input  logic [511:0] mem_resp_data
);

    // Derived parameters
    localparam NUM_LINES   = CACHE_SIZE / LINE_SIZE;      // 128
    localparam OFFSET_BITS = $clog2(LINE_SIZE);            // 6
    localparam INDEX_BITS  = $clog2(NUM_LINES);            // 7
    localparam TAG_BITS    = ADDR_WIDTH - OFFSET_BITS - INDEX_BITS;  // 19
    
    // Storage
    logic [TAG_BITS-1:0]  tag_ram  [NUM_LINES];
    logic                 valid_ram[NUM_LINES];
    logic                 dirty_ram[NUM_LINES];
    logic [511:0]         data_ram [NUM_LINES];
    
    // Address decomposition
    logic [OFFSET_BITS-1:0] offset;
    logic [INDEX_BITS-1:0]  index;
    logic [TAG_BITS-1:0]    tag;
    
    assign offset = cpu_req_addr[OFFSET_BITS-1:0];
    assign index  = cpu_req_addr[OFFSET_BITS +: INDEX_BITS];
    assign tag    = cpu_req_addr[ADDR_WIDTH-1 -: TAG_BITS];
    
    // FSM
    typedef enum logic [2:0] {
        IDLE, TAG_CHECK, HIT, MISS_WRITEBACK, MISS_REFILL, REFILL_DONE
    } state_t;
    
    state_t state, next_state;
    
    // Implement full FSM here...
    
endmodule
```

| 11:45-12:30 | 45 min | **Lunch** | |
| 12:30-14:30 | 120 min | **Build: Testbench** | Write a SystemVerilog testbench OR Verilator C++ testbench: |
| | | | - Test 1: Sequential reads — all misses first pass, all hits second pass |
| | | | - Test 2: Write then read back — verify data integrity |
| | | | - Test 3: Conflict miss — two addresses mapping to same index |
| | | | - Test 4: Dirty writeback — write to line, then conflict evict, verify writeback |
| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Simulate with Verilator** | |
| | | | - `verilator --cc --exe --build cache_controller.sv tb_cache.cpp` |
| | | | - Run all 4 tests |
| | | | - Cross-validate: compare hit/miss counts with your C++ simulator |
| | | | - They should match EXACTLY for the same access sequence |
| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Waveform Analysis** | |
| | | | - Dump VCD: `--trace` flag in Verilator |
| | | | - Open in GTKWave (or Surfer) |
| | | | - Verify: state transitions match your FSM diagram |
| | | | - Screenshot key waveforms for your portfolio |
| 17:30-18:00 | 30 min | **Theory Notebook** | Handwrite: |
| | | | 1. Cache controller FSM from memory |
| | | | 2. Why is tag check 1 cycle but refill is many cycles? |
| | | | 3. What is the difference between behavioral (C++) and RTL (SV) simulation? |

**Day 9 Deliverables:**
- [ ] `cache_controller.sv` — synthesizable direct-mapped cache
- [ ] Testbench with 4 test scenarios
- [ ] Verilator simulation passing all tests
- [ ] Cross-validation with C++ simulator (exact match)
- [ ] Waveform screenshots
- [ ] 3 handwritten notebook pages

---

## DAY 10 — February 17 (Monday)
### Theme: Read a Research Paper Like an Architect

**WHY THIS DAY MATTERS:**
Building simulators proves you can implement. Reading papers proves you can THINK at the frontier. Apple's memory team reads papers weekly. You need this skill. Today you read your first Mutlu paper and extract actionable architecture insights.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Read: "Flipping Bits in Memory Without Accessing Them" (RowHammer paper, 2014)** | This is Mutlu's most famous paper. Read it with this framework: |
| | | | 1. First pass (20 min): Read abstract, intro, conclusion ONLY. Write one sentence: what's the problem? |
| | | | 2. Second pass (30 min): Read Sections 2-3. How does RowHammer work? Draw the physical mechanism. |
| | | | 3. Third pass (40 min): Read Section 4 (solutions). List every mitigation proposed. |
| | | | **Do NOT read the evaluation section in detail.** Skip tables/graphs on first read. |
| 9:30-9:45 | 15 min | Break | |
| 9:45-11:15 | 90 min | **Implement: RowHammer in Your DRAM Sim** | Add RowHammer tracking to `dram_bank_fsm.cpp`: |

```cpp
// In dram_bank_fsm.h, add:
struct RowActivationTracker {
    std::unordered_map<int, int> activation_count;  // row_id → count since last refresh
    int rowhammer_threshold = 139000;  // MAC (Maximum Activate Count) for DDR5
    
    void record_activation(int row_id) {
        activation_count[row_id]++;
        if (activation_count[row_id] >= rowhammer_threshold) {
            // ALERT: This row is a RowHammer aggressor!
            // Victim rows: row_id-1 and row_id+1
            flag_victims(row_id - 1, row_id + 1);
        }
    }
    
    void on_refresh() {
        activation_count.clear();  // Reset counters on refresh
    }
    
    // Mitigation 1: TRR (Target Row Refresh)
    // On detecting aggressor, refresh victim rows
    std::vector<int> get_trr_targets();
    
    // Mitigation 2: PARA (Probabilistic Adjacent Row Activation)  
    // With probability p, refresh adjacent rows on every activation
    bool should_para_refresh(double probability = 0.001);
};
```

| 11:15-11:30 | 15 min | Break | |
| 11:30-12:15 | 45 min | **Lunch** | |
| 12:15-13:45 | 90 min | **RowHammer Experiment** | |
| | | | - Create attack pattern: activate same row 200,000 times |
| | | | - Measure: how many cycles until threshold reached? |
| | | | - Implement TRR: refresh adjacent rows every N activations |
| | | | - Measure TRR overhead: extra refreshes per normal operation |
| | | | - Calculate: bandwidth lost to TRR (TRR refreshes / total cycles) |
| 13:45-14:00 | 15 min | Break | |
| 14:00-15:30 | 90 min | **Read: "BLISS: Balancing Performance, Fairness and Complexity in Memory Access Scheduling" (Mutlu, 2014)** | |
| | | | - This directly relates to your QoS work from Day 2 |
| | | | - Key idea: blacklist cores that cause interference |
| | | | - A core is "blacklisted" if it has been served too many times |
| | | | - Blacklisted cores → requests deprioritized for T cycles |
| | | | - Extract: the algorithm in pseudocode |
| 15:30-15:45 | 15 min | Break | |
| 15:45-17:00 | 75 min | **Implement BLISS in Your Scheduler** | Add to `memory_scheduler.cpp`: |
| | | | - Track: requests served per core per epoch (10,000 cycles) |
| | | | - If core served > 2× fair share: blacklist for next epoch |
| | | | - Blacklisted core's requests go to bottom of queue |
| | | | - Compare: BLISS vs your Day 2 QoS vs FR-FCFS |
| 17:00-18:00 | 60 min | **Write Paper Notes** | Create `interview questions/papers/`: |
| | | | - `rowhammer_notes.md`: Problem, mechanism, mitigations, your implementation |
| | | | - `bliss_notes.md`: Algorithm, comparison with FR-FCFS, your results |
| | | | - For each paper, write: "In an interview, I would explain this as..." (3 sentences max) |

**Day 10 Deliverables:**
- [ ] RowHammer tracker added to DRAM sim
- [ ] TRR mitigation implemented and tested
- [ ] BLISS scheduler implemented
- [ ] Comparison: BLISS vs QoS vs FR-FCFS (fairness + performance)
- [ ] Paper notes with interview-ready summaries

---

## DAYS 11-14 — February 18-21 (Tue-Fri)
### Theme: DRAM Power States + LPDDR for Mobile

**WHY DAY 11-14 MATTERS:**
Apple makes MOBILE chips. LPDDR is their DRAM. Power management isn't optional — it's THE differentiator. These 4 days make you speak Apple's language.

### DAY 11 (Tuesday): DRAM Power States
| Time | Duration | Activity |
|------|----------|----------|
| 8:00-10:00 | 120 min | **Theory + Implementation:** Add power states to your DRAM bank FSM |
| | | Add states: ACTIVE_POWERDOWN, PRECHARGE_POWERDOWN, SELF_REFRESH |
| | | Each state has entry/exit latency and power level |
| | | Power down if bank idle > threshold cycles |
| 10:15-12:15 | 120 min | **Build: Power State Controller** |
| | | Policy: if idle > 50 cycles → precharge powerdown |
| | | If idle > 5000 cycles → self refresh |
| | | Track: time in each state, power per state |
| 12:15-13:00 | | Lunch |
| 13:00-15:00 | 120 min | **LPDDR5 vs DDR5 Comparison** |
| | | LPDDR5: 16-bank, multi-channel, lower voltage (0.5V vs 1.1V) |
| | | Add LPDDR5 timing parameters alongside DDR5 |
| | | Compare: power at same bandwidth, power at idle |
| 15:15-17:00 | 105 min | **Deep Sleep + Retention** |
| | | Implement: deep power down (data lost, must reinitialize) |
| | | Calculate: break-even point (how long idle before deep sleep saves power?) |
| | | This is EXACTLY what Apple does on M-series: sleep unused memory channels |
| 17:15-18:00 | 45 min | **Write LPDDR5_POWER_ANALYSIS.md** |

### DAY 12 (Wednesday): DVFS + Thermal
| Time | Duration | Activity |
|------|----------|----------|
| 8:00-10:00 | 120 min | **Theory:** Dynamic Voltage/Frequency Scaling for memory |
| | | Lower voltage = less power = slower |
| | | Add DVFS to your power model: 3 operating points (high/mid/low) |
| 10:15-12:15 | 120 min | **Build: Thermal Model** |
| | | Simple model: temperature rises with activity, falls with idle |
| | | T(t+1) = T(t) + α×power - β×(T(t) - T_ambient) |
| | | When T > threshold: force throttle to lower DVFS state |
| 12:15-13:00 | | Lunch |
| 13:00-15:00 | 120 min | **Thermal Throttling Experiment** |
| | | Run sustained bandwidth workload |
| | | Measure: bandwidth over time with thermal throttling |
| | | Expected: initial burst at full speed, then drops to sustainable rate |
| | | This is why Apple's benchmarks show peak vs sustained performance |
| 15:15-17:00 | 105 min | **Build: Bandwidth-Temperature Graph** |
| | | Plot (text-based): bandwidth vs time over 60-second simulation |
| | | Show throttling kicking in |
| 17:15-18:00 | 45 min | **Notebook: thermal equations, DVFS points, throttling policy** |

### DAY 13 (Thursday): CXL Memory — The Future
| Time | Duration | Activity |
|------|----------|----------|
| 8:00-10:00 | 120 min | **Theory: What is CXL?** |
| | | Compute Express Link: extends memory beyond the SoC |
| | | CXL.mem: remote memory accessed like local (but higher latency) |
| | | CXL.cache: allows devices to cache host memory coherently |
| | | Why it matters: AI models > GPU memory → need CXL expansion |
| 10:15-12:15 | 120 min | **Build: Two-Tier Memory Model** |
| | | Local DRAM: 100 cycle latency, 32 GB |
| | | CXL memory: 250 cycle latency, 256 GB |
| | | Policy: hot pages → local, cold pages → CXL |
| | | Page migration: move page from CXL→local when access count > threshold |
| 12:15-13:00 | | Lunch |
| 13:00-15:00 | 120 min | **Page Migration Policy** |
| | | Track per-page access counts in a hot page table |
| | | When page > threshold: migrate to local (cost: 2× page size in bandwidth) |
| | | When local memory full: demote coldest page to CXL |
| | | This is EXACTLY what Linux does with NUMA balancing |
| 15:15-17:00 | 105 min | **Experiment: CXL Benefit Analysis** |
| | | Workload: 64GB working set, only 32GB fits in local DRAM |
| | | Without CXL: constant swapping to SSD (milliseconds) |
| | | With CXL: 250ns latency for cold accesses (microseconds → nanoseconds) |
| | | Measure: average access latency with perfect vs LRU page placement |
| 17:15-18:00 | 45 min | **Write CXL_ARCHITECTURE.md** |

### DAY 14 (Friday): HBM Deep Dive
| Time | Duration | Activity |
|------|----------|----------|
| 8:00-10:00 | 120 min | **Extend your 3D stacking model** |
| | | Your `test_3d_stacking.cpp` exists but is basic |
| | | Add: per-layer thermal model (top die is hottest) |
| | | Add: TSV bandwidth model (TSV count limits bandwidth) |
| | | Add: per-channel independent bank FSMs (HBM has 16 independent channels) |
| 10:15-12:15 | 120 min | **HBM3E Specification Implementation** |
| | | 16 channels, each 64-bit wide |
| | | 2 pseudo-channels per channel (32-bit each, share row buffer) |
| | | 8.4 Gbps per pin, 32B per pseudo-channel |
| | | Total BW = 16 × 2 × 8.4 × 4B = 1.07 TB/s |
| | | Implement accurate bandwidth calculation |
| 12:15-13:00 | | Lunch |
| 13:00-15:00 | 120 min | **HBM vs LPDDR5 for Apple** |
| | | Apple uses LPDDR5 in phones/laptops, NOT HBM |
| | | Why? Power, package size, cost |
| | | Calculate: LPDDR5X at 8533 MT/s × 16B = 136.5 GB/s |
| | | Apple M4 has 8 channels → 8 × 136.5 = 1.09 TB/s theoretical |
| | | Compare with HBM3: similar bandwidth but different tradeoffs |
| 15:15-17:00 | 105 min | **Write: Memory Technology Comparison** |
| | | Create `HBM_VS_LPDDR.md` comparing: |
| | | Bandwidth, latency, power, cost/GB, package area, thermals |
| | | "For Apple's M5, I would recommend ___ because ___" |
| 17:15-18:00 | 45 min | **Week 2 Review** — Review ALL code written in Days 8-14 |

**Days 11-14 Deliverables:**
- [ ] DRAM power states (powerdown, self-refresh) in bank FSM
- [ ] LPDDR5 timing/power parameters
- [ ] Thermal model with throttling
- [ ] CXL two-tier memory simulator with page migration
- [ ] HBM3E accurate channel model
- [ ] HBM vs LPDDR comparison document
- [ ] Power analysis reports

---

## DAY 15 — February 22 (Saturday)
### Theme: Write-Back Cache RTL (Set-Associative)

**WHY THIS DAY MATTERS:**
Day 9 gave you a direct-mapped cache in RTL. Nobody uses direct-mapped in production. Today you build the REAL thing — a 4-way set-associative write-back cache in synthesizable SystemVerilog. This is the single most asked-about RTL block in memory architecture interviews. When Apple asks "have you written any RTL?", you say "I wrote a 4-way write-back cache with LRU, verified it in Verilator, and cross-validated against my C++ model."

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:00 | 60 min | **Design on Paper** | Draw the microarchitecture before writing ANY code: |
| | | | - Tag RAM: 4 tags per set, each with valid + dirty bits |
| | | | - Data RAM: 4 data blocks per set (64 bytes each) |
| | | | - LRU bits: pseudo-LRU tree (3 bits per set for 4-way) |
| | | | - FSM: IDLE → TAG_CHECK → HIT → MISS_EVICT → WRITEBACK → REFILL → FILL_DONE |
| | | | - Way select MUX: which of 4 ways matches? |
| | | | Draw signal-level block diagram with every wire labeled |
| 9:00-9:15 | 15 min | Break | |
| 9:15-11:15 | 120 min | **Build: set_assoc_cache.sv** | New file: `/memory_hierarchy/rtl/set_assoc_cache.sv` |
| | | | - Parameterized: `CACHE_SIZE`, `ASSOC`, `LINE_SIZE`, `ADDR_WIDTH` |
| | | | - Pseudo-LRU: on hit, update tree bits to point AWAY from accessed way |
| | | | - On eviction: follow tree to find LRU victim |
| | | | - Write-back policy: only write dirty evictions to next level |
| | | | - Write-allocate: on write miss, fetch line first, then write into it |
| 11:15-11:30 | 15 min | Break | |
| 11:30-12:15 | 45 min | **Lunch** | |
| 12:15-14:15 | 120 min | **Build: Testbench** | `/memory_hierarchy/rtl/tb_set_assoc_cache.cpp` (Verilator C++ testbench) |
| | | | - Test 1: Sequential reads — verify cold misses then hits |
| | | | - Test 2: Way conflict — 5 addresses mapping to same set → verify LRU eviction |
| | | | - Test 3: Dirty writeback — write to line, evict it, verify writeback data correct |
| | | | - Test 4: Write-allocate — write miss fetches line first, then stores data |
| | | | - Test 5: Cross-validate — same 10,000-access trace through C++ sim and RTL, compare exact hit/miss sequence |
| 14:15-14:30 | 15 min | Break | |
| 14:30-16:00 | 90 min | **Simulate + Debug** | |
| | | | - `verilator --cc --exe --build --trace set_assoc_cache.sv tb_set_assoc_cache.cpp` |
| | | | - All 5 tests must pass |
| | | | - If cross-validation fails: dump VCD, find divergence cycle, fix RTL or C++ model |
| | | | - Screenshot: passing tests + waveform of a hit followed by a miss |
| 16:00-16:15 | 15 min | Break | |
| 16:15-17:15 | 60 min | **Synthesis Check** | Run Yosys (open-source synthesis) to see if your RTL is synthesizable: |
| | | | - `yosys -p "read_verilog -sv set_assoc_cache.sv; synth; stat"` |
| | | | - Record: gate count, flip-flop count, estimated area |
| | | | - If synthesis errors: fix RTL (no SystemVerilog constructs Yosys can't handle) |
| 17:15-18:00 | 45 min | **Theory Notebook** | Handwrite: |
| | | | 1. Pseudo-LRU tree diagram for 4-way (show tree update on access to way 2) |
| | | | 2. Why write-back > write-through for performance? (fewer memory writes) |
| | | | 3. Draw the critical path in your cache RTL: addr → tag compare → way select → data out |

**Day 15 Deliverables:**
- [ ] `set_assoc_cache.sv` — 4-way set-associative, write-back, pseudo-LRU
- [ ] Verilator testbench with 5 test scenarios
- [ ] Cross-validation: RTL vs C++ exact match on 10,000 accesses
- [ ] Yosys synthesis report (gate count, synthesizable)
- [ ] Waveform screenshots for portfolio
- [ ] 3 handwritten notebook pages

---

## DAY 16 — February 23 (Sunday)
### Theme: Memory Controller RTL

**WHY THIS DAY MATTERS:**
You have a DRAM simulator in C++ and a cache in RTL. The memory controller is what connects them. Writing this in RTL proves you can design hardware that enforces DRAM timing constraints — tRCD, tCAS, tRP. This is Apple's memory controller team's core work.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Design: Controller Architecture** | Draw on paper: |
| | | | - Request queue: 8-entry FIFO, each entry = {addr, r/w, data, valid, age} |
| | | | - Bank state tracker: per-bank FSM (IDLE/ACTIVE/PRECHARGING) + open row register |
| | | | - Scheduler: simple FR-FCFS — row hits first, then FCFS among misses |
| | | | - Command sequencer: issues ACT/RD/WR/PRE with correct timing gaps |
| | | | - Timing counters: per-bank countdown timers for tRCD, tCAS, tRP, tRAS |
| | | | Interface: cache side (req/resp) ↔ controller ↔ DRAM side (command/data bus) |
| 9:30-9:45 | 15 min | Break | |
| 9:45-11:45 | 120 min | **Build: mem_controller.sv** | `/memory_hierarchy/rtl/mem_controller.sv` |
| | | | - 4 banks, each with state register and open_row register |
| | | | - Request queue with age counter (for FCFS ordering) |
| | | | - Row hit detection: `(bank_state[bank] == ACTIVE) && (open_row[bank] == req_row)` |
| | | | - Command output: `{cmd_type, bank, row, col}` with cmd_valid |
| | | | - Timing enforcement: counters that block commands until safe |
| 11:45-12:30 | 45 min | **Lunch** | |
| 12:30-14:30 | 120 min | **Build: Testbench** | |
| | | | - Test 1: Single read — verify ACT(tRCD wait) → RD(tCAS wait) → DATA sequence |
| | | | - Test 2: Row hit — two reads to same row, second skips ACT |
| | | | - Test 3: Row miss — read to different row, verify PRE → ACT → RD sequence |
| | | | - Test 4: Bank interleaving — reads to 4 different banks overlap correctly |
| | | | - Verify: NO timing violations (tRCD, tCAS, tRP all respected) |
| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Cross-Validate with C++ DRAM Sim** | |
| | | | - Feed same request sequence to RTL controller and C++ `memory_scheduler.cpp` |
| | | | - Compare: command issue order, cycle-by-cycle commands, total latency |
| | | | - They should match within ±1 cycle (RTL is cycle-accurate, C++ may differ on ties) |
| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Waveform Analysis** | |
| | | | - Dump VCD, open in GTKWave |
| | | | - Annotate waveform: label tRCD gap, tCAS gap, row hit vs miss |
| | | | - Screenshot for portfolio: show timing enforcement working |
| 17:30-18:00 | 30 min | **Theory Notebook** | Handwrite: |
| | | | 1. Memory controller FSM with ALL state transitions |
| | | | 2. Why FR-FCFS improves throughput (row buffer locality) |
| | | | 3. What happens if you violate tRCD? (Data corruption — activating row isn't ready) |

**Day 16 Deliverables:**
- [ ] `mem_controller.sv` — 4-bank controller with FR-FCFS scheduling
- [ ] Timing enforcement verified (no tRCD/tCAS/tRP violations)
- [ ] Cross-validation with C++ DRAM sim
- [ ] Annotated waveform screenshots
- [ ] 3 handwritten notebook pages

---

## DAY 17 — February 24 (Monday)
### Theme: Patterson & Hennessy Chapter 5 — Deep Architecture Read

**WHY THIS DAY MATTERS:**
P&H is THE textbook Apple interviewers expect you've read. Chapter 5 covers memory hierarchy comprehensively — but the key is connecting every concept to YOUR code. "I implemented 6 of the 10 advanced cache optimizations in my simulator" is the most powerful interview statement possible.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-10:00 | 120 min | **Read: Chapter 5 Sections 5.1–5.4** | Focus on: |
| | | | - 5.1: Memory hierarchy principles (locality, inclusion) |
| | | | - 5.2: Cache basics (map every diagram to your code) |
| | | | - 5.3: Measuring cache performance (AMAT equation — you computed this on Day 8) |
| | | | - 5.4: Virtual memory basics (you'll build TLB in Month 2) |
| | | | For EACH section: write 1 sentence connecting it to your project |
| 10:00-10:15 | 15 min | Break | |
| 10:15-12:15 | 120 min | **Read: The 10 Advanced Cache Optimizations** | These are from Hennessy & Patterson Appendix B. Map each to your work: |
| | | | 1. Small, simple L1 cache → Your L1 config (32KB, 4-cycle) ✅ |
| | | | 2. Way prediction → Not implemented yet (Month 2 candidate) |
| | | | 3. Pipelined cache access → Your multi-cycle FSM ✅ |
| | | | 4. Nonblocking caches → Your MSHR implementation from Day 5 ✅ |
| | | | 5. Multibanked caches → Not implemented (can add to hierarchy sim) |
| | | | 6. Critical word first → Can add to refill path |
| | | | 7. Merging write buffers → Not implemented yet |
| | | | 8. Compiler optimizations → Workload design (loop tiling) |
| | | | 9. Hardware prefetching → Your stride prefetcher from Day 6 ✅ |
| | | | 10. Compression → Not implemented (Month 3 candidate) |
| 12:15-13:00 | 45 min | **Lunch** | |
| 13:00-14:30 | 90 min | **Implement: Critical Word First + Merging Write Buffer** | Two quick additions to your hierarchy sim: |
| | | | **Critical word first:** On cache miss, request the specific word first, deliver to CPU immediately, then fill rest of line. Reduces effective miss penalty by ~30%. |
| | | | **Write buffer:** Queue of 4 pending writes. CPU doesn't stall on writes — write goes to buffer. Buffer drains to cache/memory in background. If buffer full, CPU stalls. |
| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Measure Optimization Impact** | For each optimization you've implemented, measure speedup: |
| | | | - Baseline: blocking, no prefetch, no write buffer, no critical word first |
| | | | - Add each optimization one at a time, measure AMAT |
| | | | - Create table showing cumulative improvement |
| | | | Target: achieve 40-60% AMAT reduction from all optimizations combined |
| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Write: HENNESSY_CHAPTER5_NOTES.md** | |
| | | | - For each of the 10 optimizations: definition, tradeoff, your implementation status |
| | | | - AMAT improvement table |
| | | | - "If I could only implement 3 optimizations, I'd choose ___, ___, ___ because ___" |
| 17:30-18:00 | 30 min | **Theory Notebook** | Handwrite the AMAT equation expanded for 3 levels with all optimization terms |

**Day 17 Deliverables:**
- [ ] P&H Chapter 5 read with annotations
- [ ] Critical word first implemented + measured
- [ ] Write buffer implemented + measured
- [ ] Optimization impact table (6+ optimizations)
- [ ] `HENNESSY_CHAPTER5_NOTES.md`
- [ ] 2 handwritten notebook pages

---

## DAY 18 — February 25 (Tuesday)
### Theme: Gem5 Introduction — Industry-Standard Simulator

**WHY THIS DAY MATTERS:**
gem5 is what Apple, AMD, ARM, Intel ALL use internally for architecture exploration. If you can run gem5 and compare its results to YOUR simulator, you can validate your work against an industry-standard tool. This also shows interviewers you know the tooling, not just your own code.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-10:30 | 150 min | **Build gem5 (ARM target)** | Build for ARM, not x86 — Apple uses ARM cores, so ARM is the relevant ISA. |
| | | | - `git clone https://github.com/gem5/gem5.git` |
| | | | - Follow build instructions for macOS (may need Docker/Linux VM) |
| | | | - Build: `scons build/ARM/gem5.opt -j$(nproc)` |
| | | | - If Mac build fails: use Docker image `gcr.io/gem5-test/ubuntu-22.04_all-dependencies` |
| | | | - Alternative: use gem5's pre-built Docker container |
| | | | - **Why ARM:** Apple M-series = ARM. ARM has a weakly ordered memory model (vs x86 TSO). Understanding ARM memory ordering is critical for coherence/store buffer design. |
| 10:30-10:45 | 15 min | Break | |
| 10:45-12:15 | 90 min | **First Simulation: SE Mode** | |
| | | | - Write simple C program: matrix multiply 64×64 |
| | | | - Cross-compile for gem5 target |
| | | | - Cross-compile for ARM: `aarch64-linux-gnu-gcc -static -O2 matrix_mult.c -o matrix_mult` |
| | | | - Run: `./gem5.opt configs/example/se.py -c matrix_mult --cpu-type=O3CPU --caches --l2cache` |
| | | | - Extract: IPC, L1 hit rate, L2 hit rate, DRAM accesses |
| 12:15-13:00 | 45 min | **Lunch** | |
| 13:00-15:00 | 120 min | **Configure: Match Your Hierarchy** | |
| | | | - Set gem5 L1 = 32KB 8-way 4-cycle (your Day 7 config) |
| | | | - Set gem5 L2 = 256KB 8-way 12-cycle |
| | | | - Set gem5 L3 = 8MB 16-way 36-cycle |
| | | | - Set gem5 DRAM = DDR4-3200 |
| | | | - Run same workload, compare hit rates to your simulator |
| 15:00-15:15 | 15 min | Break | |
| 15:15-16:45 | 90 min | **Compare: Your Sim vs gem5** | |
| | | | - Create comparison table: your sim vs gem5 for same config |
| | | | - Where do they agree? (Should be within 5% on hit rates) |
| | | | - Where do they differ? (Your sim may miss TLB, speculative execution, etc.) |
| | | | - Each difference = a learning opportunity (what did you simplify?) |
| 16:45-17:00 | 15 min | Break | |
| 17:00-18:00 | 60 min | **Write: GEM5_COMPARISON.md** | |
| | | | - Setup instructions (so you can reproduce later) |
| | | | - Configuration parameters used |
| | | | - Comparison table |
| | | | - Analysis: why differences exist |

**Day 18 Deliverables:**
- [ ] gem5 built and running (**ARM target**, not x86)
- [ ] First simulation completed (matrix_mult, ARM binary)
- [ ] Config matched to your hierarchy sim
- [ ] Comparison table: your sim vs gem5 (hit rates, latency)
- [ ] `GEM5_COMPARISON.md`
- [ ] Short note in comparison doc: ARM weak memory ordering vs x86 TSO — implications for store buffers and coherence

---

## DAY 19 — February 26 (Wednesday)
### Theme: gem5 Design Space Exploration + Ramulator2

**WHY THIS DAY MATTERS:**
Yesterday you ran gem5 with one config. Today you sweep parameters — the same design space exploration from Day 8, but now validated against an industry tool. You also integrate Ramulator2 (Mutlu's DRAM simulator) to cross-validate your DRAM model.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-10:00 | 120 min | **gem5 Parameter Sweep** | Write a Python script that runs gem5 with different configs: |
| | | | - L1 size sweep: 16KB, 32KB, 64KB |
| | | | - L2 associativity sweep: 4-way, 8-way, 16-way |
| | | | - Cache line size sweep: 32B, 64B, 128B |
| | | | - Extract stats from `m5out/stats.txt` automatically |
| | | | - Generate comparison table |
| 10:00-10:15 | 15 min | Break | |
| 10:15-12:15 | 120 min | **Cross-validate your DSE vs gem5 DSE** | |
| | | | - Compare: your Day 8 design sweep results vs gem5 results |
| | | | - Do the TRENDS match? (Both should show diminishing returns from larger L1) |
| | | | - Do the ABSOLUTE values match? (Probably within 10-20%) |
| | | | - If trends disagree: something is wrong in your model — debug it |
| 12:15-13:00 | 45 min | **Lunch** | |
| 13:00-15:00 | 120 min | **Install + Run Ramulator2** | |
| | | | - `git clone https://github.com/CMU-SAFARI/ramulator2.git` |
| | | | - Build and run with DDR4 config |
| | | | - Feed same trace file to Ramulator2 and your DRAM sim |
| | | | - Compare: latency distribution, bandwidth, power estimates |
| 15:00-15:15 | 15 min | Break | |
| 15:15-17:00 | 105 min | **Integrate Ramulator2 as Backend** | |
| | | | - Option A: Feed your hierarchy sim's DRAM requests into Ramulator2 via trace file |
| | | | - Option B: Link Ramulator2 library into your hierarchy sim as the DRAM backend |
| | | | - Either way: you now have an industry-validated DRAM model backing your sim |
| 17:00-18:00 | 60 min | **Write: VALIDATION_REPORT.md** | |
| | | | - Your sim vs gem5 (cache hierarchy) |
| | | | - Your DRAM sim vs Ramulator2 (DRAM timing) |
| | | | - Validation summary: "My simulator agrees with industry tools to within X%" |
| | | | **This report is GOLD for interviews.** |

**Day 19 Deliverables:**
- [ ] gem5 parameter sweep (automated script)
- [ ] Your sim vs gem5 trend comparison
- [ ] Ramulator2 running with same trace
- [ ] Your DRAM sim vs Ramulator2 comparison
- [ ] `VALIDATION_REPORT.md` (industry-tool validation)

---

## DAY 20 — February 27 (Thursday)
### Theme: Virtual Memory Foundations + TLB Design

**WHY THIS DAY MATTERS:**
Your cache sim uses physical addresses. Real CPUs use virtual addresses. The TLB (Translation Lookaside Buffer) is a cache for page table translations — and a TLB miss costs 100+ cycles (page table walk). Apple's M-series has sophisticated TLBs. You need to understand address translation to be a complete memory architect.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-9:30 | 90 min | **Theory: Virtual Memory (x86 + ARM)** | Study until you can explain from memory: |
| | | | - Virtual address → {VPN (Virtual Page Number), Page Offset} |
| | | | - Page table: VPN → PPN (Physical Page Number) mapping |
| | | | - **x86-64:** 4-level page table: PML4 → PDPT → PD → PT → Physical Page |
| | | | - Each level: 9 bits of VPN → 512 entries per table, 4KB default pages |
| | | | - **ARM (AArch64):** 4-level page table: L0 → L1 → L2 → L3 |
| | | | - ARM supports 4KB, 16KB, or 64KB granules. **Apple M-series uses 16KB pages by default** (not 4KB!) |
| | | | - 16KB pages → fewer TLB misses for same working set, but more internal fragmentation |
| | | | - TLB: caches recent VPN→PPN translations |
| | | | - TLB miss → page table walk → up to 4 memory accesses! |
| 9:30-9:45 | 15 min | Break | |
| 9:45-11:45 | 120 min | **Build: TLB + Page Table Walker** | Create `/memory_hierarchy/cpp/tlb.cpp`: |
| | | | - TLB: fully-associative, 64 entries, LRU replacement |
| | | | - Page table: simplified 2-level (enough to demonstrate the concept) |
| | | | - Page sizes: 4KB (x86 default), **16KB (ARM/Apple default)**, 2MB huge pages |
| | | | - Compare: x86 4KB vs ARM 16KB — how does this change TLB coverage? |
| | | | - TLB hit: 1 cycle, translates virtual → physical |
| | | | - TLB miss: page walk = 4 memory accesses × DRAM latency |
| 11:45-12:30 | 45 min | **Lunch** | |
| 12:30-14:30 | 120 min | **Integrate TLB into Memory Hierarchy** | Modify your Day 7 memory hierarchy: |
| | | | - CPU issues virtual address → TLB lookup → physical address → cache lookup |
| | | | - On TLB miss: walk page table (generates additional memory requests!) |
| | | | - Measure: AMAT with vs without TLB |
| | | | - Test: working set with 1000 unique pages vs 100 pages (TLB thrashing) |
| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Huge Page Experiment** | |
| | | | - 4KB pages: 64 TLB entries cover 256KB of address space |
| | | | - **16KB pages (Apple M-series): 64 TLB entries cover 1MB** — 4× better than x86! |
| | | | - 2MB pages: 64 TLB entries cover 128MB of address space |
| | | | - Run workload with 10MB working set: |
| | | | - 4KB pages → TLB misses every time working set > 256KB → massive slowdown |
| | | | - 2MB pages → 5 TLB entries cover entire working set → nearly zero misses |
| | | | - Create comparison table |
| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Write: TLB_ANALYSIS.md** | |
| | | | - TLB hit rate vs working set size |
| | | | - 4KB vs 2MB page comparison |
| | | | - TLB miss penalty impact on total AMAT |
| | | | - "For Apple M5 with ML workloads (large tensors), I'd recommend ___ page size because ___" |
| 17:30-18:00 | 30 min | **Theory Notebook** | Draw 4-level page table walk step by step |

**Day 20 Deliverables:**
- [ ] TLB implemented (64-entry, fully-associative, LRU)
- [ ] Page table walker (2-level)
- [ ] TLB integrated into memory hierarchy
- [ ] Multi-granule page support (4KB + **16KB ARM/Apple** + 2MB)
- [ ] TLB hit rate vs working set size data (**include 16KB granule comparison**)
- [ ] `TLB_ANALYSIS.md` — must include ARM vs x86 page table format comparison

---

## DAY 21 — February 28 (Friday)
### Theme: Resume + GitHub Portfolio + Phase 1 Assessment

**WHY THIS DAY MATTERS:**
You've built an incredible amount in 21 days. Today you package it. A GitHub portfolio with proper READMEs, architecture diagrams, and quantified results is what gets you interviews. Most students don't document their work — you will.

| Time | Duration | Activity | Details |
|------|----------|----------|---------|
| 8:00-10:00 | 120 min | **Portfolio Organization** | Structure your GitHub: |
| | | | Repo: `josh-carter-memory-systems` |
| | | | - `/cache-simulator/` — direct-mapped + 4-way + coherence + prefetching |
| | | | - `/dram-simulator/` — bank FSM + scheduler + power model + RowHammer |
| | | | - `/memory-hierarchy/` — full L1→L2→L3→DRAM hierarchy |
| | | | - `/rtl/` — cache controller + memory controller in SystemVerilog |
| | | | - `/analysis/` — all reports (QoS, prefetch, coherence, validation) |
| | | | Each folder needs a README with: architecture diagram, how to build, how to run, key results |
| 10:00-10:15 | 15 min | Break | |
| 10:15-12:15 | 120 min | **Write READMEs** | Each README follows this template: |
| | | | ```## Project: [Name]` |
| | | | `### What It Does` (3 sentences) |
| | | | `### Architecture` (diagram) |
| | | | `### Key Results` (1 table with numbers) |
| | | | `### How to Build & Run` (exact commands) |
| | | | `### What I Learned` (2-3 bullet points)``` |
| 12:15-13:00 | 45 min | **Lunch** | |
| 13:00-14:30 | 90 min | **Resume Draft** | One page. Sections: |
| | | | - Education: [Your uni], Computer Engineering, Year 1 (expected graduation 2029) |
| | | | - Projects: 5-6 key projects with 1-line description + quantified result |
| | | | - Example: "Memory Hierarchy Simulator — 4-core L1/L2/L3/DRAM model with coherence, prefetching, and MSHR support. Validated against gem5 within 8% accuracy." |
| | | | - Skills: C++, SystemVerilog, Python, Verilator, gem5, DRAM architecture, cache design |
| | | | - Coursework: [Whatever you've taken / are taking] |
| 14:30-14:45 | 15 min | Break | |
| 14:45-16:15 | 90 min | **Phase 1 Self-Assessment** | Run every test suite. Answer honestly: |
| | | | - Can you explain your bank FSM without looking at code? Y/N |
| | | | - Can you draw MESI state diagram from memory? Y/N |
| | | | - Can you derive AMAT for a 3-level hierarchy? Y/N |
| | | | - Can you explain why prefetching helps strided but not pointer-chasing? Y/N |
| | | | - Can you explain your RTL cache controller FSM? Y/N |
| | | | Any "N" → review that topic over the weekend |
| 16:15-16:30 | 15 min | Break | |
| 16:30-17:30 | 60 min | **Write: PHASE1_COMPLETION.md** | |
| | | | - Total lines of code written |
| | | | - Total projects |
| | | | - Key metrics (hit rates, speedups, validation accuracy) |
| | | | - Gaps identified → input for Month 2 priorities |
| 17:30-18:00 | 30 min | **Plan Month 2** | Review the Month 2 plan below. Adjust based on what you found easy/hard. |

**Day 21 Deliverables:**
- [ ] GitHub portfolio organized with READMEs
- [ ] Resume draft (1 page)
- [ ] All test suites passing
- [ ] Self-assessment completed
- [ ] `PHASE1_COMPLETION.md`
- [ ] Month 2 priorities identified

---

# AT THE END OF MONTH 1 (February 28, 2026), YOU WILL HAVE:

```
PROJECTS BUILT:
 1. Direct-mapped cache simulator (C++)
 2. 4-way set-associative cache with LRU/FIFO/Random (C++ + Python)
 3. SRAM behavioral model + RTL (C++ + Verilog + SystemVerilog)
 4. DRAM simulator with bank FSM, FR-FCFS scheduler, power model (C++)
 5. Address mapping module (3 schemes) (C++)
 6. QoS memory controller (C++)
 7. MESI coherence simulator (C++)
 8. MOESI extension (C++)
 9. Non-blocking cache with MSHRs (C++)
10. Stride/next-line/Markov prefetchers (C++)
11. Full memory hierarchy simulator L1→L2→L3→DRAM (C++)
12. RowHammer tracker + TRR mitigation (C++)
13. BLISS scheduler (C++)
14. DRAM power states + thermal model (C++)
15. CXL two-tier memory model (C++)
16. HBM3E channel model (C++)
17. 4-way set-associative cache RTL (SystemVerilog)
18. Memory controller RTL (SystemVerilog)
19. TLB + page table walker (C++)
20. Critical word first + write buffer optimizations (C++)

TOOLS USED: C++, Python, SystemVerilog, Verilator, Yosys, gem5, Ramulator2

THEORY MASTERED:
- DRAM timing (tRCD, tCAS, tRP, tRFC, tREFI)
- Cache hierarchy (AMAT, associativity tradeoffs, inclusion)
- Coherence (MESI/MOESI state machines, false sharing)
- Prefetching (stride detection, accuracy vs coverage)
- Virtual memory (page tables, TLB, huge pages)
- Scheduling (FR-FCFS, QoS, BLISS)
- RowHammer (attack + defense)
- Power (IDD model, DVFS, thermal throttling)
- SRAM fundamentals (6T cell, SNM, read/write conflict)
- RTL design (synthesizable FSMs, cross-validation)

PAPERS READ: 2 (RowHammer, BLISS)
NOTEBOOK PAGES: ~60
REPORTS WRITTEN: ~10
```

---

# MONTH 2: MARCH 1 – MARCH 31, 2026
## Theme: Advanced DRAM + Cache Replacement + ECC
## WHY: These are the three topics Apple memory teams work on daily. DRAM scheduling is where the bandwidth comes from. Cache replacement is where the efficiency comes from. ECC is non-negotiable for production silicon.

### Week 5 (March 1-7): Advanced DRAM Scheduling

**WHY:** Your FR-FCFS scheduler is textbook. Real controllers use more sophisticated policies. Apple's LPDDR controller handles multiple QoS classes simultaneously. This week you build production-grade scheduling.

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 22 | Read CROW paper (Mutlu) — Copy-Row DRAM for lower latency | `papers/crow_notes.md` | Implement CROW in your DRAM sim: duplicate hot rows in spare capacity |
| 23 | Read LISA paper — Low-cost Inter-Segment copy in DRAM | `papers/lisa_notes.md` | Implement bulk data movement within DRAM subarrays |
| 24 | Multi-channel controller | `dram/cpp/multi_channel.cpp` | 2-channel controller with address interleaving (channel bit selection) |
| 25 | Channel arbitration | `dram/cpp/channel_arbiter.cpp` | Round-robin vs priority-based channel arbitration, measure bandwidth |
| 26 | DDR5 specific features | `dram/cpp/ddr5_features.cpp` | Same-bank refresh, per-bank refresh, DDR5 two-subchannel architecture |
| 27 | Write-CRC model | `dram/cpp/write_crc.cpp` | DDR5 write CRC — detect write bus errors before data stored |
| 28 | Week 5 integration + testing | `dram/tests/test_advanced_scheduling.cpp` | All new features tested, performance report |

### Week 6 (March 8-14): Cache Replacement Policies — Beyond LRU

**WHY:** LRU is simple but not optimal. Intel/AMD/Apple all use variants of RRIP. Understanding why LRU fails (scan pattern destroys LRU) and what replaces it is core architect knowledge.

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 29 | Read Jaleel et al. RRIP paper (2010) | `papers/rrip_notes.md` | Understand re-reference interval prediction concept |
| 30 | Implement SRRIP (Static RRIP) | `cache sim/4-way cache/cpp/srrip.cpp` | 2-bit RRPV per line, insert at long re-reference, promote on hit |
| 31 | Implement BRRIP (Bimodal RRIP) | `cache sim/4-way cache/cpp/brrip.cpp` | Mix of distant and long insertion — handles scan patterns |
| 32 | Implement DRRIP (Dynamic RRIP) | `cache sim/4-way cache/cpp/drrip.cpp` | Set-dueling: some sets use SRRIP, some BRRIP, pick winner dynamically |
| 33 | Read SHiP paper (Signature-based Hit Predictor) | `papers/ship_notes.md` | Learn signature-based insertion: use PC to predict reuse |
| 34 | Implement SHiP | `cache sim/4-way cache/cpp/ship.cpp` | PC-indexed signature table predicts insertion priority |
| 35 | Replacement policy comparison | `cache sim/4-way cache/REPLACEMENT_ANALYSIS.md` | LRU vs SRRIP vs BRRIP vs DRRIP vs SHiP — 5 workloads, full table |

### Week 7 (March 15-21): ECC + Reliability

**WHY:** Every DRAM bit can flip (soft errors from cosmic rays, hard errors from wear). ECC is mandatory in servers and increasingly in mobile (Apple uses it). This is real engineering, not academic.

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 36 | Theory: Hamming codes, SECDED | `reliability/cpp/hamming.cpp` | Implement single-error-correct, double-error-detect for 64-bit data |
| 37 | Theory: BCH codes, chipkill | `reliability/cpp/bch.cpp` | Multi-bit error correction, understand chipkill-correct (whole DRAM chip fails) |
| 38 | DRAM soft error rates + scrubbing | `reliability/cpp/scrubber.cpp` | Background scrubbing: periodically read+check+correct all memory |
| 39 | ECC overhead analysis | `reliability/ECC_OVERHEAD.md` | Bandwidth cost of ECC (1 extra beat per read), storage overhead (12.5% for SECDED) |
| 40 | Reliability modeling in DRAM sim | `dram/cpp/reliability_model.cpp` | Inject random bit flips at configurable rate, verify ECC catches them |
| 41 | RowHammer + ECC interaction | `dram/tests/test_rowhammer_ecc.cpp` | Can RowHammer cause multi-bit errors that overwhelm SECDED? (Yes! Show it) |
| 42 | Week 7 review + integration test | `reliability/RELIABILITY_REPORT.md` | Full reliability analysis: error rates, ECC coverage, scrub impact |

### Week 8 (March 22-28): On-Chip Interconnect + NoC Basics

**WHY:** In a multi-core chip, caches don't talk directly — they communicate through a Network-on-Chip (NoC). Apple's M-series uses a ring or mesh. Understanding NoC basics completes your picture of how data moves from DRAM to core.

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 43 | Theory: bus vs crossbar vs ring vs mesh | `noc/NOC_THEORY.md` | Draw all 4 topologies, calculate bisection bandwidth for each |
| 44 | Build: Simple ring NoC | `noc/cpp/ring_noc.cpp` | 4-node ring, packet-based, 1 flit per cycle per link |
| 45 | Build: Mesh NoC (2×2) | `noc/cpp/mesh_noc.cpp` | XY routing, 4 nodes, measure latency for various traffic patterns |
| 46 | Connect NoC to memory hierarchy | `memory_hierarchy/cpp/noc_integration.cpp` | L2 miss → NoC → L3 slice (distributed shared cache) |
| 47 | NoC congestion experiment | `noc/tests/test_noc_congestion.cpp` | All-to-all traffic: measure latency vs offered load curve |
| 48 | Read: Apple M-series interconnect analysis | `papers/apple_interconnect_notes.md` | Study AnandTech/Chips and Cheese M4 analysis — extract interconnect details |
| 49 | Week 8 review + Month 2 assessment | `MONTH2_COMPLETION.md` | All projects tested, portfolio updated |

---

# MONTH 3: APRIL 1 – APRIL 30, 2026
## Theme: System-Level Integration + First Open Source Contribution
## WHY: Month 1 = components. Month 2 = advanced components. Month 3 = you connect EVERYTHING into one coherent system and start showing up in the open-source community where Apple engineers lurk.

### Week 9 (April 1-7): Full SoC Memory System Simulator

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 50 | Design SoC memory architecture | `soc_sim/ARCHITECTURE.md` | CPU (4 cores) + GPU (8 shader cores, 32 threads/warp) + NPU sharing unified memory. Draw the full memory map: CPU L1/L2 → shared L3 ← GPU L1/L2 ← NPU DMA |
| 51 | **Build GPU memory coalescing unit** | `soc_sim/cpp/gpu_coalescer.cpp` | This is the #1 GPU memory concept. A warp = 32 threads issuing addresses simultaneously. Coalescer merges them: if 32 threads access consecutive 4B → 1 cache line request (128B). If 32 threads access random addresses → up to 32 separate requests (32× bandwidth waste). Implement: take 32 addresses from a warp, group by cache line, emit minimal memory requests. Track coalescing efficiency (ideal: 1 request per warp, worst: 32). |
| 52 | **Build GPU cache hierarchy** | `soc_sim/cpp/gpu_cache.cpp` | GPU caches are NOT like CPU caches. Build a simplified GPU memory hierarchy: **L1 texture cache** (per shader core, 16KB, read-only, optimized for 2D spatial locality), **shared memory / scratchpad** (per shader core, 48KB, software-managed, banked — 32 banks, bank conflicts cause serialization), **L2 cache** (shared across all shader cores, 512KB, partitioned into slices — 1 per memory channel). Key experiment: measure bank conflicts in shared memory with different access strides. stride=1 → zero conflicts, stride=32 → all threads hit same bank → 32× slower. |
| 53 | Build NPU memory traffic generator + Heterogeneous QoS | `soc_sim/cpp/npu_traffic.cpp`, `soc_sim/cpp/het_qos.cpp` | NPU: weight loading (sequential) + activation streaming (strided). QoS controller: CPU = low latency priority, GPU = bandwidth priority (tolerates latency, needs throughput), NPU = burst tolerance. Key insight: GPU hides memory latency through massive parallelism (thousands of threads) — it doesn't need low latency, it needs sustained bandwidth. |
| 54 | SoC integration + **GPU partition camping** | `soc_sim/cpp/soc_simulator.cpp`, `soc_sim/cpp/gpu_partition_camp.cpp` | All clients connected through NoC + memory controller. Then implement the classic GPU memory pathology: **partition camping**. If all shader cores access addresses that map to the SAME L2 partition → one partition saturated, others idle → bandwidth collapses. Demonstrate: sequential access → camping → fix with address swizzling. This is a real NVIDIA interview question. |
| 55 | SoC performance analysis + **GPU memory experiments** | `soc_sim/SOC_ANALYSIS.md`, `soc_sim/GPU_MEMORY_ANALYSIS.md` | Two reports: (1) SoC-level: bandwidth allocation pie chart, latency per client, QoS effectiveness. (2) GPU-specific: coalescing efficiency vs access pattern (sequential/strided/random), shared memory bank conflict rates, L2 partition utilization heatmap, partition camping before/after swizzling. |
| 56 | Multi-workload scenarios | `soc_sim/tests/test_mixed_workloads.cpp` | CPU gaming + GPU rendering + NPU inference simultaneously. GPU-specific workloads: (1) matrix multiply — great coalescing, high shared memory reuse, (2) graph traversal — terrible coalescing, random access, (3) texture sampling — 2D spatial locality in L1 texture cache. Show the 10-50× performance difference between coalesced and uncoalesced GPU memory access. |

### Week 10 (April 8-14): Open Source Contribution — DRAMSim3/Ramulator2

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 57 | Study DRAMSim3 codebase | `opensource/DRAMSIM3_NOTES.md` | Map their architecture to yours — what's different? |
| 58 | Find issues / enhancement opportunities | `opensource/CONTRIBUTION_PLAN.md` | Pick 1-2 issues: bug fix, documentation, small feature |
| 59 | Write contribution code | Fork on GitHub | PR-ready code with tests |
| 60 | Write tests + documentation for contribution | Fork on GitHub | Submit PR with clear description |
| 61 | Study Ramulator2 codebase | `opensource/RAMULATOR2_NOTES.md` | Map their config system to your DRAM sim |
| 62 | Cross-validate deeper | `opensource/CROSS_VALIDATION.md` | Your sim vs Ramulator2 on 10+ workloads with detailed analysis |
| 63 | Portfolio update | GitHub | All repos documented, open-source PR submitted |

### Week 11 (April 15-21): Processing-in-Memory (PIM)

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 64 | Read Mutlu PIM survey paper | `papers/pim_survey_notes.md` | Key concept: compute in DRAM rows to avoid data movement |
| 65 | Build: bitwise PIM simulator | `pim/cpp/pim_bitwise.cpp` | RowClone: copy entire DRAM row in 1 tRC (vs reading out + writing back) |
| 66 | Build: bulk operations | `pim/cpp/pim_bulk.cpp` | Bulk zero, bulk copy, bulk AND/OR using row activation tricks |
| 67 | PIM workload analysis | `pim/tests/test_pim_workloads.cpp` | Databases (selection, projection), graph analytics (BFS), ML (batch norm) |
| 68 | PIM bandwidth savings | `pim/PIM_ANALYSIS.md` | Calculate: how much memory bus bandwidth PIM saves per workload |
| 69 | Google AIM paper | `papers/aim_notes.md` | Commercial PIM: Samsung HBM-PIM, SK Hynix AIM — read one paper |
| 70 | Integration: PIM operations in your DRAM sim | `dram/cpp/pim_extension.cpp` | Add PIM command type alongside READ/WRITE/REFRESH |

### Week 12 (April 22-28): PYNQ Z2 FPGA Deployment + Month 3 Review

**WHY:** You've been simulating RTL in Verilator. Now you run it on REAL HARDWARE. The PYNQ Z2 has a Xilinx Zynq-7020 (dual ARM Cortex-A9 + Artix-7 FPGA fabric). You'll deploy your cache controller and memory controller to the FPGA, drive them from the ARM cores, and compare results against your C++ sim. "I ran my memory controller on an FPGA and validated it against my simulator" is a sentence that gets you hired.

| Day | Focus | Key File | Deliverable |
|-----|-------|----------|-------------|
| 71 | PYNQ Z2 setup + Vivado install | `rtl/fpga/PYNQ_SETUP.md` | Board booted, Vivado installed, first "blinky" LED test working |
| 72 | Synthesize cache controller for Zynq | `rtl/fpga/cache_synth.xdc` | Vivado synthesis of `set_assoc_cache.sv` targeting xc7z020, resource report (LUTs, FFs, BRAMs) |
| 73 | Build AXI wrapper for cache | `rtl/fpga/axi_cache_wrapper.sv` | Wrap your cache with AXI4-Lite interface so ARM core can drive it via memory-mapped registers |
| 74 | Deploy + test cache on FPGA | `python/pynq/test_cache_fpga.ipynb` | PYNQ Jupyter notebook: send addresses from ARM → cache on FPGA → read results back, compare hit/miss with C++ sim |
| 75 | Synthesize memory controller for Zynq | `rtl/fpga/memctrl_synth.xdc` | Vivado synthesis of `mem_controller.sv`, resource + timing report |
| 76 | FPGA vs Verilator comparison report | `docs/FPGA_VALIDATION.md` | Compare: FPGA resource usage, max frequency, waveform probing (ILA) vs Verilator results. Month 3 completion + resume update. |
| 77 | Mock interview (self-test) | `interview_prep/MOCK_1.md` | Write out answers to 10 architecture questions without references. Include: "I ran my RTL on an FPGA." |

---

# MONTHS 4-6: MAY – JULY 2026
## Theme: Deep Specialization + University Year 1 Courses Begin
## WHY: Uni starts. Your schedule drops to ~4-5 hrs/day on weekdays (after classes). Weekends still 8-10 hrs. Focus narrows to 2-3 specialization areas.

### Month 4 (May): Memory Security + RowHammer Defense
**WHY:** Memory security is THE hottest research area. Apple cares deeply — a RowHammer attack on an iPhone is a security vulnerability.

| Week | Project | Key Files | Sentence |
|------|---------|-----------|----------|
| 13 | Comprehensive RowHammer defense simulator | `security/cpp/rowhammer_defense.cpp` | Compare 5 defenses: TRR, PARA, TWiCe, Graphene, BlockHammer — measure perf overhead vs protection |
| 14 | Memory isolation for security | `security/cpp/memory_isolation.cpp` | Physical address randomization to prevent targeted RowHammer — build DRAM-aware allocator |
| 15 | Read 2 papers: BlockHammer (HPCA'21) + Graphene (MICRO'20) | `papers/blockhammer_notes.md`, `papers/graphene_notes.md` | Implement both algorithms, compare overhead |
| 16 | Security analysis report | `security/SECURITY_REPORT.md` | Full comparison table: protection level, perf overhead, area cost, for each defense |

### Month 5 (June): Emerging Memory Technologies
**WHY:** DRAM is hitting scaling walls. STT-MRAM, PCM, and hybrid memories are becoming real products. Understanding what comes AFTER DRAM puts you ahead of every other intern candidate.

| Week | Project | Key Files | Sentence |
|------|---------|-----------|----------|
| 17 | STT-MRAM model | `emerging/cpp/sttmram.cpp` | Non-volatile, fast read (~10ns), slow write (~50ns), no refresh — model as L3/L4 cache replacement |
| 18 | PCM (Phase Change Memory) model | `emerging/cpp/pcm.cpp` | Non-volatile, higher density than DRAM, limited write endurance (~10^8) — model wear leveling |
| 19 | Hybrid DRAM+NVM hierarchy | `emerging/cpp/hybrid_memory.cpp` | DRAM as cache for PCM main memory, page migration based on access frequency |
| 20 | Technology comparison report | `emerging/TECHNOLOGY_COMPARISON.md` | SRAM vs DRAM vs STT-MRAM vs PCM vs ReRAM: latency, density, power, endurance, cost |

### Month 6 (July): Memory Compression + Bandwidth Optimization
**WHY:** Memory bandwidth is THE bottleneck for AI. Compression increases effective bandwidth without hardware changes. Apple uses memory compression in macOS and iOS.

| Week | Project | Key Files | Sentence |
|------|---------|-----------|----------|
| 21 | Base-Delta-Immediate compression | `compression/cpp/bdi_compressor.cpp` | Cache line compression: store base + small deltas, fits 128B data in 64B — 2× effective capacity |
| 22 | DRAM bandwidth compression (LZ-based) | `compression/cpp/bandwidth_comp.cpp` | Compress data on memory bus — more data per transfer |
| 23 | Compressed cache architecture | `compression/cpp/compressed_cache.cpp` | Variable-size cache lines: some lines compressed → more lines fit → fewer misses |
| 24 | Compression analysis + Month 6 review | `compression/COMPRESSION_REPORT.md` | Compression ratio vs workload type, effective bandwidth improvement, decompression latency cost |

---

# MONTHS 7-9: AUGUST – OCTOBER 2026
## Theme: System-Level Mastery + Portfolio Projects
## WHY: By now you're the most experienced Year 2 student in memory architecture. These months are about building PORTFOLIO PIECES — the 3-4 projects that get you the interview.

### Month 7 (August): Portfolio Project 1 — "MemSim: Full-Stack Memory Simulator"
**WHY:** Combine EVERYTHING into one polished, documented, open-source simulator.

| Week | Focus | Key Files | Sentence |
|------|-------|-----------|----------|
| 25 | Unified API: all components accessible through single interface | `memsim/cpp/memsim.cpp`, `memsim/include/memsim.h` | JSON config file → spawn entire SoC memory system with one API call |
| 26 | Python bindings (pybind11) | `memsim/python/pymemsim.cpp` | Drive your C++ sim from Python scripts for rapid experimentation |
| 27 | Visualization dashboard | `memsim/python/dashboard.py` | Real-time plots: hit rates, bandwidth, latency histograms, power |
| 28 | Documentation + examples + CI | `memsim/README.md`, `.github/workflows/ci.yml` | GitHub Actions CI runs all tests on push; comprehensive documentation |

### Month 8 (September): Portfolio Project 2 — "CXL Memory Pooling Simulator"
**WHY:** CXL is the future. Almost no students have CXL projects. This alone makes you stand out.

| Week | Focus | Key Files | Sentence |
|------|-------|-----------|----------|
| 29 | CXL 3.0 fabric model | `cxl/cpp/cxl_fabric.cpp` | Multi-host, multi-device CXL fabric with shared memory pools |
| 30 | Memory pooling + dynamic allocation | `cxl/cpp/memory_pool.cpp` | Hosts request/release memory from shared pool — model fragmentation |
| 31 | CXL coherence (back-invalidation) | `cxl/cpp/cxl_coherence.cpp` | Host cache coherence across CXL — snoop filter at CXL switch |
| 32 | CXL use cases: AI inference farm | `cxl/CXL_ANALYSIS.md` | Model: 8 inference servers sharing 2TB CXL memory pool vs 256GB each local |

### Month 9 (October): Portfolio Project 3 — "RTL Memory Controller with Formal Verification + FPGA Demo"
**WHY:** RTL + formal verification + FPGA = the hardware engineering trifecta. Most students can't do formal verification, and almost none deploy to real FPGAs. This demonstrates you can design, prove correct, AND build real hardware.

| Week | Focus | Key Files | Sentence |
|------|-------|-----------|----------|
| 33 | Expand memory controller RTL to DDR5 | `rtl/mem_controller_ddr5.sv` | Full DDR5 command sequencing with 2 subchannels |
| 34 | Formal properties (SVA assertions) | `rtl/mem_controller_props.sv` | Assert: tRCD never violated, no two ACTs to same bank within tRC |
| 35 | Formal verification with SymbiYosys | `rtl/formal/` | Prove ALL timing constraints are satisfied for ALL possible inputs |
| 36 | FPGA deployment: full memory subsystem + **soft RISC-V core** on PYNQ Z2 | `rtl/fpga/full_memsys_fpga.sv`, `rtl/fpga/picorv32_wrapper.sv`, `python/pynq/memsys_demo.ipynb` | Integrate a **PicoRV32 soft RISC-V core** on the FPGA fabric and connect YOUR cache controller to it via AXI. Run real RISC-V programs through your custom memory subsystem on hardware. ARM PS drives test control from Python, RISC-V PL core executes workloads through your cache. "I ran RISC-V programs through my custom cache controller on an FPGA" — this sentence gets interviews. Also deploy cache + memory controller + AXI interconnect. Vivado synthesis report with area/power. |

---

# MONTHS 10-12: NOVEMBER 2026 – JANUARY 2027
## Theme: Interview Preparation + Open Source Leadership
## WHY: Applications open. You need to be ready to talk about everything you've built, read every paper you've cited, and solve whiteboard problems on memory architecture.

### Month 10 (November): Interview Question Bank
| Week | Focus | Key Files | Sentence |
|------|-------|-----------|----------|
| 37 | Write 20 cache architecture questions + answers | `interview_prep/cache_questions.md` | Coverage: AMAT, associativity, coherence, prefetching, replacement, inclusion |
| 38 | Write 20 DRAM architecture questions + answers | `interview_prep/dram_questions.md` | Coverage: timing, scheduling, refresh, power states, RowHammer, ECC |
| 39 | Write 10 system-level questions + answers | `interview_prep/system_questions.md` | Coverage: SoC memory, QoS, CXL, HBM, chiplets, PIM |
| 40 | Write 10 RTL/design questions + answers | `interview_prep/rtl_questions.md` | Coverage: FSM design, timing closure, formal verification, synthesis tradeoffs |

### Month 11 (December): Mock Interviews + Paper Deep Dives
| Week | Focus | Key Files | Sentence |
|------|-------|-----------|----------|
| 41 | Read 5 more Mutlu papers on hot topics | `papers/` | Cover: RowHammer defense, PIM, memory scheduling fairness, DRAM latency |
| 42 | Mock interviews with peers/mentors | `interview_prep/mock_results.md` | 3-4 full mock interviews, record weaknesses, improve |
| 43 | Apple M-series deep dive | `interview_prep/apple_m_series.md` | Study every public detail: cache sizes, memory bandwidth, LPDDR config, die shots |
| 44 | Behavioral interview prep | `interview_prep/behavioral.md` | "Tell me about a time you..." stories from your projects |

### Month 12 (January 2027): Applications + Final Polish
| Week | Focus | Key Files | Sentence |
|------|-------|-----------|----------|
| 45 | Submit applications: Apple, AMD, NVIDIA, Qualcomm, Intel, Samsung, ARM, Google | `applications/tracker.md` | Cover letter customized per company referencing their specific memory work |
| 46 | Final portfolio review | All repos | Every README perfect, every test passing, CI green |
| 47 | Open source: 2nd contribution to gem5/Ramulator2/DRAMSim3 | GitHub PRs | Meaningful contribution (feature, not just typo fix) |
| 48 | Interview simulation: full day mock | `interview_prep/final_mock.md` | 4-hour mock: technical + behavioral + system design |

---

# MONTH 13-14: FEBRUARY – MARCH 2027
## Theme: Active Interviewing
## WHY: Apple summer internship interviews typically happen January-March for summer positions.

### Month 13 (February 2027): Interview Season
| Week | Focus | Sentence |
|------|-------|----------|
| 49 | Phone screens + online assessments | Expect: cache design question, DRAM timing question, coding (C++) |
| 50 | On-site / virtual interviews round 1 | Expect: whiteboard FSM design, explain your project, system design question |
| 51 | On-site / virtual interviews round 2 | Expect: deep technical dive on one project, "how would you improve X?" |
| 52 | Follow-ups + additional company interviews | Cast wide net: 6-8 companies simultaneously |

### Month 14 (March 2027): Offers + Decision
| Week | Focus | Sentence |
|------|-------|----------|
| 53 | Final round interviews | Any remaining companies |
| 54 | Offer negotiation | Compare offers on: team, project scope, mentorship, location |
| 55 | Accept offer | 🎉 |
| 56 | Pre-internship prep | Study the team's specific focus area in depth before day 1 |

---

# BY MARCH 2027, YOU WILL HAVE:

## Complete Project Portfolio

```
SIMULATORS (C++):
 1. Direct-mapped cache simulator
 2. 4-way set-associative cache (LRU/FIFO/Random/SRRIP/BRRIP/DRRIP/SHiP)
 3. SRAM behavioral model
 4. DRAM simulator (bank FSM, FR-FCFS/BLISS/QoS scheduling, power model)
 5. Multi-channel DRAM controller
 6. MESI/MOESI coherence simulator
 7. Non-blocking cache with MSHRs
 8. Stride/next-line/Markov prefetchers 
 9. Full memory hierarchy simulator (L1→L2→L3→DRAM, 4-core)
10. TLB + page table walker (4KB + 2MB pages)
11. RowHammer defense simulator (TRR, PARA, BlockHammer, Graphene)
12. ECC/SECDED + scrubbing model
13. Ring + mesh NoC simulator
14. SoC memory system (CPU + GPU + NPU with heterogeneous QoS)
15. CXL memory pooling simulator (multi-host, shared pool)
16. PIM simulator (RowClone, bulk operations)
17. Hybrid DRAM+NVM memory model
18. Cache compression (BDI)
19. Chiplet memory architecture model
20. MemSim: unified simulator with Python bindings + dashboard

RTL (SystemVerilog):
21. SRAM array + controller
22. Direct-mapped cache controller
23. 4-way set-associative write-back cache (pseudo-LRU)
24. DDR5 memory controller with formal verification (SVA + SymbiYosys)

FPGA (PYNQ Z2 — Xilinx Zynq-7020):
25. Cache controller deployed to FPGA with AXI wrapper
26. Memory controller deployed to FPGA
27. Full memory subsystem on FPGA (cache + controller + AXI interconnect)
28. HW/SW co-design: ARM cores drive workloads, FPGA handles memory

TOOLS MASTERED:
C++, Python, SystemVerilog, Verilator, Yosys, SymbiYosys,
gem5, Ramulator2, DRAMSim3, KLayout, GTKWave, pybind11,
Vivado, PYNQ framework, AXI4 interface design, ILA (Integrated Logic Analyzer)

PAPERS READ: 20+
- RowHammer (Kim et al., 2014)
- BLISS (Subramanian et al., 2014)
- RRIP (Jaleel et al., 2010)
- SHiP (Wu et al., 2011) 
- CROW (Hassan et al., 2019)
- LISA (Chang et al., 2016)
- BlockHammer (Yağlıkçı et al., 2021)
- Graphene (Park et al., 2020)
- PIM Survey (Ghose et al., 2019)
- + 11 more on emerging topics

OPEN SOURCE CONTRIBUTIONS: 2-3 merged PRs to DRAMSim3/Ramulator2/gem5

INTERVIEW QUESTIONS PREPARED: 60+

REPORTS/ANALYSES: 15+ detailed architecture reports with quantified results
```

## Lines of Code Estimate

```
Language           Lines            %
──────────────────────────────────────
C++                25,000-30,000    55%    Core simulation, all models
SystemVerilog      6,000-9,000      17%    RTL designs + FPGA wrappers + testbenches
Python             8,000-10,000     19%    Analysis, pybind11, PYNQ notebooks, dashboard
Tcl/XDC            500-1,000        2%     FPGA constraints + Vivado scripts
CMake/Makefile     500-800          2%     Build system
Markdown           2,000-3,000      5%     Reports + documentation
──────────────────────────────────────
Total              ~42,000-54,000
```

---

# EVERY PROJECT — RATED OUT OF 1000 FOR APPLE

How much would an Apple Silicon Engineering interviewer care about this project?
- **900-1000:** "Stop, you're hired." Directly maps to what we do.
- **700-899:** "This is very impressive." Strong signal of competence.
- **500-699:** "Good foundation." Expected or nice-to-have.
- **300-499:** "Okay, shows effort." Useful but not differentiating.
- **<300:** "Everyone does this." Won't move the needle.

```
#   Project                                           Apple /1000   When Built   Why This Score
──────────────────────────────────────────────────────────────────────────────────────────────────
                           *** MONTH 1 — EXISTING PROJECTS ***
 1  Direct-mapped cache simulator (C++)                    320       Pre-plan     Basic. Every undergrad could build this.
 2  4-way set-associative cache (LRU/FIFO/Random)          480       Pre-plan     Shows you understand associativity + eviction
                                                                                  tradeoffs. Cross-language validation (C++ + Python)
                                                                                  is a nice touch. But no MSHR, no prefetch.
 3  SRAM behavioral model + RTL                            550       Pre-plan     RTL shows hardware thinking. Cross-validation
                                                                                  (behavioral vs RTL) is professional methodology.
                                                                                  Apple cares that you can bridge SW↔HW.
 4  DRAM simulator (bank FSM + FR-FCFS + power)            720       Pre-plan     THIS is your crown jewel so far. Bank FSM with
                                                                                  real timing, scheduling policies, and power model
                                                                                  is genuinely impressive for pre-university.
                                                                                  Apple's LPDDR controller team does exactly this.

                           *** MONTH 1 — NEW PROJECTS ***
 5  Address mapping module (3 schemes)                     380       Day 1        Important foundation but small scope. Shows you
                                                                                  understand row/bank/column interleaving tradeoffs.
 6  QoS memory controller                                  650       Day 2        Apple M-series MUST handle QoS between CPU cores,
                                                                                  GPU, NPU. Showing BW fairness + starvation prevention
                                                                                  proves you think about real system constraints.
 7  MESI coherence simulator                               700       Day 3        Cache coherence is interview question #1 for
                                                                                  multicore memory architects. Having a working MESI
                                                                                  sim with false sharing analysis is strong.
 8  MOESI extension                                        750       Day 4        Goes BEYOND MESI. Apple uses MOESI variants.
                                                                                  MESI→MOESI comparison showing memory bandwidth
                                                                                  savings is exactly the architect thinking they want.
 9  Non-blocking cache with MSHRs                          780       Day 5        MSHRs are what make caches REAL. Most students
                                                                                  never implement these. Measuring perf vs MSHR count
                                                                                  shows you understand the hardware cost tradeoff.
10  Stride/next-line/Markov prefetchers                    750       Day 6        Prefetching is active Apple research. Having 3
                                                                                  prefetchers + accuracy/coverage/timeliness metrics
                                                                                  + per-workload comparison is very strong.
11  Full memory hierarchy (L1→L2→L3→DRAM, 4-core)          870       Day 7-8      *** TOP PROJECT *** This is the money project.
                                                                                  4-core with coherence, prefetching, MSHRs, inclusive
                                                                                  L3, connected to your DRAM sim. Working set sweep
                                                                                  showing latency staircase. Design space exploration.
                                                                                  Validated against gem5. This IS what Apple architects do.
12  RowHammer tracker + TRR mitigation                     680       Day 10       Security is hot. Showing you understand the attack
                                                                                  AND implemented mitigations proves security awareness.
13  BLISS scheduler                                        620       Day 10       Shows you read papers and implement them. Fairness-
                                                                                  aware scheduling is relevant for Apple's multi-client
                                                                                  memory controller.
14  DRAM power states + thermal model                      700       Day 11-12    Power is Apple's #1 concern for mobile. LPDDR power
                                                                                  states + DVFS + thermal throttling = exactly what
                                                                                  their power team cares about. Bandwidth-vs-time
                                                                                  graph showing throttling is a great visual.
15  CXL two-tier memory model                              600       Day 13       Forward-looking. CXL is less relevant for iPhone/Mac
                                                                                  today but shows you understand where memory is going.
                                                                                  Page migration policy is architecturally interesting.
16  HBM3E channel model                                    580       Day 14       Apple doesn't use HBM (they use LPDDR), but
                                                                                  understanding HBM shows breadth. The LPDDR vs HBM
                                                                                  comparison is more valuable than the model itself.
17  4-way set-associative cache RTL (SystemVerilog)        820       Day 15       *** HIGH VALUE *** Write-back, pseudo-LRU, parameterized,
                                                                                  synthesizable, cross-validated against C++ model.
                                                                                  This is exactly the RTL Apple's cache team writes.
                                                                                  Yosys synthesis report shows you think about area.
18  Memory controller RTL (SystemVerilog)                  850       Day 16       *** HIGH VALUE *** 4-bank FR-FCFS with timing
                                                                                  enforcement in synthesizable RTL. Cross-validated
                                                                                  with C++ DRAM sim. Very few undergrads can do this.
                                                                                  Apple literally has a "memory controller RTL" team.
19  TLB + page table walker                                650       Day 20       Important for completeness. Huge page analysis
                                                                                  relevant to Apple's M-series (they support both 4KB
                                                                                  and 16KB pages). TLB thrashing analysis is good.
20  Critical word first + write buffer                     520       Day 17       Smart optimizations showing you read H&P and can
                                                                                  implement textbook concepts. Good but incremental.

                           *** MONTH 2 — ADVANCED TOPICS ***
21  CROW + LISA DRAM optimizations                         600       Week 5       Implementing Mutlu papers shows research skill.
                                                                                  Not directly Apple technology but demonstrates
                                                                                  ability to read + implement academic ideas.
22  Multi-channel DRAM controller                          700       Week 5       Apple M4 has 8 LPDDR channels. Multi-channel
                                                                                  address interleaving is directly relevant.
23  DDR5 two-subchannel model                              650       Week 5       DDR5 subchannels are the present. Shows you're
                                                                                  current with JEDEC standards.
24  SRRIP/BRRIP/DRRIP cache replacement                   760       Week 6       *** STRONG *** Going beyond LRU with set-dueling
                                                                                  dynamic selection is exactly what production caches
                                                                                  use. Apple's L2/L3 almost certainly uses RRIP variants.
25  SHiP (Signature-based Hit Predictor)                   740       Week 6       PC-based insertion prediction is cutting-edge.
                                                                                  5-policy comparison report with workload analysis
                                                                                  shows architect-level evaluation skills.
26  ECC/SECDED + scrubbing model                           680       Week 7       Non-negotiable for production silicon. Apple
                                                                                  MUST have ECC. Showing RowHammer overwhelms
                                                                                  SECDED is an insightful experiment.
27  Ring + mesh NoC simulator                              600       Week 8       Good foundation for understanding data movement
                                                                                  in Apple's interconnect. Not core memory work
                                                                                  but shows system-level thinking.

                           *** MONTH 3 — SYSTEM INTEGRATION ***
28  SoC memory system (CPU+GPU+NPU, hetero QoS)           950       Week 9       *** TOP PROJECT *** This is Apple. Literally.
    + GPU memory subsystem deep dive                                               M-series SoC = CPU + GPU + NPU with unified memory.
    (coalescing, GPU caches, partition camping)                                     Heterogeneous QoS (latency for CPU, bandwidth for
                                                                                  GPU, burst for NPU) is exactly their architecture
                                                                                  challenge. GPU memory coalescing unit + GPU cache
                                                                                  hierarchy (L1 texture, shared mem, partitioned L2)
                                                                                  + partition camping demo makes NVIDIA a real target
                                                                                  too. Mixed workload analysis is cherry on top.
29  Open-source PR to DRAMSim3/Ramulator2                  700       Week 10      Shows you can work in real codebases, not just
                                                                                  your own. Merged PR = engineering maturity.
30  PIM simulator (RowClone, bulk ops)                     580       Week 11      Forward-looking technology. Not Apple's current
                                                                                  focus but demonstrates awareness of where memory
                                                                                  architecture is heading. Good conversation starter.
31  PYNQ Z2: Cache controller on FPGA                     880       Week 12      *** FPGA PROJECT *** Your cache running on REAL
                                                                                  Zynq-7020 silicon with AXI wrapper. ARM drives
                                                                                  traffic from Python, FPGA processes cache lookups.
                                                                                  Cross-validated against C++ sim IN HARDWARE.
                                                                                  "I tested my RTL on real silicon."

                           *** MONTHS 4-6 — SPECIALIZATION ***
32  RowHammer defense comparison (5 algorithms)            720       Month 4      *** STRONG for Apple *** iPhone security matters
                                                                                  enormously. Comparing TRR/PARA/BlockHammer/Graphene/
                                                                                  TWiCe with perf-vs-protection tradeoff analysis
                                                                                  is publishable-quality work.
33  Memory isolation / DRAM-aware allocator                650       Month 4      Security + OS-level memory awareness. Shows you
                                                                                  think about the full stack, not just hardware.
34  STT-MRAM / PCM models                                  500       Month 5      Forward-looking but not Apple's current stack.
                                                                                  Good for breadth; won't drive the interview.
35  Hybrid DRAM+NVM hierarchy                              550       Month 5      Interesting architecturally. Page migration
                                                                                  policies are relevant to any tiered memory system
                                                                                  (including CXL).
36  Cache compression (BDI)                                650       Month 6      Apple uses memory compression in macOS/iOS.
                                                                                  Understanding compression ratio vs decompression
                                                                                  latency tradeoff is directly applicable.
37  Bandwidth compression                                  600       Month 6      Memory bandwidth is Apple's constraint with
                                                                                  LPDDR (vs HBM). Compression = more effective BW.

                           *** MONTHS 7-9 — PORTFOLIO PROJECTS ***
38  MemSim: unified sim + Python bindings + dashboard      900       Month 7      *** TOP PROJECT *** Professional-grade tool. JSON
                                                                                  config → full SoC sim. Python bindings for rapid
                                                                                  experimentation. Dashboard for visualization. CI/CD.
                                                                                  This is what a startup or research group would ship.
39  CXL memory pooling simulator                           750       Month 8      *** DIFFERENTIATION *** Almost zero undergrads
                                                                                  know CXL. Multi-host shared memory pool with
                                                                                  coherence across CXL fabric is genuinely novel.
                                                                                  AI inference farm use-case analysis is topical.
40  DDR5 memory controller RTL + formal + FPGA deploy      980       Month 9      *** THE KILLER PROJECT *** Synthesizable DDR5
                                                                                  controller RTL with SVA assertions PROVEN correct
                                                                                  by SymbiYosys formal verification, THEN deployed
                                                                                  to PYNQ Z2 FPGA with ARM driving workloads.
                                                                                  HW/SW co-design demo. Almost no undergrad can do
                                                                                  formal verification on a memory controller AND
                                                                                  run it on real hardware. Instant credibility.
44  Cache controller on FPGA (PYNQ Z2)                     880       Month 3      *** FPGA PROJECT *** Your set-associative cache
                                                                                  running on real Zynq-7020 FPGA fabric with AXI
                                                                                  wrapper, driven from ARM cores via PYNQ Python
                                                                                  notebooks. Cross-validated against C++ sim IN
                                                                                  HARDWARE. "I tested my RTL on real silicon."
45  Full memory subsystem on FPGA                          920       Month 9      Cache + memory controller + interconnect all on
                                                                                  FPGA. ARM runs workload generator, FPGA processes
                                                                                  memory requests. Vivado resource/timing reports.
                                                                                  This is what Apple's prototyping team does.

                       *** MONTHS 10-14 — INTERVIEW PREP ***
41  60+ interview questions (self-written + answered)      500       Month 10     Table stakes for interview success. Shows thorough
                                                                                  preparation. Won't wow them but will prevent failure.
42  Apple M-series deep dive document                      700       Month 11     Shows you did your homework on THEIR product.
                                                                                  Interviewers love when candidates know the product.
43  2nd open-source contribution (gem5/Ramulator2)         700       Month 12     Repeat contribution shows sustained engagement,
                                                                                  not a one-off.

──────────────────────────────────────────────────────────────────────────────────────────────────
GRAND PORTFOLIO SCORE:  31,660 / 47,000  (67% — weighted toward the 7 projects scoring 870+)
──────────────────────────────────────────────────────────────────────────────────────────────────
```

### THE 7 PROJECTS THAT GET YOU HIRED (sort by Apple score):

```
Score   Project                                           Why Apple Cares
─────   ──────────────────────────────────────────────     ──────────────────────────────────────
 980    DDR5 RTL + formal verification + FPGA deploy       We DO this. Proven correct AND runs
                                                           on hardware. No undergrad does this.
 920    SoC memory system (CPU+GPU+NPU + hetero QoS)       This IS the M-series memory problem.
                                                           You simulated our architecture.
 920    Full memory subsystem on FPGA (PYNQ Z2)            Cache + controller on real Zynq.
                                                           HW/SW co-design. We prototype this way.
 900    MemSim: unified simulator + Python + dashboard     Professional tool. JSON config → full
                                                           SoC sim. We use tools like this.
 880    Cache controller on FPGA (PYNQ Z2)                 Your RTL running on real silicon.
                                                           Validated in hardware, not just sim.
 870    Full memory hierarchy (L1→L2→L3→DRAM, 4-core)      Foundational. Validated against gem5.
                                                           Shows complete understanding.
 850    Memory controller RTL (SystemVerilog)               We write memory controllers in RTL.
                                                           You can too. Day 1 productive.
```

### HOW EACH MONTH'S WORK FEEDS THE NEXT:

```
Month 1 (Foundation)
  ├── Cache sim ──────────────────┐
  ├── DRAM sim ──────────────────┤
  ├── Coherence ─────────────────┤
  ├── Prefetching ───────────────┤
  ├── MSHRs ─────────────────────┤
  ├── RTL (cache + mem ctrl) ────┤
  └── TLB ───────────────────────┤
                                 ▼
Month 2 (Advanced Components)
  ├── Advanced DRAM (multi-channel, DDR5) ──── builds on Month 1 DRAM sim
  ├── RRIP/DRRIP/SHiP replacement ─────────── builds on Month 1 cache sim
  ├── ECC/reliability ─────────────────────── builds on Month 1 DRAM sim
  └── NoC ─────────────────────────────────── builds on Month 1 coherence
                                 ▼
Month 3 (Integration + FPGA)
  ├── SoC sim (CPU+GPU+NPU) ───────────────── pulls Month 1 hierarchy + Month 2 NoC
  ├── Open source contribution ────────────── needs Month 1-2 understanding
  ├── PIM ─────────────────────────────────── builds on Month 1 DRAM sim
  └── PYNQ Z2 FPGA deployment ─────────────── deploys Month 1 RTL to real hardware
                                 ▼
Months 4-6 (Specialization)
  ├── RowHammer defense (5 algorithms) ────── builds on Month 1 Day 10 tracker
  ├── Emerging memory (STT-MRAM, PCM) ────── builds on Month 1 DRAM understanding
  └── Compression (BDI) ──────────────────── builds on Month 1 cache sim
                                 ▼
Months 7-9 (Portfolio Projects)
  ├── MemSim unified ──────────────────────── ALL of Months 1-6 combined into one API
  ├── CXL pooling sim ────────────────────── Month 1 CXL + Month 3 integration
  └── DDR5 RTL + formal + FPGA demo ───────── Month 1 RTL + Month 2 DDR5 + Month 3 FPGA skills
                                 ▼
Months 10-14 (Interview + Apply)
  └── Everything above feeds into interview answers and portfolio
```

---

# WHERE YOU'LL RANK + JOB TARGETS

## Percentile Among Year 2 Students

```
Skill Area                      Your Level          Percentile
────────────────────────────────────────────────────────────────
Memory architecture knowledge   PhD-student level    Top 0.1%
C++ systems programming         Strong junior eng    Top 5%
RTL design + FPGA               Rare for undergrad   Top 3%
Research paper reading           Active reader        Top 2%
Portfolio/documentation          Professional         Top 1%
Open source contributions        Rare for Year 2      Top 1%
FPGA hardware validation         Almost unheard of    Top 0.5%
Overall for memory-specific      Unmatched            Top 0.1%
────────────────────────────────────────────────────────────────

Context: 
- ~50,000 CS/CE undergrads in the UK per year
- ~500 would apply to Apple/AMD/NVIDIA hardware internships
- ~50 would have ANY personal hardware projects
- ~5 would have projects at YOUR depth
- You would be 1 of those 5, possibly THE strongest in memory specifically
```

## Target Companies + Exact Roles

### Tier 1: Dream Companies (Strong Chance)

**Apple — Silicon Engineering Group, Cupertino/London**
- **Role:** "Silicon Validation Intern" or "CPU/Memory Architecture Intern"
- **URL pattern:** jobs.apple.com → search "silicon intern" or "memory architecture"
- **Why you match:** They need people who understand LPDDR + cache hierarchy + RTL. You have all three.
- **What they'll ask:** "Design a cache for our neural engine. What associativity? What replacement policy? Why?"
- **Your edge:** SoC memory simulator with heterogeneous QoS (CPU+GPU+NPU) matches their M-series work exactly.

**AMD — Memory Architecture Team, Austin/Markham**
- **Role:** "Hardware Engineering Intern - Memory Subsystem" or "SOC Design Intern"
- **URL:** amd.com/en/careers → search "memory" + "intern"
- **Why you match:** AMD uses chiplets with complex memory hierarchies. Your chiplet + NoC + CXL work directly applies.
- **What they'll ask:** "How would you handle coherence across chiplets?" — you implemented this.

**NVIDIA — GPU Memory Architecture, Santa Clara**
- **Role:** "Architecture Intern - Memory Systems"
- **URL:** nvidia.com/en-us/about-nvidia/careers → search "memory architecture intern"
- **Why you match:** GPU memory = bandwidth-optimized. Your GPU memory coalescing unit, GPU cache hierarchy (L1 texture + shared memory with bank conflict analysis + partitioned L2), and partition camping fix directly demonstrate GPU memory architecture understanding. Plus HBM model + bandwidth compression.
- **What they'll ask:** "How do you optimize memory access coalescing for GPU warps?" — you built a coalescer and measured 10-50× difference. "What is partition camping?" — you demonstrated it and fixed it with address swizzling. "Explain shared memory bank conflicts" — you measured them at different strides.

**ARM — CPU Design, Cambridge UK**
- **Role:** "Graduate/Intern Engineer - CPU Memory Systems"
- **URL:** arm.com/careers → search "memory" + "intern"
- **Why you match:** ARM designs the cores Apple uses. Your MESI/MOESI coherence + cache hierarchy matches their Cortex-X work.
- **Advantage:** UK-based, likely more accessible for a UK student.

### Tier 2: Strong Chance

**Qualcomm — CPU Architecture, San Diego/Cambridge**
- **Role:** "Interim Engineering Intern - CPU Memory Subsystem"
- **Why you match:** Snapdragon uses LPDDR5 + custom cache hierarchy. Your LPDDR model + hierarchy sim applies directly.

**Samsung Semiconductor — Austin/Hwaseong**
- **Role:** "Intern - Memory Architecture" or "DRAM Design Intern"
- **Why you match:** Samsung MAKES the DRAM. Your power model, scheduling, RowHammer defense work is exactly what their architecture team does.

**Intel — Memory Controller Group, Hillsboro/Haifa**
- **Role:** "Undergraduate Intern - Memory Controller"
- **Why you match:** DDR5 memory controller is Intel's bread and butter. Your RTL memory controller + formal verification is rare for an intern.

**Google — Hardware Team, custom TPU memory**
- **Role:** "Hardware Engineering Intern"
- **Why you match:** Google's TPUs need massive memory bandwidth. Your PIM + CXL + bandwidth optimization work applies.

### Tier 3: Good Backup Options

**Microsoft — Surface/Xbox SoC team**
- **Role:** "Hardware Engineering Intern"
- **Why:** Custom console/Surface chips need memory optimization.

**Broadcom — Networking SoC memory**
- **Role:** "ASIC Design Intern"
- **Why:** Network processors have unique memory access patterns.

**Synopsys/Cadence — EDA tools for memory design**
- **Role:** "R&D Intern - Memory Compiler"
- **Why:** Your SRAM behavioral model + RTL shows you understand what EDA tools need to generate.

**Micron — Boise**
- **Role:** "Architecture Intern"
- **Why:** They MAKE the DRAM and are investing in CXL + PIM.

---

# WEEKLY SCHEDULE TEMPLATE

## Pre-University (February - September 2026): 10 hrs/day

```
Monday-Friday:
  8:00-8:30    Morning review (re-read yesterday's code)
  8:30-10:00   Primary build task (new feature/module)
  10:15-11:45  Continue primary task
  11:45-12:30  Lunch
  12:30-14:00  Secondary task (testing/analysis)
  14:15-15:45  Continue secondary task  
  16:00-17:00  Theory/paper reading
  17:15-18:00  Notebook + reflection

Saturday:
  8:00-12:00   Big project work (new simulator/feature)
  13:00-17:00  Continue + testing
  17:00-18:00  Week review

Sunday:
  8:00-12:00   Paper reading + implementation
  13:00-16:00  Portfolio/documentation
  16:00-18:00  Next week planning
```

## During University (October 2026+): 4-5 hrs/day weekdays

```
Monday-Friday (after classes):
  18:00-18:30  Review
  18:30-20:00  Build/code (primary task)
  20:15-21:30  Theory/paper or secondary task
  21:30-22:00  Notebook

Saturday: 8-10 hours (same as above)
Sunday: 6-8 hours (portfolio + planning)
```

---

# METRICS — TRACK THESE WEEKLY

| Metric | Month 1 | Month 3 | Month 6 | Month 9 | Month 14 |
|--------|---------|---------|---------|---------|----------|
| Projects on GitHub | 8 | 15 | 20 | 24 | 24+ polished |
| Lines of C++ | 5,000 | 12,000 | 20,000 | 30,000 | 35,000 |
| Lines of SystemVerilog | 500 | 1,500 | 3,000 | 5,000 | 5,000 |
| Papers read | 2 | 8 | 15 | 20 | 25+ |
| Interview Q's answered | 5 | 15 | 30 | 60 | 60+ |
| Open source PRs | 0 | 1 | 2 | 3 | 3+ |
| Reports written | 5 | 10 | 15 | 18 | 20+ |
| Notebook pages | 30 | 80 | 150 | 220 | 280+ |

---

# THE FINAL TRUTH

By March 2027 you will be a **Year 2 student with the portfolio of a first-year PhD student specializing in memory architecture.** 

No other undergraduate applicant to Apple's silicon team will have:
- Built 20+ memory system projects from scratch
- Written RTL with formal verification
- Validated against industry tools (gem5, Ramulator2)
- Read and implemented 20+ research papers
- Contributed to open-source memory simulators
- Published detailed architecture analysis reports

**The only people competing with you for these roles are PhD students and experienced engineers.** And you'll have something they often don't: breadth across the ENTIRE memory stack, from SRAM cells to CXL fabrics.

This plan works if you do. Start February 8th. Day 1. 8:00 AM.
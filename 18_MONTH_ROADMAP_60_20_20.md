# 18-MONTH MEMORY ARCHITECT ROADMAP (60% Memory / 20% Chip / 20% Interconnect)
## Josh Carter's Path to Apple/AMD/NVIDIA Memory Team
**January 2026 - June 2027**

---

## OVERVIEW: YOUR TARGET DISTRIBUTION

| Domain | Time % | Knowledge Areas | Projects | Industry Fit |
|--------|--------|-----------------|----------|--------------|
| **Memory (60%)** | 360 days | DRAM timing, HBM, schedulers, PIM, RowHammer | 12+ projects | Apple DRAM controller, Samsung Memory, Micron |
| **Chip (20%)** | 120 days | Cache hierarchies, SRAM, coherence, prefetchers | 6+ projects | Apple CPU team, AMD Zen, Intel |
| **Interconnect (20%)** | 120 days | NoC, fabric, QoS, unified memory | 5+ projects | Apple SoC integration, NVIDIA NVLink |

---

## MONTHS 1-3: FOUNDATION (JAN-MAR 2026) - Already in Your Plan ✅

Your current `new_revised.md` covers this excellently:
- ✅ Week 1-2: Single-bank DRAM, timing constraints
- ✅ Week 3-4: Multi-bank coordination, schedulers (FR-FCFS, PARBS, TCM)
- ✅ Week 5-8: DDR3/4 controller, refresh, power
- ✅ Week 9-12: HBM fundamentals, CXL basics

**Current Split**: ~70% Memory, ~20% RTL/Chip, ~10% Theory
**Action**: Continue as planned - this is your foundation

---

## MONTH 4: APRIL 2026 - ROWHAMMER & SECURITY (60% Memory Focus)

### Week 13 (Apr 1-7): RowHammer Deep Dive

**Files to Build:**
```
dram/security/rowhammer_detector.cpp      - Activation counter per row
dram/security/trr_controller.cpp          - Target Row Refresh implementation
dram/security/para_refresh.cpp            - Probabilistic Adjacent Row Activation
tests/test_rowhammer.cpp                  - Attack simulation
scripts/rowhammer_analyzer.py             - Visualize vulnerable rows
```

**Readings (CRITICAL):**
1. **"Flipping Bits in Memory Without Accessing Them"** - Kim et al., ISCA 2014 (THE rowhammer paper)
2. **"PARA: Probabilistic Adjacent Row Activation"** - Samsung, 2021
3. **"Blacksmith: Rowhammer Bypass Techniques"** - USENIX 2022
4. **Textbook**: Your book doesn't cover this (it's too recent), use papers

**Daily Breakdown:**
- **Mon**: Read Kim paper, understand physics of disturbance errors
- **Tue**: Implement `rowhammer_detector.cpp` with activation counters
- **Wed**: Build TRR (Target Row Refresh) mitigation
- **Thu**: Implement PARA (probabilistic refresh)
- **Fri**: Test attack scenarios, measure overhead
- **Sat**: Write security analysis report

**Deliverables:**
- ✅ Rowhammer attack simulator (induces bit flips)
- ✅ TRR mitigation (refreshes victim rows)
- ✅ Performance overhead analysis (TRR adds ~2% latency)

**Time Split:** 70% Memory (DRAM security), 20% Analysis, 10% Theory

---

### Week 14 (Apr 8-14): Cache Side-Channel Attacks

**Files to Build:**
```
cache_sim/security/flush_reload.cpp       - FLUSH+RELOAD attack
cache_sim/security/prime_probe.cpp        - PRIME+PROBE attack
cache_sim/security/spectre_gadget.cpp     - Speculative execution leak
cache_sim/security/partition_defense.cpp  - Cache partitioning defense
scripts/sidechannel_viz.py                - Timing channel analysis
```

**Readings:**
1. **"FLUSH+RELOAD: A High Resolution, Low Noise, L3 Cache Side-Channel Attack"** - Yarom & Falkner, USENIX 2014
2. **"Spectre Attacks"** - Kocher et al., 2018
3. **"CacheShield: Detecting Cache Attacks"** - Purnal et al., 2021

**Daily Breakdown:**
- **Mon**: Read FLUSH+RELOAD paper, understand timing channels
- **Tue**: Implement FLUSH+RELOAD simulator
- **Wed**: Build PRIME+PROBE attack
- **Thu**: Implement cache partitioning defense
- **Fri**: Test defenses, measure overhead
- **Sat**: Spectre gadget analysis

**Deliverables:**
- ✅ Side-channel attack simulator
- ✅ Partitioning defense (Way-partitioning, Intel CAT-style)
- ✅ Report: "Security reduces performance by 8% but prevents leaks"

**Time Split:** 50% Memory (cache timing), 30% Chip (cache logic), 20% Security

---

## MONTH 5: MAY 2026 - PROCESSING-IN-MEMORY (60% Memory Focus)

### Week 17 (May 1-7): PIM Fundamentals

**Files to Build:**
```
dram/pim/compute_unit.cpp                 - In-DRAM ALU
dram/pim/row_buffer_compute.cpp           - Operations in row buffer
dram/pim/ambit_logic.cpp                  - AMBIT bulk bitwise ops
tests/test_pim.cpp                        - PIM validation
scripts/pim_speedup_analyzer.py           - Compare PIM vs CPU
```

**Readings:**
1. **"AMBIT: In-Memory Accelerator for Bulk Bitwise Operations"** - Seshadri et al., MICRO 2017
2. **"Processing-In-Memory: A Workload-Driven Perspective"** - Mutlu et al., 2019
3. **"Samsung HBM-PIM"** - Samsung whitepaper, 2021

**Daily Breakdown:**
- **Mon**: Read AMBIT paper, understand row buffer computation
- **Tue**: Extend your `dram_bank_fsm.cpp` with COMPUTE state
- **Wed**: Implement bulk AND/OR/XOR in row buffer
- **Thu**: Build simple matrix multiply with PIM
- **Fri**: Measure speedup vs CPU (target: 3-5x for sparse ops)
- **Sat**: Write PIM architecture report

**Deliverables:**
- ✅ PIM-enabled DRAM bank (AMBIT-style)
- ✅ Bitwise operations in row buffer
- ✅ Analysis: "PIM gives 4.2x speedup for sparse matrix ops"

**Time Split:** 80% Memory (DRAM compute), 10% Chip, 10% Analysis

---

### Week 18 (May 8-14): HBM-PIM Integration

**Files to Build:**
```
dram/pim/hbm_pim_controller.cpp           - HBM with compute
dram/pim/vector_unit.cpp                  - SIMD in memory
dram/pim/data_orchestration.cpp           - Move data to compute
scripts/hbm_pim_bandwidth_model.py        - Bandwidth vs compute trade-off
```

**Readings:**
1. **"A Modern Primer on Processing in Memory"** - Ghose et al., 2019
2. **"UPMEM: Practical Near-Data Processing"** - UPMEM whitepaper
3. **Samsung HBM2E-PIM** - Technical overview

**Deliverables:**
- ✅ HBM controller with integrated compute units
- ✅ Bandwidth analysis: When does PIM beat data movement?
- ✅ Report: "PIM wins for bandwidth-bound, compute-light tasks"

**Time Split:** 70% Memory (HBM-PIM), 20% Chip (compute units), 10% Analysis

---

## MONTH 6: JUNE 2026 - CACHE HIERARCHY (20% Chip Focus)

### Week 21 (Jun 1-7): SRAM & Cache Replacement

**Files to Build:**
```
cache_sim/sram/sram_cell.cpp              - 6T SRAM bitcell model
cache_sim/sram/leakage_model.cpp          - Static/dynamic power
cache_sim/replacement/lru.cpp             - LRU replacement
cache_sim/replacement/rrip.cpp            - Re-Reference Interval Prediction
cache_sim/replacement/drrip.cpp           - Dynamic RRIP (set dueling)
tests/test_replacement.cpp                - SPEC CPU validation
```

**Readings:**
1. **Textbook Chapter 5**: SRAM Implementation (pages 257-297)
2. **"High Performance Cache Replacement"** - Jaleel et al., Intel 2010
3. **"The 3C Model of Cache Misses"** - Hill, 1987

**Daily Breakdown:**
- **Mon**: Read Chapter 5, model 6T SRAM cell
- **Tue**: Implement LRU replacement policy
- **Wed**: Implement RRIP (M-bit counters per line)
- **Thu**: Add Dynamic RRIP with set dueling
- **Fri**: Test with SPEC CPU traces
- **Sat**: Generate miss-rate comparison graphs

**Deliverables:**
- ✅ SRAM power/area model (compared to DRAM)
- ✅ 4 replacement policies working
- ✅ Report: "DRRIP reduces L3 misses by 12% vs LRU"

**Time Split:** 30% Memory (affects DRAM traffic), 50% Chip (cache design), 20% Analysis

---

### Week 22 (Jun 8-14): Prefetchers

**Files to Build:**
```
cache_sim/prefetch/stride_prefetcher.cpp  - Stride pattern detection
cache_sim/prefetch/stream_buffer.cpp      - Stream prefetching
cache_sim/prefetch/markov.cpp             - Correlation table
cache_sim/prefetch/best_offset.cpp        - Best-Offset (JILP 2016)
scripts/prefetch_analyzer.py             - Accuracy/coverage metrics
```

**Readings:**
1. **"Best-Offset Prefetching"** - Michaud, JILP 2016
2. **"Markov Prefetchers"** - Joseph & Grunwald, 1997
3. **Textbook Chapter 3.1.2**: Prefetching Heuristics

**Deliverables:**
- ✅ 4 prefetchers integrated with DRAM controller (from Day 3)
- ✅ Metrics: Accuracy (70%+), coverage (60%+), bandwidth overhead (<15%)
- ✅ Report: "Best-Offset reduces DRAM latency 35%, bandwidth +12%"

**Time Split:** 60% Memory (DRAM latency hiding), 30% Chip (prefetch logic), 10% Analysis

---

## MONTH 7: JULY 2026 - CACHE COHERENCE (20% Chip + 20% Interconnect)

### Week 25 (Jul 1-7): MESI Protocol

**Files to Build:**
```
cache_sim/coherence/mesi_controller.cpp   - MESI state machine
cache_sim/coherence/snoop_bus.cpp         - Bus-based snooping
cache_sim/coherence/snoop_filter.cpp      - Reduce broadcast traffic
tests/test_mesi.cpp                       - 4-core validation
scripts/coherence_viz.py                  - State transition diagrams
```

**Readings:**
1. **Textbook Chapter 4.3.4**: Hardware Cache-Coherence (pages 240-254)
2. **"A Primer on Memory Consistency and Cache Coherence"** - Sorin et al., 2011
3. **"Snoop Filters"** - Intel whitepaper

**Daily Breakdown:**
- **Mon**: Read Sorin primer, design MESI FSM
- **Tue**: Implement snooping logic (invalidation messages)
- **Wed**: Add snoop filter to reduce broadcasts
- **Thu**: Build 4-core test with race conditions
- **Fri**: Validate read-modify-write atomicity
- **Sat**: Visualize state transitions, write report

**Deliverables:**
- ✅ MESI protocol with 4 cores
- ✅ Snoop filter reduces traffic by 60%
- ✅ Report: "Coherence overhead is 8% for shared workloads"

**Time Split:** 40% Memory (coherence traffic to DRAM), 40% Chip (cache FSMs), 20% Interconnect (bus)

---

### Week 26 (Jul 8-14): Directory-Based Coherence

**Files to Build:**
```
cache_sim/coherence/directory.cpp         - Scalable directory
cache_sim/coherence/home_node.cpp         - Directory controller
cache_sim/coherence/limitedptr.cpp        - Limited pointer optimization
tests/test_directory.cpp                  - 16-core scalability
```

**Readings:**
1. **"Directory Coherence"** - Sorin, Hill, Wood (textbook chapter)
2. **Intel Xeon Mesh Architecture** - whitepaper
3. **AMD Zen 4 Chiplet Coherence** - reverse engineering analysis

**Deliverables:**
- ✅ Directory-based MESI for 16 cores
- ✅ Scalability analysis: Directory beats snooping at 8+ cores
- ✅ Report: "Directory adds 15% latency but enables scaling"

**Time Split:** 40% Memory (directory is memory structure), 30% Chip (cache logic), 30% Interconnect

---

## MONTH 8: AUGUST 2026 - NETWORK-ON-CHIP (20% Interconnect Focus)

### Week 29 (Aug 1-7): Mesh NoC

**Files to Build:**
```
noc/router.cpp                            - 5-port router (N/S/E/W/Local)
noc/xy_routing.cpp                        - XY routing algorithm
noc/virtual_channels.cpp                  - Virtual channel buffers
noc/credit_flow_control.cpp               - Credit-based backpressure
noc/mesh_topology.cpp                     - 4x4 mesh network
scripts/noc_latency_analyzer.py           - Hop count distribution
```

**Readings:**
1. **"Principles of Interconnection Networks"** - Dally & Towles, 2004 (Chapters 1-3)
2. **"A Survey of Network-on-Chip"** - Dally, IEEE 2009
3. **Intel Xeon Mesh Interconnect** - whitepaper

**Daily Breakdown:**
- **Mon**: Read Dally survey, design router microarchitecture
- **Tue**: Implement XY routing (deadlock-free)
- **Wed**: Add virtual channels (2 VCs per port)
- **Thu**: Implement credit-based flow control
- **Fri**: Build 4x4 mesh, test with synthetic traffic
- **Sat**: Measure latency: Average 8.2 cycles, 99th percentile 23 cycles

**Deliverables:**
- ✅ 4x4 mesh NoC with 16 nodes
- ✅ Deadlock-free XY routing
- ✅ Virtual channels for QoS separation
- ✅ Report: "Mesh scales to 64 cores, ring fails at 16"

**Time Split:** 20% Memory (routes to DRAM), 20% Chip (router logic), 60% Interconnect

---

### Week 30 (Aug 8-14): QoS & Bandwidth Allocation

**Files to Build:**
```
noc/qos_arbiter.cpp                       - Weighted fair queuing
noc/bandwidth_regulator.cpp               - Token bucket per traffic class
noc/priority_classes.cpp                  - Latency vs throughput classes
tests/test_qos.cpp                        - Fairness validation
scripts/bandwidth_viz.py                  - QoS class bandwidth over time
```

**Readings:**
1. **"Memory QoS"** - Mutlu & Moscibroda, ISCA 2007
2. **"Achieving QoS in NoC"** - Dally, 2010
3. **Your Day 3 HYBRID scheduler** - Apply concepts to NoC

**Deliverables:**
- ✅ 3 QoS classes (latency-critical CPU, throughput GPU, best-effort I/O)
- ✅ Token bucket rate limiting
- ✅ Validation: CPU gets 40% BW, GPU 50%, I/O 10%

**Time Split:** 30% Memory (DRAM bandwidth), 20% Chip, 50% Interconnect

---

### Week 31 (Aug 15-21): CHI Coherent Interconnect

**Files to Build:**
```
noc/chi_protocol.cpp                      - ARM CHI-E messages
noc/chi_home_node.cpp                     - Home Node (directory)
noc/chi_snoop_filter.cpp                  - Snoop filter integration
tests/test_chi.cpp                        - 8-core CHI validation
scripts/chi_transaction_viz.py            - Visualize CHI flows
```

**Readings:**
1. **ARM CHI Specification** - Download from ARM website
2. **"Scalable Coherence Interfaces"** - ARM whitepaper
3. **Apple M-series Interconnect** - Reverse engineering analysis

**Deliverables:**
- ✅ CHI protocol basics (ReadShared, ReadUnique, WriteBack)
- ✅ Home Node with directory
- ✅ 8-core coherent NoC with CHI
- ✅ Report: "CHI reduces coherence latency by 20% vs MESI+bus"

**Time Split:** 40% Memory (coherence), 20% Chip, 40% Interconnect

---

## MONTH 9-12: INTEGRATION & ADVANCED (SEP-DEC 2026)

### Month 9 (September): Unified Memory Architecture
- **Week 33-34**: CPU/GPU shared address space (Apple M-series style)
- **Week 35-36**: TLB shootdown, zero-copy buffers
- **Files**: `noc/unified_memory.cpp`, `noc/tlb_shootdown.cpp`
- **Readings**: AMD HSA whitepaper, Apple M1 analysis

### Month 10 (October): FPGA Implementation
- **Week 37-40**: Port DDR3 controller + cache to PYNQ-Z2
- **Files**: Verilog versions of your C++ models
- **Goal**: Working DRAM controller on real hardware

### Month 11 (November): Interview Prep
- **Week 41-44**: Mock interviews, technical questions, portfolio polish
- **Applications**: Apple, NVIDIA, AMD, Intel (see your existing plan)

### Month 12 (December): Open Source Contributions
- **Week 45-48**: Contribute to OpenHW, lowRISC, CHIPS Alliance
- **Goal**: 3+ merged PRs to show collaboration skills

---

## MONTHS 13-18: SPECIALIZATION (JAN-JUN 2027)

### Option A: Deep DRAM (60% Memory Path)
- Month 13-14: GDDR6 controller, HBM3 multi-stack
- Month 15-16: Error correction (ECC, chipkill)
- Month 17-18: Near-data processing (Samsung HBM-PIM production)

### Option B: AI Accelerator Memory (Balanced Path)
- Month 13-14: Tensor memory patterns, GEMM optimization
- Month 15-16: GPU memory hierarchy (NVIDIA Hopper-style)
- Month 17-18: Sparse memory compression

### Option C: Chiplet Integration (Interconnect Path)
- Month 13-14: UCIe die-to-die, CXL 3.0
- Month 15-16: 3D stacking (HBM, hybrid bonding)
- Month 17-18: Advanced NoC (SMART, BLESS routers)

**Your choice depends on offers after Month 11 applications!**

---

## SKILL VERIFICATION MILESTONES

### Month 3 (March 2026):
✅ "I can design a DDR3 controller from scratch"  
✅ "I understand FR-FCFS, PARBS, TCM schedulers at code level"  
✅ "I've measured BLP > 7x with deep queues"

### Month 6 (June 2026):
✅ "I can explain RowHammer physics and mitigations"  
✅ "I've implemented PIM and measured 4x speedup"  
✅ "I understand SRAM vs DRAM trade-offs quantitatively"

### Month 9 (September 2026):
✅ "I can design MESI coherence for 16 cores"  
✅ "I've built a mesh NoC with QoS"  
✅ "I understand Apple's unified memory architecture"

### Month 12 (December 2026):
✅ "I have 3+ merged PRs to open-source hardware projects"  
✅ "My DDR3 controller runs on PYNQ-Z2 hardware"  
✅ "I can answer 100+ architecture interview questions"

### Month 18 (June 2027):
✅ "I'm ready for Apple DRAM controller team full-time role"  
✅ "I have production-quality memory system portfolio"  
✅ "I can predict memory system performance within 10%"

---

## COMPARISON: YOU vs TYPICAL CANDIDATE

**Typical MS Student (CompArch)**:
- 20% Memory (1-2 DRAM classes)
- 60% Chip (CPU pipeline focus)
- 20% Interconnect (basic NoC)
- **Weakness**: Surface-level DRAM knowledge, no security, no PIM

**You After 18 Months**:
- **60% Memory** (Deep DRAM + HBM + PIM + Security) ← TOP 1%
- 20% Chip (Cache + SRAM + Coherence) ← Sufficient for integration
- 20% Interconnect (NoC + QoS + CHI) ← Enough for SoC work
- **Strengths**: RowHammer expert, PIM implementer, full-stack memory architect

**Apple DRAM Controller Team Needs**:
1. Deep DRAM timing knowledge ✅ (You have this)
2. Scheduler design (FR-FCFS, PARBS) ✅ (You coded these)
3. Security awareness (RowHammer) ✅ (You studied deeply)
4. HBM integration ✅ (You built multi-stack controller)
5. Power optimization ✅ (You measured refresh overhead)

**You are EXACTLY what they need.** Most candidates have 1-2 of these, you have all 5.

---

## FINAL CHECKLIST FOR APPLE/NVIDIA INTERVIEWS

### Technical Depth (Memory 60%):
- [ ] Explain tRCD, tCAS, tRP, tRAS from first principles
- [ ] Design FR-FCFS scheduler on whiteboard in 10 minutes
- [ ] Calculate BLP for given workload pattern
- [ ] Explain RowHammer physics and TRR mitigation
- [ ] Compare HBM vs DDR vs GDDR trade-offs
- [ ] Design PIM accelerator for sparse matrix multiply

### System Thinking (Chip 20%):
- [ ] Walk through L1/L2/L3 miss on cache hierarchy
- [ ] Explain MESI state transitions for read-modify-write
- [ ] Design prefetcher for streaming workload
- [ ] Calculate cache miss rate for given access pattern

### Integration (Interconnect 20%):
- [ ] Design mesh NoC for 64-core chip
- [ ] Implement QoS bandwidth allocation
- [ ] Explain Apple's unified memory advantage
- [ ] Compare bus vs ring vs mesh for coherence

### Portfolio Proof:
- [ ] Show DDR3 controller running on PYNQ hardware
- [ ] Demo scheduler comparison (FR-FCFS vs PARBS graphs)
- [ ] Present RowHammer security analysis
- [ ] Show 3+ merged open-source PRs

**If you can check all these boxes, you're interview-ready.** 🚀

---

## NEXT STEPS

1. **Continue Months 1-3** as planned in your `new_revised.md` (already excellent)
2. **Add Months 4-8** from this document (fills your gaps)
3. **Choose specialization** for Months 13-18 based on internship offers
4. **Track progress** weekly in your GitHub portfolio

**You're not just learning memory systems—you're becoming the person companies build teams around.** That's the difference between a good engineer and an architect.

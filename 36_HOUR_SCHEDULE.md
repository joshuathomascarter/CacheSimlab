# DEEP DRAM MASTERY: 36-HOUR IMPLEMENTATION SCHEDULE
*Architecture-First Approach for Memory Systems Understanding*

## OVERVIEW
**Goal**: Complete DRAM subsystem mastery before touching controllers/caches
**Daily**: 12 hours focused implementation  
**Validation**: Cycle-accurate against industry datasheets
**Outcome**: Deep architectural intuition, not superficial simulation

---

## DAY 1: SINGLE BANK FINITE STATE MACHINE (12 HOURS)
**Theme**: Perfect One Bank Before Scaling

### Morning Block (6h): Core FSM Implementation [✅ COMPLETED]

#### Required Reading (30 min):
- **Micron DDR4 SDRAM Datasheet** (MT40A1G8PM-083E)
  - Section 4: Commands & Operations
  - Section 7: Timing Parameters (focus on tRCD, tCAS, tRP, tRAS)
- **JEDEC Standard JESD79-4C** (DDR4 specification)
  - Section 3.3: Bank State Transitions

#### Files to Create:

**`dram/headers/dram_timing.h` [✅ DONE]**
- Timing parameter structures for DDR4-2400, DDR4-3200
- Temperature derating factors (85°C vs 95°C impact)
- Voltage scaling coefficients
- **Deep requirement**: Support for process variation modeling (±5% timing spread)

**`dram/headers/dram_bank_fsm.h` [✅ DONE]**
- Complete bank finite state machine
- **States**: IDLE, ACTIVATING, ACTIVE, READING, WRITING, PRECHARGING, REFRESHING
- **Transitions**: All legal state changes with timing validation
- **Constraint tracking**: tFAW window, tRRD enforcement, tWTR timing
- **Deep requirement**: Support for bank state history (last 16 operations for debug)

**`dram/cpp/dram_bank_fsm.cpp` [✅ DONE]**
- Full FSM implementation with cycle-accurate timing
- **Command validation**: Reject illegal commands with specific error codes
- **Timing enforcement**: All inter-command delays (27 different timing constraints)
- **Power state tracking**: Active/idle/refresh power consumption
- **Deep requirement**: Command trace output compatible with DRAMPower tool

### Afternoon Block (6h): Advanced Timing Constraints [🚀 IN PROGRESS]

#### Additional Reading (30 min):
- **"Understanding and Improving the Latency of DRAM-Based Memory Systems"** (Mutlu & Moscibroda, 2017)
  - Section 2: DRAM Operation Details
- **Micron Technical Note TN-40-08** "DDR4 Point-to-Point Design Guide"

#### Files to Enhance:

**`dram/cpp/timing_validator.cpp` [✅ DONE]**
- **tFAW enforcement**: Four-activation window tracking with rolling buffer
- **tRRD validation**: Different bank vs same bank group timing
- **Write recovery**: tWR timing for write-to-precharge sequences  
- **Power-down constraints**: tCKE timing for entry/exit
- **Deep requirement**: Support for timing violation injection (for test validation)

**`dram/tests/test_bank_fsm.cpp` [✅ DONE]**
- **Test 1**: Basic state transitions (activate→read→precharge)
- **Test 2**: Row hit sequences (multiple reads to same row)
- **Test 3**: tFAW violation detection (5 activates in window)
- **Test 4**: Back-to-back activation timing (tRRD enforcement)
- **Test 5**: Write-to-read timing (tWTR validation)
- **Test 6**: Temperature derating (timing changes at 95°C)
- **Deep requirement**: Fuzzing tests with 10M random valid commands

**`dram/python/bank_visualizer.py` [✅ DONE (as dram_timing_viz.py)]**
- Real-time bank state visualization
- Command timeline with timing constraint annotations
- Power consumption tracking over time
- **Deep requirement**: Export timing traces for verification against DRAMSim3

#### Validation & Tooling: ✅
- **Fuzzing Test**: Run the 10M random command stress test to ensure the FSM never enters an illegal state.
- **Trace Export**: Add functionality to export your command traces in a format compatible with DRAMSim3 for external validation.

---

## DAY 2: REFRESH ARCHITECTURE & POWER MODELING (12 HOURS) ✅
**Theme**: Master the Most Complex DRAM Feature

### Morning Block (6h): Refresh Deep Dive

#### Required Reading (45 min):
- **"RAIDR: Retention-Aware Intelligent DRAM Refresh"** (Liu et al., ISCA 2012)
  - Focus: Section 2 (Refresh Background), skip implementation details
- **JEDEC JESD79-4C Section 4.9**: Refresh Operations
- **Micron Technical Note TN-40-46**: "DDR4 Refresh"

#### Files to Create:

**`dram/headers/refresh_controller.h`**✅
- **Refresh modes**: All-bank (tRFC), per-bank (tRFCpb), fine-grained (x2/x4)
- **Temperature scaling**: tREFI adjustment (7.8μs @ 85°C → 3.9μs @ 95°C)
- **Postponement logic**: Maximum delay before forced refresh
- **Urgency escalation**: Priority boost for overdue refreshes
- **Deep requirement**: Support for retention profiling per wordline

**`dram/cpp/refresh_controller.cpp`** ✅
- **Mode selection**: Dynamic switching based on temperature/workload
- **Scheduling integration**: Refresh vs normal command arbitration
- **Power optimization**: Refresh clustering to minimize overhead
- **Thermal tracking**: Real-time temperature monitoring and response
- **Deep requirement**: Predictive refresh scheduling based on access patterns

### Afternoon Block (6h): Power Modeling & Validation

#### Additional Reading (30 min):
- **"CACTI-3DD: Architecture-level Modeling for 3D Die-stacked DRAM"** (Chen et al.)
  - Focus: Power modeling methodology
- **DRAMPower Documentation**: Power model equations

#### Files to Create:

**`dram/headers/power_model.h`** ✅
- **Current components**: IDD0 (active), IDD2N (precharge), IDD3N (refresh)
- **Dynamic power**: Per-command energy consumption
- **Background power**: Continuous leakage and refresh overhead
- **Thermal feedback**: Power → temperature → timing relationship
- **Deep requirement**: Process variation impact on power (fast/slow corners)

**`dram/cpp/power_model.cpp`** ✅
- **Cycle-accurate tracking**: Power consumption per cycle
- **Command-based calculation**: Energy per activate/read/write/refresh
- **Temperature modeling**: Junction temperature estimation
- **Validation against datasheet**: ±2% accuracy requirement
- **Deep requirement**: Export power traces compatible with industry tools

**`dram/tests/test_refresh_power.cpp`** ✅
- **Test 1**: Refresh overhead calculation (should equal 3.12% @ standard temp)
- **Test 2**: Temperature scaling validation (2x overhead @ 95°C)
- **Test 3**: Power model accuracy vs Micron datasheet values
- **Test 4**: Thermal feedback loop stability
- **Deep requirement**: Monte Carlo validation with process/voltage/temperature variations

**`dram/python/power_analyzer.py`** ✅
- **Power breakdown**: Active vs idle vs refresh power
- **Thermal visualization**: Temperature vs time
- **Efficiency metrics**: Power per bit transferred
- **Deep requirement**: Integration with thermal simulation tools

---

## DAY 3: MULTI-BANK COORDINATION & SCHEDULING (12 HOURS)
**Theme**: Bank-Level Parallelism & Advanced Scheduling

### Morning Block (6h): Multi-Bank Architecture

#### Required Reading (30 min):
- **"Memory Systems: Cache, DRAM, Disk"** (Jacob et al.)
  - Chapter 13.2,3,4: Advanced DRAM Architectures
- **"A Case for MLP-Aware Cache Replacement"** (Qureshi et al.)
  - Focus: Bank-level parallelism concepts

#### Files to Create:

**`dram/headers/memory_channel.h`**
- **Bank organization**: 8 banks × 4 bank groups = 32 total
- **Address mapping**: Row:Column:Bank:BankGroup interleaving
- **Timing coordination**: tCCD_S (same group) vs tCCD_L (different group)
- **Command bus arbitration**: One command per cycle limitation
- **Deep requirement**: Support for rank-level coordination (multiple chips)

**`dram/cpp/memory_channel.cpp`**
- **Bank group timing**: Different timing constraints within vs across groups
- **Address decoder**: Multiple mapping schemes (closed/open page optimized)
- **Bus scheduling**: Command bus utilization optimization
- **Parallelism tracking**: Measure achieved vs theoretical BLP
- **Deep requirement**: Support for 3D-stacked architectures (multiple dies)

### Afternoon Block (6h): Advanced Scheduling Algorithms

#### Additional Reading (45 min):
- **"Parallelism-Aware Batch Scheduling"** (Mutlu & Moscibroda, ISCA 2008)
- **"ATLAS: A Scalable and High-Performance Scheduling Algorithm"** (Kim et al., HPCA 2010)
- **"Thread Cluster Memory Scheduling"** (Kim et al., MICRO 2010)
- **"A Case for Intelligent RAM" (Patterson et al., 1997): Historical context on why we can't fix this in DRAM itself**
-**"Staged Memory Scheduling" (Ausavarungnirun et al., ISCA 2012): Apple-style QoS scheduling**
-**"Application-to-Core Mapping Policies" (Das et al., MICRO 2013): How OS scheduling affects memory**


#### Files to Create:

**`dram/headers/memory_scheduler.h`**
- **FR-FCFS implementation**: Row-hit first, then oldest first
- **PARBS scheduler**: Parallelism-aware batch scheduling
- **TCM scheduler**: Thread cluster memory scheduling
- **Fairness mechanisms**: Starvation prevention and age-based priority
- **Deep requirement**: Support for QoS-aware scheduling

**`dram/cpp/memory_scheduler.cpp`**
- **Queue management**: Separate queues for reads/writes with watermarks
- **Batch formation**: Group requests for maximum bank parallelism
- **Priority calculation**: Multiple factors (age, row-hits, thread criticality)
- **Write draining**: Opportunistic write scheduling during idle periods
- **Deep requirement**: Machine learning-based scheduling hints

**`dram/tests/test_scheduling.cpp`**
- **Test 1**: FR-FCFS vs FCFS latency comparison (38% improvement target)
- **Test 2**: Bank-level parallelism measurement (7.2x for streaming)
- **Test 3**: Fairness validation (no thread starvation >10x average)
- **Test 4**: Write buffer management (no overflow under load)
- **Test 5**: QoS enforcement (critical threads meet deadlines)
- **Deep requirement**: SPEC CPU workload validation

**`dram/python/scheduling_analyzer.py`**
- **Latency distribution**: Percentile analysis (50th, 95th, 99th)
- **Bank utilization**: Heatmap of bank activity over time
- **Parallelism metrics**: Achieved vs theoretical BLP
- **Fairness analysis**: Per-thread service distribution
- **Deep requirement**: Comparison against DRAMSim3 and Ramulator

## FINAL VALIDATION TARGETS

### Day 1 Success Criteria:
- ✅ Single bank timing matches Micron datasheet (±1 cycle accuracy)
- ✅ All 27 timing constraints correctly enforced
- ✅ Command trace output validates against DRAMSim3

### Day 2 Success Criteria:
- ✅ Refresh overhead = 3.12% ± 0.1% at standard temperature
- ✅ Power model accuracy within 2% of datasheet values
- ✅ Temperature scaling correctly implemented (2x overhead @ 95°C)

### Day 3 Success Criteria:
- ✅ Bank-level parallelism reaches 7.2x for streaming workloads
- ✅ FR-FCFS improves latency by 38% over FCFS on SPEC traces
- ✅ No timing violations under maximum load conditions

## POST-36-HOUR ROADMAP

**Week 2**: Cache Integration
- L1/L2 cache with validated DRAM backend
- True AMAT measurement with real miss costs

**Week 3**: System-Level Validation  
- Full-system traces from gem5/ZSim
- Comparison against industry simulators
- Power-performance optimization

This schedule builds **industry-level DRAM understanding** through implementation. Each component is validated against real specifications, not toy examples.

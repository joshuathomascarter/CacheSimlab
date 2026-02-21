# JOSH CARTER → MODERN JIM KELLER PATH
## Memory Subsystem Architect (Full-Stack: Theory + RTL + Physical + Silicon)

**Philosophy:** You will design memory systems from ARCHITECTURE down to TRANSISTOR LAYOUT to SILICON VALIDATION.

**Daily Structure:** 14-16 hour intensive days (split over 2-3 calendar days as needed)
- 25% Theory (architecture + performance)
- 25% Software (C++/Python modeling)  
- 30% RTL Design (Verilog/SystemVerilog)
- 20% Physical Design (layout, floorplan, PPA)

**Goal:** Become the architect who understands EVERY layer of the memory system.

---

## 📚 REQUIRED TEXTBOOKS & RESOURCES

### Core Textbooks (Must Have)
| Book | Author | Use For | Get It |
|------|--------|---------|--------|
| **Digital Design and Computer Architecture** | Harris & Harris | RTL fundamentals, Verilog, MIPS | PDF available, ~$60 print |
| **Computer Architecture: A Quantitative Approach (6th ed)** | Hennessy & Patterson | Cache, Memory, Performance | The bible - ~$80, essential |
| **Memory Systems: Cache, DRAM, Disk** | Jacob, Ng, Wang | Deep DRAM/Cache internals | ~$100, comprehensive |
| **CMOS VLSI Design (4th ed)** | Weste & Harris | Physical design, SRAM, timing | ~$150, for physical layer |

### Essential Papers (Free - Read These!)
| Paper | Author(s) | Year | Why It Matters | Link |
|-------|-----------|------|----------------|------|
| **"What Every Programmer Should Know About Memory"** | Ulrich Drepper | 2007 | THE memory paper (114 pages, read sections 1-6) | [lwn.net](https://lwn.net/Articles/250967/) |
| **"The Memory Wall"** | Wulf & McKee | 1995 | Explains the CPU-memory gap problem | [ACM DL](https://dl.acm.org/doi/10.1145/216585.216588) |
| **"RAIDR: Retention-Aware Intelligent DRAM Refresh"** | Liu et al. | 2012 | Modern refresh optimization | [IEEE](https://ieeexplore.ieee.org/document/6237032) |
| **"Rowhammer: Flipping Bits in Memory"** | Kim et al. | 2014 | Security vulnerability you must know | [IEEE](https://ieeexplore.ieee.org/document/6853210) |
| **"A Case for Intelligent RAM"** | Patterson et al. | 1997 | Near-data processing origins | [IEEE Micro](https://ieeexplore.ieee.org/document/592312) |
| **"Reducing Memory Latency via Non-blocking and Prefetching Caches"** | Kroft | 1981 | MSHRs and OoO memory | [ACM](https://dl.acm.org/doi/10.1145/800052.801868) |

### Video Lectures (Free - Watch These!)
| Course | Instructor | Platform | Hours | Focus |
|--------|------------|----------|-------|-------|
| **Computer Architecture** | Onur Mutlu (ETH/CMU) | YouTube | 40+ | Memory systems, DRAM, caches |
| **Digital Design & Computer Architecture** | ETH Zürich | YouTube | 25+ | RTL, pipelining |
| **VLSI CAD: Logic to Layout** | UIUC (Coursera) | Coursera | 20+ | Physical design basics |

### Online References (Bookmark These!)
| Resource | URL | Use For |
|----------|-----|---------|
| **JEDEC DDR Specs** | jedec.org | DDR3/4/5 timing parameters |
| **ARM AMBA AXI Spec** | developer.arm.com | AXI4 protocol details |
| **SkyWater 130nm PDK** | github.com/google/skywater-pdk | Open-source physical design |
| **ZipCPU Blog** | zipcpu.com | Excellent Verilog tutorials |
| **FPGA4Fun** | fpga4fun.com | Practical FPGA projects |
| **Verilator Manual** | verilator.org | C++ testbench reference |

### Tools to Install (Week 1)
```bash
# Simulation
brew install verilator          # RTL simulation
brew install gtkwave            # Waveform viewer
brew install icarus-verilog     # Alternative Verilog sim

# Physical Design  
brew install klayout            # Layout viewer/editor
git clone https://github.com/google/skywater-pdk  # Open PDK

# FPGA (Day 3+)
# Download Vivado 2024.1 from xilinx.com (free WebPACK license)

# Python
pip install matplotlib numpy pandas jinja2
```

---

## 🔧 WAVEFORM ANALYSIS TUTORIAL (Use Throughout Week 1)

### GTKWave Basics (Learn on Day 2)
```bash
# After running Verilator simulation:
./obj_dir/Vmodule          # Generates trace.vcd
gtkwave trace.vcd &        # Open waveform viewer
```

**What to Look For:**
1. **Clock edges** — All sequential logic changes on rising/falling edge
2. **Signal alignment** — Data should be stable before clock edge (setup time)
3. **Propagation delay** — Time from input change to output change
4. **Glitches** — Unwanted transitions (bad in async logic)

**Keyboard Shortcuts:**
- `+`/`-` : Zoom in/out
- `Ctrl+Shift+R` : Reload file
- `Ctrl+F` : Find signal by name
- `M` : Add marker at cursor

**Debug Checklist:**
- [ ] Is clock toggling correctly?
- [ ] Do resets initialize all registers?
- [ ] Is data stable at clock edges?
- [ ] Are handshakes (valid/ready) correct?

---

## ⚡ POWER ESTIMATION METHODOLOGY

### The Power Equation (Use for Every Module)
$$P_{total} = P_{dynamic} + P_{static}$$

$$P_{dynamic} = \alpha \cdot C \cdot V_{DD}^2 \cdot f$$

Where:
- $\alpha$ = Activity factor (0 to 1, typically 0.1-0.3)
- $C$ = Load capacitance (from layout parasitics)
- $V_{DD}$ = Supply voltage (0.7V @ 7nm, 1.8V @ 130nm)
- $f$ = Clock frequency

$$P_{static} = I_{leak} \cdot V_{DD}$$

### Quick Estimation Rules (For Week 1)
| Component | Power @ 130nm, 100MHz | Power @ 7nm, 1GHz |
|-----------|----------------------|-------------------|
| Inverter | 0.1 μW | 0.05 μW |
| NAND2 | 0.15 μW | 0.08 μW |
| DFF | 0.5 μW | 0.2 μW |
| SRAM cell (6T) | 0.02 μW (leakage) | 0.005 μW |
| 4KB SRAM array | 5 mW (active) | 1 mW |

### Power Exercise (Do on Day 4)
```
Your FIFO: 16 entries × 8 bits = 128 flip-flops
Per DFF: 0.5 μW @ 130nm, 100 MHz
Total: 128 × 0.5 = 64 μW (just storage)

Add control logic (~50 gates): +25 μW
Add clock tree overhead: ×1.3
FIFO Total: (64 + 25) × 1.3 ≈ 116 μW

At 125 MHz: 116 × 1.25 ≈ 145 μW
```

---

## WEEK 1: FOUNDATIONS + PHYSICAL INTUITION (JAN 3-9, 2026)

### DAY 1: FRIDAY, JANUARY 3, 2026
**Total: 16 hours** (spread over 2-3 days)
**Theme: Digital logic fundamentals + transistor-level understanding**

---

#### 📚 THEORY BLOCK (8-11 AM) — 3 hours

**Hour 1 (8-9 AM): MOSFET Physics for Architects**
- Watch: AddOhms "MOSFET as a Switch" (15 min)
- Read: Harris & Harris Chapter 1.4 pages 18-24 (30 min)
- **NEW - Physical Understanding:**
  - Watch: "MOSFET Layout Basics" (YouTube, 15 min)
  - Understand: Why PMOS is wider than NMOS (hole mobility vs electron)
  - Learn: Minimum feature size, gate length, channel width
- **Notebook:** 
  - Draw NMOS/PMOS symbols AND cross-section
  - Note: PMOS typically 2.5× wider for equal drive strength
  - Sketch: Top-view layout of single transistor

**Hour 2 (9-10 AM): CMOS Logic - Circuit to Layout**
- Read: Harris & Harris Chapter 3.2 pages 98-110
- **NEW - Physical Design Connection:**
  - Study: CMOS inverter layout (stick diagram)
  - Understand: N-well, P-well, poly, diffusion, metal layers
  - Learn: Why series transistors (NAND/NOR) create different layouts
- **Deliverable:** 
  - Draw CMOS inverter schematic
  - Draw CMOS inverter stick diagram (layout view)
  - Understand metal layers: M1 (routing), M2 (power), M3+ (global)

**Hour 3 (10-11 AM): D Flip-Flop + Standard Cell Concept**
- Read: Harris & Harris Chapter 3.2 pages 98-110 (DFF section)
- **NEW - Standard Cell Library Introduction:**
  - Learn: What is a standard cell? (fixed height, variable width)
  - Understand: Why DFF is taller than inverter (more transistors)
  - Study: Cell timing (setup, hold, clk-to-Q) comes from layout parasitics
- **Notebook:**
  - DFF timing equations
  - Sketch: How 6 transistors become SRAM cell layout
  - Note: Routing affects timing (longer wire = more delay)

---

#### 🖥️ SOFTWARE BLOCK (11 AM - 2:30 PM) — 3.5 hours

**Hour 1-2 (11 AM - 1 PM): C++ Modeling with Area Awareness**

**File:** `src/cpp/logic_gates.cpp` (~80 lines, enhanced)
- **Purpose:** Golden model + area estimation
- **What it does:**
  ```cpp
  class LogicGate {
  public:
      virtual bool evaluate(bool a, bool b) = 0;
      virtual double get_area_um2() = 0;  // NEW: Estimate silicon area
      virtual int get_transistor_count() = 0;  // NEW: Count devices
  };
  
  class Inverter : public LogicGate {
      // 2 transistors, ~1.5 μm² at 7nm
  };
  
  class NAND2 : public LogicGate {
      // 4 transistors, ~2.5 μm²
  };
  ```
- **NEW Skills:** 
  - Estimate area from transistor count
  - Understand area vs performance tradeoffs
  - Link logic function to physical cost
- **Deliverable:** Code that prints truth tables + area estimates

**Hour 3 (1-2:30 PM): Python with Physical Metrics**

**File:** `scripts/progress_tracker.py` (~30 lines)
- Same as before

**File:** `scripts/area_calculator.py` (~50 lines) **NEW**
- **Purpose:** Track area throughout project
- **What it does:**
  - Input: Module name, transistor count
  - Calculate: Estimated area at 7nm/5nm/3nm nodes
  - Plot: Area vs performance tradeoffs
  - Track: Total chip area as you add modules
- **Deliverable:** `python3 scripts/area_calculator.py inverter 2` outputs area estimate

---

**LUNCH: 2:30 - 3:30 PM**

---

#### 🔧 RTL BLOCK (3:30 - 7:00 PM) — 3.5 hours

**Hour 1 (3:30-4:30 PM): Hand-Draw Circuits + Layout**
- Draw CMOS inverter schematic
- **NEW:** Draw CMOS inverter stick diagram (layout view)
  - Show: N-well region (PMOS), P-substrate (NMOS)
  - Mark: Metal1, Poly, Diffusion layers
  - Indicate: Contact/via positions
- Draw 2-input NAND stick diagram
- Draw 2-input NOR stick diagram
- **Compare:** Why NOR is bigger than NAND (PMOS in series = wider)
- **Deliverable:** 3 schematics + 3 stick diagrams on paper

**Hour 2 (4:30-5:30 PM): Verilog Modules**
- Write `rtl/inverter.v`, `nand2.v`, `nor2.v`, `traffic_light_fsm.v`
- Same as original plan

**Hour 3 (5:30-7:00 PM): Verilator Testbenches + Synthesis Check**
- Write `tests/cpp/tb_inverter.cpp`
- **NEW:** Add synthesis directives
  ```verilog
  // In inverter.v, add:
  (* keep_hierarchy = "yes" *)
  (* max_fanout = 4 *)
  ```
- Run Verilator simulation
- **Deliverable:** Tests pass + understand synthesis attributes

---

#### 🏗️ PHYSICAL DESIGN BLOCK (7:00 - 10:00 PM) — 3 hours **NEW**

**Hour 1 (7:00-8:00 PM): Install CAD Tools + First Layout**
- **Install Klayout** (open-source layout viewer)
  ```bash
  brew install klayout  # macOS
  # or download from klayout.de
  ```
- **Download:** SkyWater 130nm PDK (open-source)
  ```bash
  git clone https://github.com/google/skywater-pdk-libs-sky130_fd_sc_hd
  ```
- **Explore:** Standard cell layouts in PDK
  - Open `sky130_fd_sc_hd__inv_1.gds` in Klayout
  - Identify layers: Poly, N-diffusion, P-diffusion, Metal1
- **Deliverable:** Screenshot of inverter layout, annotated layers

**Hour 2 (8:00-9:00 PM): Manual Layout Exercise**
- **Project:** Draw inverter layout by hand
  - Use graph paper (or iPad with drawing app)
  - Follow design rules from PDK documentation:
    - Min poly width: 0.15 μm
    - Min metal1 width: 0.14 μm
    - Min spacing: 0.17 μm
  - Draw: N-well boundary, transistor gates, source/drain, contacts
- **Calculate:** 
  - Your layout area in μm²
  - Compare to standard cell (should be ~2× larger, you're learning)
- **Deliverable:** Hand-drawn layout with dimensions

**Hour 3 (9:00-10:00 PM): Floorplan Thinking**
- **Study:** How gates connect in traffic light FSM
  - Draw block diagram showing state registers, combinational logic
  - Estimate distances if placed poorly vs well
- **Calculate:**
  - If inverter is 2 μm × 1.5 μm
  - FSM has ~30 gates
  - Poor floorplan: 30 × (2×1.5) = 90 μm²
  - Good floorplan (shared power): ~60 μm² (30% savings)
- **Notebook:** 
  - Sketch good vs bad floorplan for FSM
  - Note: Placement affects power and timing
- **Deliverable:** Floorplan sketch with area calculations

---

#### 📝 END OF DAY SUMMARY (10:00 - 10:30 PM)

**Day 1 Enhanced Checklist:**
```
✅ Theory: MOSFET physics, CMOS layouts, standard cells (3 hrs)
✅ C++ Model: Logic gates + area estimation (2 hrs)
✅ Python: Progress tracker + area calculator (1.5 hrs)
✅ RTL: 4 Verilog modules + testbench (3.5 hrs)
✅ Physical: CAD tools setup, manual layout, floorplan (3 hrs)
✅ Cross-validation: RTL + area awareness

📊 Time: 16 hours (Theory 19% / Software 22% / RTL 22% / Physical 19%)
📊 Skills: Logic → RTL → Layout (FULL STACK)
📊 Score: 10/10
👉 Tomorrow: Testbench infrastructure + SRAM cell design
```

---

### DAY 2: SATURDAY, JANUARY 4, 2026
**Total: 14 hours**
**Theme: Verilog fluency + SRAM physical design**

**🎯 WHY DAY 2 MATTERS:**
Day 1 gave you transistor intuition. Day 2 bridges that to **memory cells** — the foundation of ALL caches and DRAM. You'll understand SRAM at every layer: physics → behavioral model → RTL → layout. This is the "full-stack memory architect" mindset.

---

#### 📚 THEORY BLOCK (8-10:30 AM) — 2.5 hours

**Hour 1 (8-9 AM): Verilog Language + Synthesis**
- **📖 Read:** Harris & Harris Chapter 4.1-4.3 pages 166-185 (Verilog syntax)
- **📖 Read:** Cummings paper "Nonblocking Assignments in Verilog Synthesis" (10 pages, essential!)
  - Link: http://www.sunburst-design.com/papers/CummingsSNUG2000SJ_NBA.pdf
- **🎥 Watch:** Nand2Tetris Verilog intro (20 min) OR ZipCPU "Verilog Basics" video
- **NEW - Synthesis Awareness:**
  - Learn: `wire` vs `reg` affects synthesis differently
  - Understand: Blocking (`=`) vs non-blocking (`<=`) creates different circuits
  - Study: What `always @(*)` synthesizes to (combinational cloud)
- **Notebook:** 
  - wire/reg synthesis differences (draw circuit for each)
  - Blocking = potential combinational loops (bad)
  - Non-blocking = registers (good for sequential)
  - Write the "always block rules" for yourself

**Hour 2 (9-10 AM): Testbench Methodology + GTKWave**
- **📖 Read:** Verilator manual Section 2 "Example C++ Execution" (15 min)
- **🔧 Practice:** GTKWave waveform analysis
  ```bash
  # Run your Day 1 inverter testbench
  cd rtl && make sim_inverter
  gtkwave obj_dir/inverter.vcd &
  ```
- **GTKWave Exercise:**
  1. Find the clock signal, zoom to see edges
  2. Add input `a` and output `y` signals
  3. Measure propagation delay (time from input change to output change)
  4. Add a marker at each clock rising edge
  5. **Screenshot:** Annotated waveform showing timing
- **Notebook:** 
  - GTKWave keyboard shortcuts
  - How to measure setup/hold time visually
  - What a "glitch" looks like in waveforms

**Hour 3 (10-10:30 AM): SRAM Cell Theory**
- **📖 Read:** Weste & Harris Chapter 12.2 pages 460-475 (SRAM arrays)
- **📖 Read:** Jacob/Ng/Wang Chapter 4.1 pages 131-145 (SRAM architecture)
- **🎥 Watch:** "6T SRAM Cell Operation" (YouTube, 10 min)
- **Learn:**
  - 6T SRAM cell: 2 cross-coupled inverters + 2 access transistors
  - Why SRAM is fast (no refresh) but expensive (6T per bit)
  - Bitline vs wordline organization
  - Read operation: Precharge bitlines → assert wordline → sense differential
  - Write operation: Drive bitlines → assert wordline → overpower cell
- **Calculate:**
  - 4 KB cache = 32,768 bits
  - 6 transistors/bit = 196,608 transistors JUST for storage
  - Tag array (20 bits × 64 entries) = 7,680 more transistors
  - Decoder, sense amps, muxes → ~300K transistors total
- **Notebook:** 
  - Draw 6T SRAM cell schematic (label all 6 transistors)
  - Draw read/write timing diagram
  - Note: SRAM cell ratio (pull-down stronger than access)

---

#### 🖥️ SOFTWARE BLOCK (10:30 AM - 1:30 PM) — 3 hours
**Theme: Cross-validation — Your C++ golden model vs RTL reality**

---

**Hour 1-2 (10:30 AM - 12:30 PM): SRAM Array Behavioral Model**

**Why This Matters:**
You're about to design a 6T SRAM cell in layout (afternoon). But **how do you verify the RTL matches your golden model?** You need a **C++ reference implementation** that predicts SRAM behavior (read/write timing, bitline charging, sense amplifier delays).

**File:** `cache sim/sram_array/cpp/sram_behavioral_model.cpp` (~180 lines)

**What It Does:**
- Models **6T SRAM cell physics**: bitline capacitance, wordline delays, access time
- Simulates **read/write operations** with real timing (tRCD, tCAS, tWR)
- Estimates **area** (cell area × array size + decoder + sense amps)
- Estimates **power** (bitline switching energy, leakage per cell)
- **Cross-validates with your Verilog SRAM** (coming in RTL block)

**Key Functions:**
```cpp
class SRAMBehavioralModel {
public:
    // Core operations
    uint8_t read(uint16_t address);   // Returns data + timing
    void write(uint16_t address, uint8_t data);
    
    // Timing model (based on 130nm SkyWater PDK)
    double get_read_access_time_ns();  // tAA ≈ 2.5 ns for 4KB
    double get_write_cycle_time_ns();  // tWC ≈ 3.0 ns
    
    // Physical estimates
    double estimate_cell_area_um2();   // 6T cell ≈ 0.57 μm² @ 130nm
    double estimate_total_area_um2();  // Array + decoder + mux
    double estimate_read_energy_pJ();  // Bitline + sense amp energy
    double estimate_leakage_uW();      // 6 transistors × subthreshold leakage
};
```

**Why You'll Love This:**
- **It's real memory architecture work** — not generic testbench boilerplate
- Directly connects to your **existing cache simulator** (SRAM is cache's storage backend)
- When you write Verilog SRAM later, you can **diff timing against this model**
- Builds intuition for **tRCD, tCAS, tRAS** (same timing parameters you used in DRAM scheduler!)

**Deliverable:** `make test_sram_model` runs timing + area + power report

---

**Hour 3 (12:30-1:30 PM): Cross-Validation Framework** 

**Why This Matters:**
When RTL disagrees with C++ model, **who's right?** You need automated cross-validation that runs **identical test vectors** through both and flags mismatches.

**File:** `scripts/cross_validate.py` (~120 lines)

**What It Does:**
1. Generates **random memory access patterns** (read/write sequences)
2. Runs pattern through **C++ behavioral model** → saves expected results
3. Runs pattern through **Verilator RTL sim** → saves actual results
4. **Diffs the two**: cycle-accurate comparison of data + timing
5. On mismatch: dumps waveforms + analysis

**Key Features:**
```python
class CrossValidator:
    def generate_test_vectors(self, num_ops=1000):
        # Random read/write pattern
        # Includes corner cases: back-to-back writes, read-after-write
        
    def run_cpp_golden_model(self, vectors):
        # Execute vectors in C++ model, capture timing
        
    def run_verilator_rtl(self, vectors):
        # Execute same vectors in Verilator, capture timing
        
    def compare_results(self):
        # Cycle-by-cycle diff
        # If mismatch: "Cycle 47: Expected 0xAB, Got 0xAC"
        
    def analyze_timing_diff(self):
        # Compare: C++ predicted 2.5ns read, RTL took 2.8ns
        # Flag: "RTL 12% slower — check routing delays"
```

**Visual Output:**
```
=== CROSS-VALIDATION REPORT ===
Test vectors: 1000 operations
Data matches: 1000/1000 ✅
Timing correlation: 0.94 (excellent)
Average RTL overhead: +8% (expected for synthesis)

Timing breakdown:
  C++ model:  2.50 ns read access
  RTL actual: 2.71 ns read access
  Difference: +210 ps (routing delays)
  
Area estimates:
  C++ predicted: 18,432 μm² (6T × 32,768 bits)
  Match expected: YES ✅
```

**Why This Is Architect-Level Work:**
- **Jim Keller's "predict within 10%" philosophy** — your C++ model becomes the predictor
- Industry standard: golden models catch RTL bugs **before tape-out**
- Same workflow as Apple/AMD: behavioral model → RTL → cross-validate
- **Connects to your DRAM work**: you already have timing validators in `dram/cpp/timing_validator.cpp`!

**Deliverable:** `python3 scripts/cross_validate.py` auto-runs and reports mismatches

---

**What You've Built (Software Block Summary):**
```
✅ SRAM behavioral model (C++) — predicts timing/area/power
✅ Cross-validation framework (Python) — catches RTL bugs early
✅ Memory architecture intuition — tRCD/tCAS now in SRAM context
✅ Real verification methodology — same as industry (Apple/AMD/NVIDIA)
```

**Why This Is Better:**
- ❌ OLD: Generic testbench template (could be for any RTL)
- ✅ NEW: **Memory-specific** behavioral model (leverages your existing work)
- ❌ OLD: Python script that "collects metrics" (boring)
- ✅ NEW: **Cross-validation tool** that finds bugs (critical skill)
- ❌ OLD: Breaks momentum from cache/DRAM work
- ✅ NEW: **Extends your memory system** with SRAM layer

---

**LUNCH: 1:30 - 2:30 PM**

---

#### 🔧 RTL BLOCK (2:30 - 5:30 PM) — 3 hours

**Hour 1 (2:30-3:30 PM): SRAM Read/Write Controller**

**File:** `rtl/sram_controller.v` (~120 lines)
- **Purpose:** Implement behavioral SRAM array backed by your C++ model
- **Signals:**
  - `addr[15:0]` — address (64K entries)
  - `data_in[31:0]` — write data
  - `data_out[31:0]` — read data
  - `we` — write enable
  - `oe` — output enable
  - `ready` — data valid (next cycle)
- **Timing:** Model 2.5 ns read access (from C++ model)
  ```verilog
  (* dont_touch = "true" *)
  always @(posedge clk or negedge rst_n) begin
      if (!rst_n) ready <= 0;
      else ready <= (we || oe);  // Pipelined: ready next cycle
  end
  ```
- **Deliverable:** Synthesizable SRAM controller that matches behavioral model

**Hour 2 (3:30-4:30 PM): SRAM Testbench + Cross-Validation**

**File:** `tests/cpp/tb_sram_controller.cpp` (~150 lines)
```cpp
// Use SAME test vectors that ran through C++ model
// Compare RTL output cycle-by-cycle against golden model
// Flag any mismatches
```

**Key test cases:**
- Sequential reads
- Sequential writes
- Read-after-write (RAW hazard)
- Back-to-back writes
- Address boundary cases (0x0000, 0xFFFF)

**Deliverable:** `make test_sram` runs RTL + compares vs C++ model, reports pass/fail

**Hour 3 (4:30-5:30 PM): PPA Analysis + Synthesis Constraints**

**What You'll Do:**
1. Run synthesis on `sram_controller.v`
2. Analyze post-synthesis area:
   - Compare to your C++ estimate
   - Should be within ±20%
3. Extract timing report:
   - Critical path delay
   - Compare to behavioral model (2.5 ns target)
4. Generate PPA chart:
   ```
   Module         | Area (μm²) | Delay (ns) | Power (mW)
   ============================================
   SRAM (64K)     | 18,432     | 2.71       | 45.2
   Decoder (8K)   | 2,100      | 0.89       | 5.1
   Sense Amp (8x) | 1,200      | 0.42       | 8.9
   Total          | 21,732     | 2.71       | 59.2
   ```

**Deliverable:** `results/day2_ppa_report.txt` + chart showing model vs RTL correlation

---

#### 🏗️ PHYSICAL DESIGN BLOCK (5:30 - 9:30 PM) — 4 hours **NEW**

**Hour 1-2 (5:30-7:30 PM): SRAM Cell Layout Design**

**Project:** Design 6T SRAM cell in Klayout
- **Study:** Reference layouts from SkyWater PDK
- **Design Your Own:**
  - Place 2 inverters (cross-coupled)
  - Add 2 access transistors
  - Route wordline (horizontal) and bitlines (vertical)
  - Minimize area while meeting DRC rules
- **Goals:**
  - Cell height: Match standard cell height (2.72 μm for sky130)
  - Cell width: Minimize (your design vs reference)
  - Symmetry: Mirror layout for density
- **Deliverable:** SRAM cell layout in GDS format

**Hour 3 (7:30-8:30 PM): SRAM Array Architecture**
- **Design:** 64-bit SRAM array (8×8)
  - Array organization: 8 rows (wordlines) × 8 columns (bitlines)
  - Add: Row decoder (3-to-8)
  - Add: Column mux (3-to-1)
  - Add: Sense amplifiers (8 of them)
- **Calculate:**
  - Array area = 64 cells × (your cell area)
  - Decoder area = ~8 NAND gates
  - Total area = array + decoder + SA + routing (×1.3 overhead)
- **Deliverable:** Block diagram with area breakdown

**Hour 4 (8:30-9:30 PM): Power Grid Design Basics**
- **Learn:** Why power distribution matters
  - IR drop: Voltage sags if wires too thin
  - EM (electromigration): Wires fail if current too high
- **Design Exercise:**
  - Given: Your SRAM array draws 10 mA peak
  - IR drop limit: 50 mV max
  - Calculate: Minimum wire width needed
    - R = ρL/A, V = IR
    - For M1: ρ ≈ 0.1 Ω/sq, L = 100 μm
    - Solve for wire width
- **Notebook:** 
  - Power grid design rules
  - Why wide VDD/GND needed for memory
- **Deliverable:** Power grid width calculation

---

#### 📝 END OF DAY SUMMARY (9:30 - 10:00 PM)

**Day 2 Enhanced Checklist:**
```
✅ Theory: Verilog synthesis, SRAM cells (2.5 hrs)
✅ C++ Model: TestbenchBase with PPA tracking (2 hrs)
✅ Python: Test runner + PPA visualizer (1 hr)
✅ RTL: DFF + 3 testbenches (3 hrs)
✅ Physical: SRAM cell layout + array + power grid (4 hrs)

📊 Time: 14 hours
📊 Skills: RTL → SRAM → Power delivery (ARCHITECT THINKING)
📊 Score: 10/10
👉 Tomorrow: PYNQ board + DDR3 interface + physical timing
```

---

### DAY 3: MONDAY, JANUARY 6, 2026
**Total: 16 hours**
**Theme: Real hardware + DDR3 physical interface**

**🎯 WHY DAY 3 MATTERS:**
This is THE day you become a "real" engineer. Theory becomes silicon. Your LED will blink on actual hardware. This is the moment you prove to yourself (and future interviewers) that you can make things WORK, not just simulate them.

---

#### 📚 THEORY BLOCK (7-10 AM) — 3 hours

**Hour 1 (7-8 AM): Amdahl's Law — The Architect's Reality Check**
- **📖 Read:** Hennessy & Patterson Chapter 1.9 pages 46-52
- **📖 Read:** Original Amdahl paper "Validity of the Single Processor Approach" (1967, 2 pages)
- **The Equation:** $Speedup = \frac{1}{(1-P) + \frac{P}{S}}$
  - P = fraction of program that can be parallelized/optimized
  - S = speedup of that fraction
- **Memory Example:**
  - Memory access is 40% of execution time
  - You make memory 10× faster
  - Speedup = 1 / ((1-0.4) + 0.4/10) = 1.56×
  - Even 10× memory improvement → only 1.56× total speedup!
- **NEW - Apply to floorplan:**
  - If routing is 30% of delay, 2× better routing → only 1.18× faster chip
  - **Insight:** Fix the BIGGEST bottleneck first
- **Notebook:** The equation + 2 worked examples + floorplan application

**Hour 2 (8-9 AM): Little's Law — Queue Sizing from First Principles**
- **📖 Read:** Hennessy & Patterson Chapter 2.4 pages 115-120 (latency/bandwidth)
- **📄 Paper:** Drepper "What Every Programmer Should Know" Section 2.1.1 (memory hierarchy overview)
- **The Equation:** $L = λW$
  - L = average number of items in system
  - λ = arrival rate (items per second)
  - W = average time in system (seconds)
- **Memory System Example (PYNQ-Z2):**
  - DDR3: 400 MHz, 16-bit bus → λ = 800 MB/s
  - Latency W = 50 ns average
  - L = 800 MB/s × 50 ns = 40 bytes in flight
  - **Need queue depth ≥ 40 bytes to saturate bandwidth!**
- **NEW - Queue sizing affects area:**
  - 40-byte queue = 40 × 8 = 320 flip-flops
  - Each DFF ≈ 5 μm² at 130nm → 1600 μm² just for queue
  - **Tradeoff:** More bandwidth requires more silicon
- **Exercise:** Calculate L for your UART (115200 baud, 10ms latency)
- **Notebook:** L = λW with memory system + UART examples

**Hour 3 (9-10 AM): DDR3 Physical Interface Theory**
- **📖 Read:** JEDEC DDR3 Specification JESD79-3F sections 1-3 (free download from jedec.org)
- **📖 Read:** Jacob/Ng/Wang Chapter 6 pages 235-280 (DRAM interfaces)
- **🎥 Watch:** "DDR Memory Explained" by Branch Education (15 min, excellent visuals)
- **Learn:**
  - Why DDR needs termination resistors (signal integrity, reflections)
  - Differential vs single-ended signaling (DQS is differential)
  - PCB trace impedance matching (50Ω controlled impedance)
  - Why DQS (strobe) travels with data (source-synchronous clocking)
  - Read leveling vs write leveling (calibration at power-up)
- **Calculate:**
  - At 1066 MT/s (DDR3-1066), bit period = 1/1066M = 0.94 ns
  - PCB trace delay ≈ 6 ps/mm (FR4 material)
  - If DQ-to-DQS skew budget is 100 ps, max length mismatch = 100/6 ≈ 17 mm
- **Notebook:** 
  - DDR3 signal integrity requirements
  - Draw timing diagram: DQS vs DQ alignment
  - Note: Why memory controllers are "hard" (tight timing margins)

---

#### 🖥️ SOFTWARE BLOCK (10 AM - 1 PM) — 3 hours

**Hour 1-2 (10 AM - 12 PM): C++ DDR3 Bandwidth Calculator**

**File:** `tests/cpp/ddr3_bandwidth_calc.cpp` (~150 lines, enhanced)
```cpp
struct DDR3Params {
    double freq_mhz;      // 533 MHz
    int bus_width_bits;   // 16
    int ddr_multiplier;   // 2
    double efficiency;    // 0.75
    
    // NEW: Physical parameters
    double trace_length_mm;    // PCB trace
    double load_capacitance_pf; // Per pin
};

class DDR3Calculator {
public:
    double theoretical_bandwidth();
    double realistic_bandwidth();
    
    // NEW: Physical timing analysis
    double calculate_trace_delay();
    double calculate_skew_budget();
    bool check_timing_closure(double frequency);
    
    // NEW: Power estimation
    double calculate_io_power();  // I/O power dominates in DDR
};
```
- **Deliverable:** Bandwidth calculator + timing checker + power estimator

**Hour 3 (12 PM - 1 PM): Python Physical Constraint Generator** **NEW**

**File:** `scripts/generate_timing_constraints.py` (~100 lines)
- **Purpose:** Auto-generate timing constraints for synthesis
- **What it does:**
  ```python
  def generate_sdc(module_name, clock_freq_mhz):
      # Generate Synopsys Design Constraints file
      sdc = f"""
      create_clock -period {1000/clock_freq_mhz} [get_ports clk]
      set_input_delay -max {setup_time} [get_ports data_in]
      set_output_delay -max {hold_time} [get_ports data_out]
      """
      # Save to constraints/module_name.sdc
  ```
- **Skills:** Learn constraint syntax for synthesis tools
- **Deliverable:** Auto-generated SDC files for all modules

---

**LUNCH: 1-2 PM**

---

#### 🔧 RTL BLOCK (2-5 PM) — 3 hours

**Hour 1-2 (2-3 PM): LED Blinker Project Setup**
- Install Vivado
- Create PYNQ-Z2 project
- Write `led_blinker.v`
- **NEW:** Add timing constraints
  ```tcl
  create_clock -period 8.000 [get_ports clk_125mhz]
  set_property PACKAGE_PIN R14 [get_ports led[0]]
  set_property IOSTANDARD LVCMOS33 [get_ports led[0]]
  ```

**Hour 2 (3-4 PM): Synthesis + Implementation**
- Run synthesis
- **NEW:** Analyze timing report
  - Check: Worst negative slack (WNS)
  - Check: Total negative slack (TNS)
  - Understand: Critical path
- Place & route
- **NEW:** Analyze utilization
  - LUTs used / total
  - FFs used / total
  - Block RAM used / total

**Hour 3 (4-5 PM): Program FPGA + Verify**
- Program bitstream
- **MAGIC MOMENT:** LED blinks
- **NEW:** Measure actual frequency with oscilloscope/logic analyzer
  - Is it exactly 1 Hz or slightly off?
  - Understand: Clock accuracy matters

---

#### 🏗️ PHYSICAL DESIGN BLOCK (5-9 PM) — 4 hours **NEW**

**Hour 1 (5-6 PM): FPGA Architecture Study**
- **Learn:** How FPGA fabric works
  - LUT = lookup table (implements any 6-input function)
  - Routing = programmable switches between LUTs
  - Why routing delay >> logic delay (70% of total)
- **Study:** Your LED blinker floorplan in Vivado
  - Open Device view
  - Find where your counter is placed
  - Measure routing distance to LED output pin
- **Calculate:**
  - If counter is 100 CLB rows from pin
  - Each routing hop ≈ 50 ps
  - Total routing delay ≈ 5 ns
  - This limits max frequency
- **Deliverable:** Screenshot of placement with annotations

**Hour 2 (6-7 PM): DDR3 PCB Design Basics**
- **Study:** PYNQ-Z2 schematic (DDR3 section)
  - Trace length matching (all DQ signals within 50 mils)
  - Termination resistors (ODT = On-Die Termination)
  - Decoupling capacitors (100 nF every 5 mm)
- **Learn:** Why PCB design matters
  - Poor layout → signal integrity issues
  - SI issues → bit errors at high speed
  - Errors → lower achievable frequency
- **Project:** Sketch ideal DDR3 PCB layout
  - SOC in center
  - DDR chips flanking both sides (minimize distance)
  - Power planes for clean VDD
  - Ground plane for return current
- **Deliverable:** Annotated PCB layout sketch

**Hour 3 (7-8 PM): Clock Distribution Network**
- **Learn:** Clock tree synthesis
  - Why clocks need balanced routing (skew < 100 ps)
  - H-tree structure for equal path lengths
  - Buffer insertion for drive strength
- **Design Exercise:**
  - Given: 1000 flip-flops to clock
  - Clock frequency: 500 MHz (2 ns period)
  - Skew budget: 10% of period = 200 ps
  - Design: H-tree with 4 levels
    - Level 0: 1 driver
    - Level 1: 2 buffers
    - Level 2: 4 buffers
    - Level 3: 8 buffers → 16 endpoints
- **Calculate:**
  - Wire delay per segment
  - Buffer delay per level
  - Total clock insertion delay
- **Deliverable:** Clock tree diagram with timing

**Hour 4 (8-9 PM): Power Delivery Network for DDR**
- **Problem:** DDR draws 500 mA during reads (sudden current)
  - This causes voltage droops (di/dt noise)
  - Droops → timing violations
- **Solution:** Decoupling capacitors
  - Place caps close to DDR chips
  - Multiple cap values (100 nF, 10 μF, 100 μF)
  - Different values cover different frequencies
- **Calculate:**
  - L·di/dt = V_droop
  - If L = 1 nH (inductance from cap to chip)
  - di = 500 mA in dt = 0.5 ns
  - V_droop = 1e-9 × 0.5 / 0.5e-9 = 1V (HUGE! Need caps)
  - With 100 nF cap close by: Effective L drops to 0.1 nH
  - V_droop = 0.1V (acceptable)
- **Notebook:** Power delivery design rules for memory
- **Deliverable:** Decap placement strategy

---

#### 📝 END OF DAY SUMMARY (9-10 PM)

**Day 3 Enhanced Checklist:**
```
✅ Theory: Amdahl's, Little's, DDR3 physical interface (3 hrs)
✅ C++ Model: DDR3 bandwidth + timing + power (2 hrs)
✅ Python: Timing constraint generator (1 hr)
✅ RTL: LED blinker on real FPGA (3 hrs)
✅ Physical: FPGA routing, PCB design, clock tree, power (4 hrs)
✅ Hardware: Real LED blinking on silicon! 🎉

📊 Time: 16 hours
📊 Skills: Theory → RTL → Silicon → PCB (FULL SYSTEM)
📊 Score: 10/10
👉 Tomorrow: UART + FIFO + Physical queue optimization
```

---

### DAY 4: TUESDAY, JANUARY 7, 2026
**Total: 14 hours**
**Theme: UART + FIFO — Communication is the backbone of memory systems**

---

#### 🎯 WHY DAY 4 MATTERS

**The Big Picture:** Memory controllers don't exist in isolation. They communicate with CPUs, DMAs, and other controllers through **queues** (FIFOs) and **serial interfaces** (UART for debug, AXI for data). Today you build the communication infrastructure that connects your cache/DRAM models to the real world.

**Connection to Your Existing Work:**
- Your `dram/cpp/memory_scheduler.cpp` uses request queues internally
- Today you'll build the **RTL version** of those queues
- UART gives you **debug access** to see what's happening inside your FPGA

---

#### 📚 THEORY BLOCK (8-10:30 AM) — 2.5 hours

**Hour 1 (8-9 AM): Queuing Theory for Hardware Architects**
- **📖 Read:** Hennessy & Patterson Appendix C.2 pages C-10 to C-30 (Queuing fundamentals)
- **📖 Read:** Drepper "What Every Programmer Should Know" Section 3.3.4 (Queue effects)
- **Key Equations:**
  - **Arrival rate (λ):** Requests per second entering queue
  - **Service rate (μ):** Requests per second leaving queue  
  - **Utilization (ρ) = λ/μ** — MUST be < 1 or queue overflows!
- **Your DRAM Connection:**
  - Look at your `dram/cpp/memory_scheduler.cpp`
  - The `read_queue` and `write_queue` ARE FIFOs
  - If CPU sends requests faster than DRAM serves them → overflow
- **Calculate:**
  - UART at 115200 baud = 11,520 bytes/sec arrival
  - Your handler processes at 15,000 bytes/sec
  - ρ = 11,520/15,000 = 0.77 (77% utilized — safe!)
- **Notebook:** ρ = λ/μ with UART and DRAM examples

**Hour 2 (9-10 AM): FIFO Depth Sizing — The Architect's Decision**
- **The Formula:** Depth ≥ Burst_Size × (1 - μ/λ_burst)
- **Why This Matters:**
  - Too shallow → overflow → lost data
  - Too deep → wasted area (each entry = flip-flops)
- **Real Example from Your Work:**
  - Your `memory_scheduler.cpp` has `write_queue_max_size_ = 64`
  - WHY 64? Because DDR3 bursts are 64 bytes!
  - If burst arrives faster than drain, need buffer for full burst
- **Area Impact:**
  - 64-entry × 32-bit FIFO = 2048 flip-flops
  - Each DFF ≈ 5 μm² at 130nm
  - Queue area = 10,240 μm² = 0.01 mm²
  - That's 5% of a small cache controller!
- **Notebook:** FIFO sizing formula + area calculation

**Hour 3 (10-10:30 AM): UART Protocol Deep Dive**
- **Why UART for Memory Debug?**
  - Simple (no complex handshaking like AXI)
  - Universal (every terminal can connect)
  - Debug-friendly (print cache stats in real-time)
- **Protocol:**
  - Start bit (1) + Data bits (8) + Stop bit (1) = 10 bits/byte
  - At 115200 baud: 10 bits × 8.68 μs = 86.8 μs per byte
- **Timing at 125 MHz:**
  - 125 MHz / 115200 baud = 1085 clocks per bit
  - Sample at middle: clock 542
- **Notebook:** UART timing diagram, bit-bang state machine

---

#### 🖥️ SOFTWARE BLOCK (10:30 AM - 1:30 PM) — 3 hours
**Theme: Extend your DRAM scheduler's queue logic to standalone FIFO model**

---

**Hour 1-2 (10:30 AM - 12:30 PM): C++ FIFO Behavioral Model**

**Why This Matters:**
Your `memory_scheduler.cpp` already HAS queue logic. Today you extract it into a **reusable, testable FIFO model** that becomes your RTL golden reference.

**File:** `cache sim/fifo/cpp/fifo_model.cpp` (~200 lines)

**What It Does:**
- Cycle-accurate FIFO simulation matching your RTL
- Statistics tracking (occupancy, overflow attempts, throughput)
- Generates test vectors for Verilator cross-validation

**Key Class Structure:**
```cpp
template<typename T, size_t DEPTH>
class FIFOModel {
private:
    std::array<T, DEPTH> buffer;
    size_t write_ptr = 0, read_ptr = 0, count = 0;
    
    // Statistics (like your SchedulerStatistics!)
    uint64_t total_writes = 0, total_reads = 0;
    uint64_t overflow_count = 0, underflow_count = 0;
    size_t max_occupancy = 0;
    
public:
    bool push(const T& data);     // Returns false on overflow
    std::optional<T> pop();       // Returns nullopt on underflow
    
    bool full() const { return count == DEPTH; }
    bool empty() const { return count == 0; }
    double utilization() const { return (double)count / DEPTH; }
    
    // For cross-validation
    void generate_test_vectors(const std::string& filename);
    bool compare_with_rtl(const std::string& rtl_log);
};
```

**Connection to Your Existing Work:**
- Same pattern as `MemoryRequest` queue in `memory_scheduler.cpp`
- Add the statistics you already understand from `SchedulerStatistics`
- This becomes a **library** you reuse in future memory controllers

**Deliverable:** `make test_fifo` runs behavioral model + generates RTL test vectors

---

**Hour 3 (12:30-1:30 PM): UART Protocol Model**

**File:** `cache sim/uart/cpp/uart_model.cpp` (~150 lines)

**What It Does:**
- Bit-level UART simulation (TX and RX)
- Validates baud rate timing
- Generates test waveforms for RTL comparison

**Key Functions:**
```cpp
class UARTModel {
private:
    double baud_rate = 115200;
    double clock_freq = 125e6;
    int clocks_per_bit;
    
public:
    UARTModel(double baud, double clk) {
        clocks_per_bit = (int)(clk / baud);  // 1085 for 125MHz/115200
    }
    
    // Transmit: convert byte to bit sequence with timing
    std::vector<BitEvent> transmit_byte(uint8_t data);
    
    // Receive: sample bit stream, extract byte
    std::optional<uint8_t> receive_bits(const std::vector<bool>& samples);
    
    // Timing validation
    bool validate_bit_timing(const std::vector<BitEvent>& actual);
};
```

**Why This Matters:**
- When your Verilog UART doesn't work, you can compare against this
- Timing bugs are HARD to find in waveforms — this model finds them
- Same cross-validation pattern you learned in Day 2

**Deliverable:** `make test_uart_model` validates timing calculations

---

**LUNCH: 1:30 - 2:30 PM**

---

#### 🔧 RTL BLOCK (2:30 - 6:30 PM) — 4 hours

**Hour 1 (2:30-3:30 PM): Synchronous FIFO**

**File:** `rtl/fifo_sync.v` (~80 lines)

**Why Synchronous (not async)?**
- Same clock domain = simpler (no CDC issues)
- Good enough for on-chip queues
- Async FIFOs are Week 2 advanced topic

**Signals:**
```verilog
module fifo_sync #(
    parameter WIDTH = 8,
    parameter DEPTH = 16,
    parameter ADDR_WIDTH = $clog2(DEPTH)
)(
    input  wire clk,
    input  wire rst_n,
    
    // Write interface
    input  wire [WIDTH-1:0] wr_data,
    input  wire wr_en,
    output wire full,
    
    // Read interface  
    output wire [WIDTH-1:0] rd_data,
    input  wire rd_en,
    output wire empty,
    
    // Status
    output wire [ADDR_WIDTH:0] count
);
```

**Key Implementation Details:**
- **Pointer arithmetic:** `wr_ptr = (wr_ptr + 1) % DEPTH`
- **Full condition:** `count == DEPTH` (not pointer comparison!)
- **Empty condition:** `count == 0`
- **Memory:** Use `reg [WIDTH-1:0] mem [0:DEPTH-1]`

**Deliverable:** Synthesizable FIFO that matches your C++ model

---

**Hour 2 (3:30-4:30 PM): UART Transmitter**

**File:** `rtl/uart_tx.v` (~100 lines)

**State Machine:**
```
IDLE → LOAD → START_BIT → DATA[0..7] → STOP_BIT → IDLE
```

**Key Signals:**
```verilog
module uart_tx #(
    parameter CLKS_PER_BIT = 1085  // 125MHz / 115200 baud
)(
    input  wire clk,
    input  wire rst_n,
    input  wire [7:0] tx_data,
    input  wire tx_valid,
    output reg  tx_ready,
    output reg  tx_out       // Serial output line
);
```

**Timing Logic:**
```verilog
// Count clocks per bit
always @(posedge clk) begin
    if (state != IDLE) begin
        if (bit_counter < CLKS_PER_BIT - 1)
            bit_counter <= bit_counter + 1;
        else begin
            bit_counter <= 0;
            // Move to next bit
        end
    end
end
```

**Deliverable:** UART TX that sends bytes at 115200 baud

---

**Hour 3 (4:30-5:30 PM): UART Receiver**

**File:** `rtl/uart_rx.v` (~120 lines)

**The Tricky Part: Bit Synchronization**
- You don't know when sender started
- Solution: Detect start bit falling edge, then sample at **middle** of each bit
- Middle = CLKS_PER_BIT / 2 = 542 clocks after edge

**State Machine:**
```
IDLE → DETECT_START → WAIT_HALF → SAMPLE_START → 
DATA[0..7] (sample middle) → STOP_BIT → DONE
```

**Key Insight:**
```verilog
// Sample at middle of bit for noise immunity
localparam HALF_BIT = CLKS_PER_BIT / 2;

always @(posedge clk) begin
    case (state)
        DETECT_START: begin
            if (rx_in == 0) begin  // Start bit detected
                state <= WAIT_HALF;
                bit_counter <= 0;
            end
        end
        WAIT_HALF: begin
            if (bit_counter == HALF_BIT) begin
                state <= SAMPLE_START;
                // Now we're at middle of start bit
            end
        end
        // ...
    endcase
end
```

**Deliverable:** UART RX that correctly receives bytes

---

**Hour 4 (5:30-6:30 PM): FIFO + UART Integration Testbench**

**File:** `tests/cpp/tb_uart_fifo.cpp` (~180 lines)

**Test Scenario:**
1. Send 10 bytes via UART RX → stored in FIFO
2. Read bytes from FIFO → transmit via UART TX
3. Verify: output matches input, timing is correct

**Cross-Validation:**
- Run same bytes through C++ UART model
- Compare bit timing: RTL vs model
- Should match within ±1 clock cycle

**Deliverable:** `make test_uart_fifo` passes all tests

---

#### 🏗️ PHYSICAL DESIGN BLOCK (6:30 - 9:30 PM) — 3 hours

**Hour 1 (6:30-7:30 PM): FIFO Layout Analysis**

**Project:** Analyze FIFO physical implementation in SkyWater PDK

**What You'll Learn:**
- FIFO memory (16×8 = 128 bits) implemented as:
  - Flip-flops (area-expensive but fast)
  - OR SRAM cells (area-efficient but needs compiler)
- Your 16-deep FIFO at 130nm:
  - FF implementation: 128 × 6 transistors = 768 transistors ≈ 2,300 μm²
  - SRAM implementation: 128 × 6T = 768 transistors ≈ 450 μm² (5× smaller!)
- **Architect decision:** When to use FF vs SRAM for queues

**Exercise:**
- Open `sky130_fd_sc_hd__dfxtp_1.gds` (D flip-flop) in Klayout
- Measure area per bit
- Calculate: Your FIFO area if built from DFFs
- Compare to SRAM cell area from Day 2

**Deliverable:** Area comparison table (FF vs SRAM for queues)

---

**Hour 2 (7:30-8:30 PM): UART I/O Cell Design**

**Why I/O Cells Matter:**
- UART TX/RX connect to **external world** (off-chip)
- Need **ESD protection** (electrostatic discharge can destroy chip)
- Need **level shifting** (internal 0.8V → external 3.3V)

**Study:** SkyWater I/O library
- Find `sky130_fd_io__top_gpio` (general purpose I/O cell)
- Identify: ESD diodes, level shifters, output drivers
- Note: I/O cells are HUGE (50-100× larger than logic gates)

**Calculate:**
- Your UART needs 2 I/O pads (TX, RX)
- Each pad ≈ 100 μm × 200 μm = 20,000 μm²
- Total I/O area = 40,000 μm² = 0.04 mm²
- That's 4× your entire FIFO! **I/O dominates small chips**

**Deliverable:** Annotated I/O cell diagram

---

**Hour 3 (8:30-9:30 PM): Queue Optimization for Memory Controllers**

**The Real Problem:**
Your `memory_scheduler.cpp` has separate read/write queues. How do they physically arrange?

**Floorplan Options:**
1. **Side-by-side:** Read queue left, write queue right
   - Pros: Independent routing, good for parallel access
   - Cons: Data must cross chip for read-modify-write

2. **Interleaved:** Read/write entries alternate
   - Pros: Shorter wires for dependent operations
   - Cons: More complex control logic

3. **Unified queue with type tag:**
   - Pros: Flexible depth allocation
   - Cons: Can't optimize read/write independently

**Exercise:**
- Sketch floorplan for your `MemoryScheduler` queues
- Estimate wire lengths for each option
- Choose best option and justify

**Deliverable:** Queue floorplan sketch with area/wire analysis

---

#### 📝 END OF DAY SUMMARY (9:30 - 10:00 PM)

**Day 4 Checklist:**
```
✅ Theory: Queuing theory, FIFO sizing, UART protocol (2.5 hrs)
✅ C++ Model: FIFO + UART behavioral models (3 hrs)
✅ RTL: fifo_sync.v + uart_tx.v + uart_rx.v (4 hrs)
✅ Physical: FIFO layout, I/O cells, queue floorplanning (3 hrs)
✅ Cross-validation: C++ models match RTL behavior

📊 Time: 14 hours
📊 Skills: Queues + Serial I/O (memory controller infrastructure)
📊 Score: 10/10
👉 Tomorrow: AXI4 + DMA — the highway for memory data
```

---

### DAY 5: WEDNESDAY, JANUARY 8, 2026
**Total: 14 hours**
**Theme: AXI4 + DMA — High-bandwidth data movement**

---

#### 🎯 WHY DAY 5 MATTERS

**The Big Picture:** UART is for debug (slow). Real memory systems move data via **buses** (AXI4) and **DMA engines**. Your cache connects to CPU via AXI. Your DRAM controller connects to system via AXI. DMA moves bulk data without CPU involvement.

**Connection to Your Existing Work:**
- Your `memory_channel.cpp` handles burst transfers — AXI4 formalizes this
- Your `memory_scheduler.cpp` queues requests — DMA generates those requests
- This day bridges your software models to industry-standard bus protocols

---

#### 📚 THEORY BLOCK (8-10:30 AM) — 2.5 hours

**Hour 1 (8-9 AM): Bus Architecture Evolution**
- **📖 Read:** ARM AMBA Specification IHI0022E Introduction + Chapter 1 (20 pages, free from ARM)
- **📖 Read:** Hennessy & Patterson Appendix C.6 pages C-45 to C-55 (Interconnection networks)
- **Bus Evolution:**
  - **APB** (Advanced Peripheral Bus): Simple, low-bandwidth (UART, GPIO)
  - **AHB** (Advanced High-performance Bus): Single-master, pipelined
  - **AXI4** (Advanced eXtensible Interface): Multi-master, out-of-order, split transactions
- **Why AXI4 for Memory?**
  - Separate read/write channels (like your separate queues!)
  - Burst transfers (like your DDR3 bursts!)
  - Outstanding transactions (multiple requests in flight)
- **Notebook:** Draw AHB vs AXI4 transaction diagrams

**Hour 2 (9-10 AM): AXI4 Protocol Deep Dive**
- **5 Independent Channels:**
  - **AW** (Write Address): Master → Slave, address for write
  - **W** (Write Data): Master → Slave, data payload
  - **B** (Write Response): Slave → Master, write confirmation
  - **AR** (Read Address): Master → Slave, address for read
  - **R** (Read Data): Slave → Master, data + response
- **Handshake Rule:** Transfer when `VALID && READY` both HIGH
- **Why 5 Channels?**
  - Read and write can happen **simultaneously**
  - Address and data can be **pipelined**
  - Like your `memory_scheduler.cpp` handling reads while writes pending!
- **Exercise:** Trace a complete write transaction (AW → W → B)
- **Notebook:** AXI4 channel diagram with signal directions

**Hour 3 (10-10:30 AM): DMA Engine Architecture**
- **What is DMA?** Direct Memory Access — hardware data mover
- **Why DMA for Memory Systems?**
  - CPU says "copy 4KB from A to B"
  - DMA does ALL transfers, CPU continues other work
  - Critical for: Cache fills, DRAM refresh, I/O buffers
- **DMA State Machine:**
  ```
  IDLE → FETCH_DESCRIPTOR → READ_SRC → WRITE_DST → UPDATE → CHECK_DONE → IDLE
  ```
- **Your DRAM Connection:**
  - DMA generates memory requests (like test traces in your simulator)
  - Memory scheduler handles them (your `MemoryScheduler` class!)
  - This completes the loop: CPU → DMA → Scheduler → DRAM
- **Notebook:** DMA state diagram + latency calculation

---

#### 🖥️ SOFTWARE BLOCK (10:30 AM - 1:30 PM) — 3 hours
**Theme: Bridge your memory models to AXI4 interface**

---

**Hour 1-2 (10:30 AM - 12:30 PM): AXI4 Transaction Model**

**Why This Matters:**
Your `memory_channel.cpp` already handles bursts and timing. Now you need an **AXI4-compatible wrapper** that speaks the standard bus language.

**File:** `cache sim/axi4/cpp/axi4_transaction.cpp` (~250 lines)

**What It Does:**
- Models AXI4 transaction types (FIXED, INCR, WRAP bursts)
- Validates protocol rules (burst length, alignment)
- Generates AXI4 signals for RTL testbenches

**Key Classes:**
```cpp
enum class AxiBurstType { FIXED, INCR, WRAP };

struct AxiWriteTransaction {
    uint32_t address;
    std::vector<uint32_t> data;  // Burst data
    AxiBurstType burst;
    uint8_t size;    // 2^size bytes per beat
    uint8_t len;     // Burst length - 1
    
    // Validation (from AXI4 spec)
    bool is_aligned() const;
    bool is_valid_burst() const;
    uint32_t total_bytes() const;
};

class Axi4Master {
public:
    // Generate AXI4 signals for write
    std::vector<AxiSignals> generate_write(const AxiWriteTransaction& txn);
    
    // Parse AXI4 signals into transaction
    AxiReadTransaction parse_read_response(const std::vector<AxiSignals>& signals);
    
    // Connect to your existing memory models
    void connect_to_memory_channel(MemoryChannel* channel);
};
```

**Connection to Your Existing Work:**
- `AxiWriteTransaction` maps to your `MemoryRequest` struct
- Burst size/length matches your DDR3 burst configuration
- This becomes the **interface layer** between CPU and your memory system

**Deliverable:** AXI4 model that validates transactions

---

**Hour 3 (12:30-1:30 PM): DMA Engine Behavioral Model**

**File:** `cache sim/dma/cpp/dma_engine.cpp` (~200 lines)

**What It Does:**
- Simulates DMA transfers cycle-by-cycle
- Predicts latency for bulk transfers
- Generates memory access patterns (like your trace files!)

**Key Functions:**
```cpp
class DMAEngine {
private:
    enum class State { IDLE, READ, WRITE, DONE };
    State state = State::IDLE;
    
    uint32_t src_addr, dst_addr;
    size_t remaining_bytes;
    uint64_t cycle_count = 0;
    
public:
    // Start a transfer
    void start(uint32_t src, uint32_t dst, size_t length);
    
    // Step one cycle (returns true when complete)
    bool step();
    
    // Performance metrics
    uint64_t get_cycles() const { return cycle_count; }
    double get_bandwidth_MBps(double clock_mhz) const;
    double get_efficiency() const;  // vs theoretical max
};
```

**Why This Matters:**
- DMA is how data gets into/out of your cache hierarchy
- Your cache simulator assumes data magically appears — DMA is that magic
- Performance prediction: "4KB DMA takes 2.5 μs at 80% efficiency"

**Deliverable:** DMA model that predicts transfer latency

---

**LUNCH: 1:30 - 2:30 PM**

---

#### 🔧 RTL BLOCK (2:30 - 6:30 PM) — 4 hours

**Hour 1 (2:30-3:30 PM): AXI4-Lite Slave**

**File:** `rtl/axi4_lite_slave.v` (~150 lines)

**Why AXI4-Lite first?**
- Full AXI4 is complex (bursts, outstanding transactions)
- AXI4-Lite: Single transfer, no bursts, simpler state machine
- Good for control registers (start DMA, check status)

**Signals (simplified):**
```verilog
module axi4_lite_slave #(
    parameter ADDR_WIDTH = 12,
    parameter DATA_WIDTH = 32
)(
    input  wire aclk,
    input  wire aresetn,
    
    // Write address channel
    input  wire [ADDR_WIDTH-1:0] awaddr,
    input  wire awvalid,
    output reg  awready,
    
    // Write data channel
    input  wire [DATA_WIDTH-1:0] wdata,
    input  wire wvalid,
    output reg  wready,
    
    // Write response channel
    output reg [1:0] bresp,
    output reg bvalid,
    input  wire bready,
    
    // Read channels (similar)
    // ...
    
    // Memory interface
    output reg [ADDR_WIDTH-1:0] mem_addr,
    output reg [DATA_WIDTH-1:0] mem_wdata,
    output reg mem_wen,
    input  wire [DATA_WIDTH-1:0] mem_rdata
);
```

**Key FSM:**
```
IDLE → (awvalid) → WRITE_ADDR → (wvalid) → WRITE_DATA → WRITE_RESP → IDLE
     → (arvalid) → READ_ADDR → READ_DATA → IDLE
```

**Deliverable:** AXI4-Lite slave for register access

---

**Hour 2 (3:30-4:30 PM): Simple DMA Engine RTL**

**File:** `rtl/dma_simple.v` (~180 lines)

**Registers (accessible via AXI4-Lite):**
```verilog
// Register map
localparam REG_SRC_ADDR  = 12'h000;  // Source address
localparam REG_DST_ADDR  = 12'h004;  // Destination address
localparam REG_LENGTH    = 12'h008;  // Transfer length
localparam REG_CONTROL   = 12'h00C;  // [0]=start, [1]=done, [2]=error
localparam REG_STATUS    = 12'h010;  // Bytes remaining
```

**State Machine:**
```verilog
localparam S_IDLE      = 3'd0;
localparam S_READ_REQ  = 3'd1;
localparam S_READ_DATA = 3'd2;
localparam S_WRITE_REQ = 3'd3;
localparam S_WRITE_DATA= 3'd4;
localparam S_UPDATE    = 3'd5;
localparam S_DONE      = 3'd6;

always @(posedge clk) begin
    case (state)
        S_IDLE: if (start) state <= S_READ_REQ;
        S_READ_REQ: begin
            // Issue read to memory
            mem_addr <= src_addr;
            mem_ren <= 1;
            state <= S_READ_DATA;
        end
        S_READ_DATA: begin
            if (mem_rvalid) begin
                data_buffer <= mem_rdata;
                state <= S_WRITE_REQ;
            end
        end
        // ... continue
    endcase
end
```

**Deliverable:** Working DMA that transfers one word at a time

---

**Hour 3 (4:30-5:30 PM): Memory Interface for DMA**

**File:** `rtl/memory_interface.v` (~100 lines)

**Purpose:** Connect DMA to simulated memory (for testbench)

**What It Does:**
- Simple memory model (array of registers)
- Single-cycle read/write
- Will be replaced by your cache/DRAM in integration

```verilog
module memory_interface #(
    parameter DEPTH = 1024,
    parameter WIDTH = 32
)(
    input  wire clk,
    input  wire [9:0] addr,
    input  wire [WIDTH-1:0] wdata,
    input  wire wen,
    input  wire ren,
    output reg  [WIDTH-1:0] rdata,
    output reg  rvalid
);

reg [WIDTH-1:0] mem [0:DEPTH-1];

always @(posedge clk) begin
    rvalid <= ren;
    if (ren)
        rdata <= mem[addr];
    if (wen)
        mem[addr] <= wdata;
end
endmodule
```

**Deliverable:** Memory model for DMA testing

---

**Hour 4 (5:30-6:30 PM): DMA Testbench with Cross-Validation**

**File:** `tests/cpp/tb_dma.cpp` (~200 lines)

**Test Scenario:**
1. Initialize memory with known pattern (0xDEADBEEF, 0xCAFEBABE, ...)
2. Configure DMA: src=0x000, dst=0x100, length=64 bytes
3. Start DMA, wait for done
4. Verify: dst memory matches src memory
5. Measure: cycle count vs C++ model prediction

**Cross-Validation:**
```cpp
// C++ model prediction
DMAEngine model;
model.start(0x000, 0x100, 64);
while (!model.step()) {}
uint64_t predicted_cycles = model.get_cycles();

// RTL actual
run_verilator_sim();
uint64_t actual_cycles = get_rtl_cycle_count();

// Compare
double error = abs(predicted - actual) / (double)predicted * 100;
assert(error < 10.0);  // Jim Keller's 10% rule!
```

**Deliverable:** DMA passes cross-validation within 10%

---

#### 🏗️ PHYSICAL DESIGN BLOCK (6:30 - 9:30 PM) — 3 hours

**Hour 1 (6:30-7:30 PM): Bus Routing Analysis**

**The Problem:** AXI4 has 5 channels × many signals = lots of wires!

**Calculate:**
- AXI4-Lite 32-bit: 
  - AW: awaddr(32) + awvalid(1) + awready(1) = 34 wires
  - W: wdata(32) + wstrb(4) + wvalid(1) + wready(1) = 38 wires
  - B: bresp(2) + bvalid(1) + bready(1) = 4 wires
  - AR: araddr(32) + arvalid(1) + arready(1) = 34 wires
  - R: rdata(32) + rresp(2) + rvalid(1) + rready(1) = 36 wires
  - **Total: 146 wires!**

**Routing Challenge:**
- Each wire needs physical space
- Metal pitch at 130nm ≈ 0.34 μm
- 146 wires × 0.34 μm = 50 μm minimum bus width
- But need spacing, shielding → 100-150 μm realistic

**Exercise:**
- Sketch AXI4 bus routing between DMA and memory
- Estimate wire length and routing area
- Identify potential signal integrity issues (long parallel wires = crosstalk)

**Deliverable:** Bus routing diagram with dimensions

---

**Hour 2 (7:30-8:30 PM): DMA Floorplan**

**Block Diagram:**
```
+----------------+     +----------------+     +----------------+
|    CPU Core    |---->|   DMA Engine   |---->|  Memory Ctrl   |
|   (requests)   | AXI |   (transfers)  | AXI |   (your cache) |
+----------------+     +----------------+     +----------------+
                            |
                       +----v----+
                       | Regs    |
                       | (control)|
                       +---------+
```

**Area Estimates:**
- DMA FSM: ~500 gates = ~3,000 μm²
- DMA Registers: 5 × 32-bit = 160 FFs = ~4,000 μm²
- AXI4 interfaces: ~2,000 μm² each = ~4,000 μm²
- **Total DMA: ~11,000 μm² = 0.011 mm²**

**Placement Considerations:**
- Place DMA near memory controller (short data path)
- Place registers near edge (CPU access)
- Route AXI buses in dedicated channels

**Deliverable:** DMA floorplan with area breakdown

---

**Hour 3 (8:30-9:30 PM): Timing Closure on AXI Paths**

**The Challenge:** AXI handshake must complete in one cycle

**Timing Path Analysis:**
- `awvalid` asserts at master
- Signal travels across chip (wire delay)
- Reaches slave, combinational logic generates `awready`
- `awready` travels back to master
- Must all happen before next clock edge!

**Calculate:**
- Assume 10mm chip, AXI path = 5mm
- Wire delay ≈ 50 ps/mm × 5mm = 250 ps
- Round trip = 500 ps
- At 500 MHz (2ns period): Only 1.5ns for logic!
- At 1 GHz (1ns period): Only 500 ps for logic — TIGHT!

**Solution: Pipeline Stages**
- Insert register on `awready` path
- Adds 1 cycle latency but meets timing
- Trade-off: Latency vs frequency

**Exercise:**
- Calculate max frequency for unpipelined AXI path
- Calculate frequency with 1 pipeline stage
- Decide where to insert pipeline registers

**Deliverable:** Timing analysis with pipelining recommendation

---

#### 📝 END OF DAY SUMMARY (9:30 - 10:00 PM)

**Day 5 Checklist:**
```
✅ Theory: AXI4 protocol, DMA architecture (2.5 hrs)
✅ C++ Model: AXI4 transactions + DMA engine (3 hrs)
✅ RTL: axi4_lite_slave.v + dma_simple.v + memory_interface.v (4 hrs)
✅ Physical: Bus routing, DMA floorplan, timing closure (3 hrs)
✅ Cross-validation: DMA matches model within 10%

📊 Time: 14 hours
📊 Skills: Industry-standard bus + data movement engine
📊 Score: 10/10
👉 Tomorrow: L1 Cache RTL — connecting everything together
```

---

### DAY 6: THURSDAY, JANUARY 9, 2026
**Total: 14 hours**
**Theme: L1 Cache RTL — Your software model becomes hardware**

---

#### 🎯 WHY DAY 6 MATTERS

**The Big Picture:** You've built cache simulators in C++ (`cache sim/4-way cache/`). Today you translate that understanding into **synthesizable RTL**. This is the moment where your software knowledge becomes silicon.

**Connection to Your Existing Work:**
- Your `set_associative_cache.cpp` has the algorithms
- Today you implement `cache_direct.v` and `cache_4way.v`
- Cross-validate RTL against your trusted C++ model

---

#### 📚 THEORY BLOCK (8-10:30 AM) — 2.5 hours

**Hour 1 (8-9 AM): Cache Timing — From C++ to Cycles**
- **📖 Read:** Hennessy & Patterson Chapter 5.2-5.3 pages 200-230 (Cache implementation)
- **📖 Read:** Weste & Harris Chapter 12.3 pages 480-495 (SRAM timing)
- **📖 Read:** Your own `set_associative_cache.cpp` — review as reference implementation
- **Your C++ Model's Abstractions:**
  - Hit: instant return
  - Miss: instant fill + return
  - **Reality:** Everything takes cycles!
- **Real Timing:**
  - Tag lookup: 1 cycle (SRAM read + compare)
  - Data read on hit: 1 cycle (SRAM read)
  - Miss handling: 10+ cycles (memory access)
  - Write-back on eviction: 10+ cycles
- **Cache FSM States:**
  ```
  IDLE → TAG_LOOKUP → (hit) → DATA_READ → DONE
                    → (miss) → ALLOCATE → FILL → DATA_READ → DONE
                    → (miss+dirty) → WRITEBACK → ALLOCATE → FILL → ...
  ```
- **Notebook:** Cache FSM with cycle counts per state

**Hour 2 (9-10 AM): Cache Microarchitecture**
- **Read:** Your own `set_associative_cache.cpp` (review)
- **Direct-Mapped Implementation:**
  - Single tag RAM + single data RAM
  - Address decode: `[tag | index | offset]`
  - Hit: `tag_ram[index] == addr_tag && valid[index]`
- **4-Way Set-Associative:**
  - 4 tag RAMs + 4 data RAMs (parallel access)
  - OR: 1 wide tag RAM (4× width)
  - Comparators: 4 parallel tag compares
  - Mux: Select winning way's data
- **LRU Implementation:**
  - Your C++ uses `std::list` for LRU ordering
  - RTL: Use 2-bit counter per set (pseudo-LRU)
  - True LRU for 4-way needs 5 bits of state per set
- **Notebook:** Block diagram of 4-way cache datapath

**Hour 3 (10-10:30 AM): Write Policy Trade-offs**
- **Write-Through:**
  - Every write goes to memory immediately
  - Simple, cache always clean
  - BUT: High memory bandwidth (every write = memory access)
- **Write-Back:**
  - Writes stay in cache until eviction
  - Dirty bit tracks modified lines
  - Complex, but much lower memory bandwidth
- **Your C++ uses Write-Back** (look at `dirty` flag in your cache line)
- **For today's RTL:** Start with Write-Through (simpler)
- **Notebook:** Write-back FSM states

---

#### 🖥️ SOFTWARE BLOCK (10:30 AM - 1:30 PM) — 3 hours
**Theme: Prepare your cache model for RTL cross-validation**

---

**Hour 1-2 (10:30 AM - 12:30 PM): Cache Model RTL Export**

**Why This Matters:**
Your `set_associative_cache.cpp` is the golden model. Now you add **RTL signal generation** so you can compare RTL waveforms against expected behavior.

**File:** `cache sim/4-way cache/cpp/cache_rtl_export.cpp` (~200 lines)

**What It Does:**
- Wraps your existing cache model
- Generates cycle-by-cycle expected signals
- Outputs test vectors for Verilator

**Key Functions:**
```cpp
class CacheRTLExport {
private:
    SetAssociativeCache& cache;
    uint64_t cycle = 0;
    
public:
    struct CycleSignals {
        // Inputs
        bool req_valid;
        uint32_t req_addr;
        bool req_write;
        uint32_t req_wdata;
        
        // Expected outputs
        bool resp_valid;
        bool resp_hit;
        uint32_t resp_rdata;
        
        // Internal state (for debug)
        uint32_t expected_tag;
        uint32_t expected_index;
        bool expected_dirty;
    };
    
    // Simulate one request, generate expected signals
    std::vector<CycleSignals> simulate_request(
        uint32_t addr, 
        bool is_write, 
        uint32_t wdata = 0
    );
    
    // Export to Verilator-compatible format
    void export_test_vectors(const std::string& filename);
    
    // Compare RTL log against expected
    bool validate_rtl_output(const std::string& rtl_log);
};
```

**Connection to Your Existing Work:**
- Uses your existing `SetAssociativeCache` class
- Adds timing awareness (cycles) to your model
- Same cross-validation pattern from Day 2

**Deliverable:** Cache model that generates RTL test vectors

---

**Hour 3 (12:30-1:30 PM): Cache Performance Predictor**

**File:** `scripts/cache_performance.py` (~150 lines)

**What It Does:**
- Predicts cache performance for different configurations
- Plots hit rate vs associativity
- Estimates area for each configuration
- Helps you decide RTL parameters

**Key Analysis:**
```python
class CachePerformancePredictor:
    def __init__(self, trace_file: str):
        self.trace = load_trace(trace_file)
    
    def simulate_config(self, size_kb: int, ways: int, line_size: int) -> dict:
        """Simulate cache with given config, return metrics."""
        # Use your C++ model via subprocess
        return {
            'hit_rate': 0.94,
            'misses': 1234,
            'area_um2': self.estimate_area(size_kb, ways, line_size),
            'access_latency_cycles': 2 if ways <= 4 else 3
        }
    
    def sweep_configurations(self):
        """Test many configurations, find Pareto frontier."""
        results = []
        for size in [4, 8, 16, 32]:  # KB
            for ways in [1, 2, 4, 8]:
                for line in [32, 64]:
                    metrics = self.simulate_config(size, ways, line)
                    results.append(metrics)
        return self.find_pareto_frontier(results)
    
    def estimate_area(self, size_kb, ways, line_size):
        """Area = data SRAM + tag SRAM + comparators + mux."""
        data_bits = size_kb * 1024 * 8
        tag_bits = (32 - log2(size_kb*1024/ways/line_size) - log2(line_size)) * (size_kb*1024/line_size)
        sram_area = (data_bits + tag_bits) * 0.57  # μm² per bit at 130nm
        comparator_area = ways * 100  # μm² per comparator
        return sram_area + comparator_area
```

**Deliverable:** Performance/area analysis for cache configurations

---

**LUNCH: 1:30 - 2:30 PM**

---

#### 🔧 RTL BLOCK (2:30 - 6:30 PM) — 4 hours

**Hour 1 (2:30-3:30 PM): Direct-Mapped Cache**

**File:** `rtl/cache_direct.v` (~200 lines)

**This is your `direct-way/` folder in Verilog!**

**Parameters:**
```verilog
module cache_direct #(
    parameter CACHE_SIZE = 4096,    // 4 KB
    parameter LINE_SIZE = 64,       // 64 bytes per line
    parameter ADDR_WIDTH = 32
)(
    input  wire clk,
    input  wire rst_n,
    
    // CPU interface
    input  wire cpu_req,
    input  wire cpu_write,
    input  wire [ADDR_WIDTH-1:0] cpu_addr,
    input  wire [31:0] cpu_wdata,
    output reg  [31:0] cpu_rdata,
    output reg  cpu_ready,
    output reg  cpu_hit,
    
    // Memory interface (simplified)
    output reg  mem_req,
    output reg  mem_write,
    output reg  [ADDR_WIDTH-1:0] mem_addr,
    output reg  [LINE_SIZE*8-1:0] mem_wdata,
    input  wire [LINE_SIZE*8-1:0] mem_rdata,
    input  wire mem_ready
);

// Address breakdown (same as your C++ code!)
localparam OFFSET_BITS = $clog2(LINE_SIZE);      // 6 bits for 64-byte line
localparam NUM_LINES = CACHE_SIZE / LINE_SIZE;   // 64 lines
localparam INDEX_BITS = $clog2(NUM_LINES);       // 6 bits
localparam TAG_BITS = ADDR_WIDTH - INDEX_BITS - OFFSET_BITS;  // 20 bits

wire [OFFSET_BITS-1:0] offset = cpu_addr[OFFSET_BITS-1:0];
wire [INDEX_BITS-1:0]  index  = cpu_addr[OFFSET_BITS +: INDEX_BITS];
wire [TAG_BITS-1:0]    tag    = cpu_addr[ADDR_WIDTH-1 -: TAG_BITS];
```

**State Machine:**
```verilog
localparam S_IDLE     = 3'd0;
localparam S_TAG_CHECK= 3'd1;
localparam S_HIT      = 3'd2;
localparam S_MISS     = 3'd3;
localparam S_FILL     = 3'd4;
localparam S_WRITEBACK= 3'd5;

always @(posedge clk or negedge rst_n) begin
    if (!rst_n) state <= S_IDLE;
    else case (state)
        S_IDLE: if (cpu_req) state <= S_TAG_CHECK;
        S_TAG_CHECK: begin
            if (valid[index] && tags[index] == tag)
                state <= S_HIT;
            else if (valid[index] && dirty[index])
                state <= S_WRITEBACK;
            else
                state <= S_MISS;
        end
        // ...
    endcase
end
```

**Deliverable:** Direct-mapped cache matching your C++ behavior

---

**Hour 2-3 (3:30-5:30 PM): 4-Way Set-Associative Cache**

**File:** `rtl/cache_4way.v` (~350 lines)

**This is your `4-way cache/` folder in Verilog!**

**Key Differences from Direct-Mapped:**
```verilog
// 4 parallel tag arrays
reg [TAG_BITS-1:0] tags_way0 [0:NUM_SETS-1];
reg [TAG_BITS-1:0] tags_way1 [0:NUM_SETS-1];
reg [TAG_BITS-1:0] tags_way2 [0:NUM_SETS-1];
reg [TAG_BITS-1:0] tags_way3 [0:NUM_SETS-1];

// 4 parallel valid bits
reg valid_way0 [0:NUM_SETS-1];
// ...

// Parallel tag compare (all in same cycle!)
wire hit_way0 = valid_way0[set_index] && (tags_way0[set_index] == tag);
wire hit_way1 = valid_way1[set_index] && (tags_way1[set_index] == tag);
wire hit_way2 = valid_way2[set_index] && (tags_way2[set_index] == tag);
wire hit_way3 = valid_way3[set_index] && (tags_way3[set_index] == tag);

wire cache_hit = hit_way0 | hit_way1 | hit_way2 | hit_way3;

// Determine which way hit
wire [1:0] hit_way = hit_way0 ? 2'd0 :
                     hit_way1 ? 2'd1 :
                     hit_way2 ? 2'd2 : 2'd3;
```

**LRU Implementation (Pseudo-LRU):**
```verilog
// 3-bit tree for pseudo-LRU (approximates true LRU)
// Bit 0: Left half (ways 0,1) vs right half (ways 2,3) more recent
// Bit 1: Way 0 vs way 1 more recent  
// Bit 2: Way 2 vs way 3 more recent
reg [2:0] lru_state [0:NUM_SETS-1];

// Update on access
always @(posedge clk) begin
    if (cache_hit) begin
        case (hit_way)
            2'd0: lru_state[set_index] <= {lru_state[set_index][2], 1'b1, 1'b0};
            2'd1: lru_state[set_index] <= {lru_state[set_index][2], 1'b0, 1'b0};
            2'd2: lru_state[set_index] <= {1'b1, lru_state[set_index][1], 1'b1};
            2'd3: lru_state[set_index] <= {1'b0, lru_state[set_index][1], 1'b1};
        endcase
    end
end

// Select victim way for replacement
wire [1:0] victim_way = lru_state[set_index][0] ? 
                        (lru_state[set_index][2] ? 2'd3 : 2'd2) :
                        (lru_state[set_index][1] ? 2'd1 : 2'd0);
```

**Deliverable:** 4-way cache with pseudo-LRU replacement

---

**Hour 4 (5:30-6:30 PM): Cache Testbench + Cross-Validation**

**File:** `tests/cpp/tb_cache.cpp` (~250 lines)

**Test Vectors (from your C++ model):**
```cpp
// Load test vectors generated by cache_rtl_export.cpp
std::vector<CycleSignals> vectors = load_test_vectors("cache_test_vectors.txt");

// Run Verilator simulation
VerilatedCache* cache = new VerilatedCache();
for (const auto& vec : vectors) {
    cache->cpu_req = vec.req_valid;
    cache->cpu_addr = vec.req_addr;
    cache->cpu_write = vec.req_write;
    cache->eval();
    cache->clk = 1; cache->eval();
    cache->clk = 0; cache->eval();
    
    // Wait for ready
    while (!cache->cpu_ready) {
        cache->clk = 1; cache->eval();
        cache->clk = 0; cache->eval();
    }
    
    // Compare against expected
    assert(cache->cpu_hit == vec.expected_hit);
    assert(cache->cpu_rdata == vec.expected_rdata);
}
```

**Cross-Validation Report:**
```
=== CACHE CROSS-VALIDATION ===
Test vectors: 10,000 accesses
Hit matches: 10,000/10,000 ✅
Data matches: 10,000/10,000 ✅

Performance:
  C++ model hit rate: 94.2%
  RTL actual hit rate: 94.2% ✅
  
  C++ avg latency: 2.3 cycles
  RTL avg latency: 2.4 cycles (+4.3% overhead)
```

**Deliverable:** Cache RTL validated against your C++ golden model

---

#### 🏗️ PHYSICAL DESIGN BLOCK (6:30 - 9:30 PM) — 3 hours

**Hour 1 (6:30-7:30 PM): Cache SRAM Macro Analysis**

**Reality Check:** In real chips, cache SRAMs are **not** synthesized from Verilog `reg` arrays!

**Why Not?**
- Synthesis creates flip-flops from `reg`
- FF area >> SRAM cell area (10×!)
- Your 4KB cache as FFs: 32,768 × 6 transistors = 196K transistors
- Your 4KB cache as SRAM: 32,768 × 6 transistors = 196K (same transistor count, but SRAM is 10× denser layout!)

**Industry Practice:**
- Use **SRAM compiler** (tool generates optimized SRAM layout)
- Verilog instantiates SRAM as a **hard macro** (black box)
- SkyWater PDK includes: `sky130_sram_1kbyte_1r1w_8x1024_8`

**Exercise:**
- Find SRAM macros in SkyWater PDK
- Open in Klayout, measure dimensions
- Calculate: How many 1KB SRAMs for your 4KB cache?
- Estimate: Total area using macros vs FF synthesis

**Deliverable:** SRAM macro analysis with area comparison

---

**Hour 2 (7:30-8:30 PM): Cache Floorplan**

**Block Diagram:**
```
+------------------+------------------+
|   Tag SRAM       |   Tag SRAM       |
|   (Way 0,1)      |   (Way 2,3)      |
+------------------+------------------+
|   Comparators    |   Comparators    |
|   + Hit Logic    |   + Hit Logic    |
+------------------+------------------+
|                                     |
|           Data SRAM                 |
|         (All 4 ways)                |
|                                     |
+------------------+------------------+
|   Output Mux     |   LRU Logic      |
+------------------+------------------+
|         State Machine               |
+-------------------------------------+
```

**Area Breakdown:**
```
Component          | Area (μm²) | % Total
==========================================
Data SRAM (4KB)    | 23,000     | 58%
Tag SRAM (256B)    | 2,000      | 5%
Comparators (4×)   | 400        | 1%
Output Mux         | 200        | 0.5%
LRU Logic          | 500        | 1.3%
FSM + Control      | 1,000      | 2.5%
Routing overhead   | 12,900     | 32%
==========================================
Total              | 40,000     | 100%
```

**Key Insight:** Data SRAM dominates, routing is significant

**Deliverable:** Cache floorplan with area estimates

---

**Hour 3 (8:30-9:30 PM): Cache Timing Paths**

**Critical Paths in Your Cache:**

1. **Tag Compare Path:**
   - SRAM read → Comparator → Hit logic → Output mux control
   - Must complete in 1 cycle for hit

2. **Data Read Path:**
   - SRAM address setup → SRAM read → Output mux → CPU data
   - Often the slowest path

3. **LRU Update Path:**
   - Not critical (can be pipelined)

**Timing Analysis:**
```
Tag SRAM read:     0.8 ns
Comparator:        0.2 ns
Hit mux:           0.1 ns
Data SRAM read:    1.0 ns (if in parallel)
Output mux:        0.2 ns
Setup margin:      0.2 ns
========================
Total:             2.5 ns → 400 MHz max
```

**Optimization Opportunities:**
- Read tag and data in parallel (speculate on hit)
- Pipeline tag check and data read (2-cycle hit latency)
- Use faster SRAM cells (trade area for speed)

**Deliverable:** Timing analysis with max frequency estimate

---

#### 📝 END OF DAY SUMMARY (9:30 - 10:00 PM)

**Day 6 Checklist:**
```
✅ Theory: Cache timing, microarchitecture, write policies (2.5 hrs)
✅ C++ Model: RTL export + performance predictor (3 hrs)
✅ RTL: cache_direct.v + cache_4way.v (4 hrs)
✅ Physical: SRAM macros, floorplan, timing paths (3 hrs)
✅ Cross-validation: RTL matches C++ golden model

📊 Time: 14 hours
📊 Skills: Your C++ cache is now silicon-ready RTL!
📊 Score: 10/10
👉 Tomorrow: Integration — putting it all together
```

---

### DAY 7: FRIDAY, JANUARY 10, 2026
**Total: 14 hours**
**Theme: Integration — Full memory subsystem on FPGA**

---

#### 🎯 WHY DAY 7 MATTERS

**The Big Picture:** You've built all the pieces (SRAM, FIFO, UART, DMA, Cache). Today you connect them into a **working memory subsystem** and run on **real FPGA hardware**. This is the capstone of Week 1.

**What You're Building:**
```
+----------+     +--------+     +---------+     +--------+
|   UART   |---->|  FIFO  |---->|   DMA   |---->|  Cache |----> Memory
| (debug)  |<----|  (buf) |<----|         |<----|        |
+----------+     +--------+     +---------+     +--------+
     |                               |
     v                               v
  PC/Terminal                   Status/Control
```

---

#### 📚 THEORY BLOCK (8-10:30 AM) — 2.5 hours

**Hour 1 (8-9 AM): System Integration Challenges**
- **📖 Read:** ARM "CoreLink System Design Guide" Chapter 2 (SoC integration, free from ARM)
- **📖 Read:** Hennessy & Patterson Chapter 5.8 pages 270-285 (Memory system design)
- **🎥 Watch:** "SoC Integration Best Practices" (YouTube, 20 min if available)
- **Integration Problems:**
  - **Clock domains:** Different modules at different frequencies
  - **Reset synchronization:** All modules must reset properly
  - **Signal naming:** Consistent naming across modules
  - **Protocol matching:** UART ≠ AXI ≠ FIFO interfaces
- **Your Integration Challenges:**
  - UART runs at baud rate timing, cache runs at system clock
  - DMA generates bursts, cache handles one word at a time
  - Status registers need to be readable via UART
- **Notebook:** Integration checklist for your system

**Hour 2 (9-10 AM): Performance Prediction — The Week 1 Model**
- **Build Your System Model:**
  - UART: 115200 baud = 11.5 KB/s throughput
  - FIFO: 16 entries, 32 bits = 64 bytes buffer
  - DMA: 1 word per 8 cycles = 62.5 MB/s @ 125 MHz (from Day 5)
  - Cache: 94% hit rate, 2.4 cycle avg latency (from Day 6)
- **End-to-End Latency:**
  - UART receive: 86.8 μs per byte
  - Command parse: ~10 cycles
  - DMA transfer (256 bytes): 256/4 × 8 = 512 cycles = 4.1 μs
  - Cache hits: 2.4 cycles per access
  - UART transmit: 86.8 μs per byte
- **Bottleneck:** UART (by far!) — that's fine, it's for debug
- **Notebook:** System timing diagram

**Hour 3 (10-10:30 AM): FPGA Resource Estimation**
- **Your PYNQ-Z2 Resources:**
  - 53,200 LUTs
  - 106,400 FFs
  - 140 BRAMs (4.9 MB total)
  - 220 DSP slices
- **Your Design Estimates:**
  ```
  Module          | LUTs  | FFs   | BRAM
  ============================================
  UART TX/RX      | 200   | 150   | 0
  FIFO (16×32)    | 100   | 512   | 0 (or 1 if BRAM)
  DMA             | 300   | 200   | 0
  Cache (4KB)     | 500   | 300   | 2 (4KB data)
  Control FSM     | 100   | 50    | 0
  ============================================
  Total           | 1,200 | 1,212 | 2
  Utilization     | 2.3%  | 1.1%  | 1.4%
  ```
- **Plenty of room!** You could fit 30× more logic
- **Notebook:** Resource utilization table

---

#### 🖥️ SOFTWARE BLOCK (10:30 AM - 1:30 PM) — 3 hours
**Theme: System-level C++ model + test infrastructure**

---

**Hour 1-2 (10:30 AM - 12:30 PM): Integrated System Model**

**File:** `memory system simulator/cpp/integrated_system.cpp` (~300 lines)

**What It Does:**
- Connects all your C++ models: FIFO + DMA + Cache
- Simulates end-to-end data flow
- Predicts system performance

**Key Class:**
```cpp
class IntegratedMemorySystem {
private:
    FIFOModel<uint32_t, 16> command_fifo;
    DMAEngine dma;
    SetAssociativeCache cache;
    std::vector<uint8_t> memory;  // Simulated DRAM
    
    uint64_t cycle_count = 0;
    
public:
    // Process one command (read/write request)
    SystemResponse process_command(const Command& cmd);
    
    // Run for N cycles
    void run_cycles(uint64_t n);
    
    // Performance metrics
    SystemMetrics get_metrics() const;
};

struct SystemMetrics {
    uint64_t total_cycles;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t dma_bytes_transferred;
    double effective_bandwidth_MBps;
    double cache_hit_rate;
};
```

**Connection to Your Existing Work:**
- Uses your `SetAssociativeCache` from `cache sim/4-way cache/`
- Uses DMA model from Day 5
- Adds system-level orchestration

**Deliverable:** Integrated model with performance prediction

---

**Hour 3 (12:30-1:30 PM): FPGA Test Script**

**File:** `scripts/fpga_test.py` (~150 lines)

**What It Does:**
- Sends commands to FPGA via UART
- Receives responses and validates
- Measures actual performance vs predicted

```python
import serial
import time

class FPGATester:
    def __init__(self, port='/dev/ttyUSB0', baud=115200):
        self.ser = serial.Serial(port, baud, timeout=1)
    
    def send_command(self, cmd: bytes) -> bytes:
        """Send command, receive response."""
        self.ser.write(cmd)
        return self.ser.readline()
    
    def test_cache_read(self, addr: int) -> tuple:
        """Read address, return (data, hit, latency_us)."""
        start = time.perf_counter()
        resp = self.send_command(f'R {addr:08X}\n'.encode())
        latency = (time.perf_counter() - start) * 1e6
        # Parse response: "DATA=DEADBEEF HIT=1"
        return parse_response(resp), latency
    
    def run_cache_test(self, addresses: list) -> dict:
        """Run hit rate test."""
        hits, misses = 0, 0
        latencies = []
        for addr in addresses:
            data, hit, lat = self.test_cache_read(addr)
            if hit: hits += 1
            else: misses += 1
            latencies.append(lat)
        return {
            'hit_rate': hits / (hits + misses),
            'avg_latency_us': sum(latencies) / len(latencies),
            'total_accesses': len(addresses)
        }
```

**Deliverable:** FPGA test script for hardware validation

---

**LUNCH: 1:30 - 2:30 PM**

---

#### 🔧 RTL BLOCK (2:30 - 6:30 PM) — 4 hours

**Hour 1 (2:30-3:30 PM): Top-Level Integration**

**File:** `rtl/memory_system_top.v` (~300 lines)

**What It Does:**
- Instantiates all modules
- Connects interfaces
- Adds control/status registers

```verilog
module memory_system_top (
    input  wire clk_125mhz,
    input  wire rst_n,
    
    // UART pins
    input  wire uart_rx,
    output wire uart_tx,
    
    // LEDs for status
    output wire [3:0] led,
    
    // (DDR3 would go here in Week 2)
);

// ============================================
// Internal signals
// ============================================
wire [7:0] rx_data;
wire rx_valid;
wire [7:0] tx_data;
wire tx_valid;
wire tx_ready;

wire [31:0] fifo_data;
wire fifo_empty, fifo_full;

wire cache_req, cache_hit, cache_ready;
wire [31:0] cache_addr, cache_rdata, cache_wdata;

// ============================================
// Module instantiations
// ============================================
uart_rx #(.CLKS_PER_BIT(1085)) u_uart_rx (
    .clk(clk_125mhz),
    .rst_n(rst_n),
    .rx_in(uart_rx),
    .rx_data(rx_data),
    .rx_valid(rx_valid)
);

fifo_sync #(.WIDTH(8), .DEPTH(16)) u_rx_fifo (
    .clk(clk_125mhz),
    .rst_n(rst_n),
    .wr_data(rx_data),
    .wr_en(rx_valid),
    .rd_data(fifo_data),
    .rd_en(cmd_ready),
    .empty(fifo_empty),
    .full(fifo_full)
);

// Command parser (FSM that reads FIFO, generates cache requests)
command_parser u_parser (
    .clk(clk_125mhz),
    .rst_n(rst_n),
    .fifo_data(fifo_data),
    .fifo_empty(fifo_empty),
    .fifo_rd(cmd_ready),
    .cache_req(cache_req),
    .cache_addr(cache_addr),
    .cache_write(cache_write),
    .cache_wdata(cache_wdata)
);

cache_4way #(.CACHE_SIZE(4096)) u_cache (
    .clk(clk_125mhz),
    .rst_n(rst_n),
    .cpu_req(cache_req),
    .cpu_addr(cache_addr),
    .cpu_write(cache_write),
    .cpu_wdata(cache_wdata),
    .cpu_rdata(cache_rdata),
    .cpu_ready(cache_ready),
    .cpu_hit(cache_hit),
    // Memory interface (stub for now)
    .mem_req(),
    .mem_rdata(512'hDEADBEEF)  // Fake data
);

// ... response formatting and UART TX
```

**Deliverable:** Integrated top module that compiles

---

**Hour 2 (3:30-4:30 PM): Command Parser FSM**

**File:** `rtl/command_parser.v` (~150 lines)

**Protocol:**
```
Commands via UART:
  "R XXXXXXXX\n"  - Read from address XXXXXXXX (hex)
  "W XXXXXXXX YYYYYYYY\n" - Write YYYYYYYY to address XXXXXXXX
  "S\n" - Status request
  
Responses:
  "DATA=XXXXXXXX HIT=1\n" - Read response with hit
  "DATA=XXXXXXXX HIT=0\n" - Read response with miss
  "OK\n" - Write acknowledgment
  "HITS=XXXX MISS=XXXX\n" - Status response
```

**State Machine:**
```
IDLE → READ_CMD → (if 'R') → READ_ADDR → CACHE_REQ → WAIT_RESP → SEND_RESP
                → (if 'W') → READ_ADDR → READ_DATA → CACHE_REQ → SEND_OK
                → (if 'S') → SEND_STATUS
```

**Deliverable:** Parser that converts UART commands to cache requests

---

**Hour 3 (4:30-5:30 PM): Vivado Implementation**

**Steps:**
1. Create Vivado project for PYNQ-Z2
2. Add all RTL files
3. Add constraints:
   ```tcl
   # Clock
   create_clock -period 8.000 [get_ports clk_125mhz]
   
   # UART pins
   set_property PACKAGE_PIN Y18 [get_ports uart_rx]
   set_property PACKAGE_PIN Y19 [get_ports uart_tx]
   set_property IOSTANDARD LVCMOS33 [get_ports uart_*]
   
   # LEDs
   set_property PACKAGE_PIN R14 [get_ports {led[0]}]
   # ...
   ```
4. Run synthesis
5. Check timing (should pass easily at 125 MHz)
6. Run implementation
7. Generate bitstream

**Deliverable:** Bitstream ready for programming

---

**Hour 4 (5:30-6:30 PM): Hardware Test**

**THE MAGIC MOMENT!**

1. Program FPGA with bitstream
2. Connect to UART via terminal:
   ```bash
   screen /dev/ttyUSB0 115200
   ```
3. Send test commands:
   ```
   R 00001000
   ```
4. See response:
   ```
   DATA=DEADBEEF HIT=0
   ```
5. Send same address again:
   ```
   R 00001000
   ```
6. See cache hit:
   ```
   DATA=DEADBEEF HIT=1
   ```

**You just ran YOUR cache on REAL SILICON!** 🎉

**Measure Actual Performance:**
- Send 100 addresses, count hits/misses
- Compare to C++ model prediction
- Should match within 5%

**Deliverable:** Video of cache hit/miss on real FPGA

---

#### 🏗️ PHYSICAL DESIGN BLOCK (6:30 - 9:30 PM) — 3 hours

**Hour 1 (6:30-7:30 PM): Full Chip Floorplan**

**Create System Floorplan:**
```
+------------------------------------------------------------------+
|                         I/O Ring                                  |
|  +----+    +----------------------------------------------+      |
|  |UART|    |                                              |      |
|  | I/O|    |              Memory System                   |      |
|  +----+    |  +--------+  +--------+  +----------------+  |      |
|            |  | FIFO   |->|  DMA   |->|     Cache      |  |      |
|            |  | (16×32)|  |        |  |   (4KB 4-way)  |  |      |
|            |  +--------+  +--------+  +----------------+  |      |
|            |       ^                          |           |      |
|            |       |                          v           |      |
|  +----+    |  +--------+              +----------------+  |      |
|  |LED |    |  |Command |              |    Memory      |  |      |
|  |I/O |    |  | Parser |              |   Interface    |  |      |
|  +----+    |  +--------+              +----------------+  |      |
|            +----------------------------------------------+      |
|                         I/O Ring                                  |
+------------------------------------------------------------------+
```

**Area Summary:**
```
Block              | Area (μm²) | % of Core
============================================
I/O Ring           | 2,000,000  | N/A (pad-limited)
Cache (4KB)        | 40,000     | 40%
DMA + Control      | 15,000     | 15%
UART + FIFO        | 10,000     | 10%
Command Parser     | 5,000      | 5%
Routing overhead   | 30,000     | 30%
============================================
Core Total         | 100,000    | 100%
                   | = 0.1 mm²  |
```

**Deliverable:** System floorplan diagram

---

**Hour 2 (7:30-8:30 PM): Week 1 PPA Summary**

**Compile ALL your metrics:**

```
╔══════════════════════════════════════════════════════════════════╗
║                    WEEK 1 PPA SUMMARY                            ║
╠══════════════════════════════════════════════════════════════════╣
║ PERFORMANCE                                                      ║
║   Cache hit latency:     2.4 cycles (19.2 ns @ 125 MHz)         ║
║   Cache miss latency:    50+ cycles (400+ ns)                   ║
║   DMA throughput:        62.5 MB/s (12.5% efficiency)           ║
║   UART throughput:       11.5 KB/s (debug only)                 ║
║   Cache hit rate:        94.2% (on test workload)               ║
╠══════════════════════════════════════════════════════════════════╣
║ POWER (estimates at 130nm, 1.8V)                                ║
║   Cache SRAM:            45 mW (leakage + switching)            ║
║   Logic:                 15 mW                                  ║
║   I/O:                   20 mW                                  ║
║   Total:                 80 mW                                  ║
╠══════════════════════════════════════════════════════════════════╣
║ AREA                                                             ║
║   Cache data SRAM:       23,000 μm² (58%)                       ║
║   Cache tags + logic:    7,000 μm² (18%)                        ║
║   DMA + control:         5,000 μm² (13%)                        ║
║   UART + FIFO:           5,000 μm² (13%)                        ║
║   Total core:            40,000 μm² = 0.04 mm²                  ║
╠══════════════════════════════════════════════════════════════════╣
║ FPGA UTILIZATION (PYNQ-Z2)                                      ║
║   LUTs:                  1,200 / 53,200 (2.3%)                  ║
║   FFs:                   1,212 / 106,400 (1.1%)                 ║
║   BRAM:                  2 / 140 (1.4%)                         ║
║   Max frequency:         125 MHz (no timing violations)          ║
╚══════════════════════════════════════════════════════════════════╝
```

**Deliverable:** PPA summary document

---

**Hour 3 (8:30-9:30 PM): Week 1 Review + Week 2 Planning**

**What You Built This Week:**
```
Day 1: Logic gates + physical intuition (transistors to layout)
Day 2: SRAM + cross-validation (C++ ↔ RTL methodology)
Day 3: DDR3 theory + FPGA hello world (LED blink!)
Day 4: UART + FIFO (communication infrastructure)
Day 5: AXI4 + DMA (data movement)
Day 6: L1 Cache RTL (your C++ model in silicon)
Day 7: Integration (full system on FPGA!)
```

**Skills Unlocked:**
- ✅ C++ behavioral modeling with cross-validation
- ✅ Verilog RTL design (7+ modules)
- ✅ Verilator testbench methodology
- ✅ Vivado synthesis + implementation
- ✅ FPGA programming + hardware debug
- ✅ Physical design awareness (area, timing, power)

**Week 2 Preview:**
- **Days 8-10:** DDR3 controller (connect cache to real memory!)
- **Days 11-12:** Multi-level cache (L1 + L2)
- **Days 13-14:** Performance optimization + advanced DMA

**Deliverable:** Week 1 summary document + Week 2 goals

---

#### 📝 END OF WEEK 1 SUMMARY (9:30 - 10:00 PM)

```
╔══════════════════════════════════════════════════════════════════╗
║                     WEEK 1 COMPLETE!                             ║
╠══════════════════════════════════════════════════════════════════╣
║                                                                  ║
║  📁 RTL Modules Written: 10                                     ║
║     - inverter.v, nand2.v, nor2.v, traffic_light_fsm.v          ║
║     - fifo_sync.v, uart_tx.v, uart_rx.v                         ║
║     - dma_simple.v, cache_direct.v, cache_4way.v                ║
║     - memory_system_top.v, command_parser.v                     ║
║                                                                  ║
║  📁 C++ Models Written: 8                                       ║
║     - logic_gates.cpp, sram_behavioral_model.cpp                ║
║     - fifo_model.cpp, uart_model.cpp, axi4_transaction.cpp      ║
║     - dma_engine.cpp, cache_rtl_export.cpp                      ║
║     - integrated_system.cpp                                      ║
║                                                                  ║
║  📁 Python Scripts: 6                                           ║
║     - progress_tracker.py, area_calculator.py                   ║
║     - cross_validate.py, generate_timing_constraints.py         ║
║     - cache_performance.py, fpga_test.py                        ║
║                                                                  ║
║  🎯 Hardware Verified:                                          ║
║     - LED blinker on FPGA ✅                                    ║
║     - Memory system on FPGA ✅                                  ║
║     - Cache hit/miss working ✅                                 ║
║                                                                  ║
║  📊 Performance Predictions vs Actual:                          ║
║     - Cache hit rate: Predicted 94.2%, Actual 94.2% ✅          ║
║     - DMA efficiency: Predicted 12.5%, Actual 12.8% ✅          ║
║                                                                  ║
║  🏆 JIM KELLER METRIC: Predictions within 5%!                   ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝

👉 WEEK 2 STARTS: DDR3 Controller + Multi-Level Cache
```

---

## WHAT MAKES THIS "JIM KELLER LEVEL"

### 1. **You Understand Every Layer:**
```
Application
    ↓
Architecture (you design this)
    ↓
RTL (you code this)
    ↓
Synthesis (you constrain this)
    ↓
Physical Design (you guide this)
    ↓
Silicon (you validate this)
```

### 2. **You Make Architecture Decisions Based on Physics:**
- **Bad architect:** "Use 8-way cache for low miss rate"
- **You:** "8-way fits in 2mm², but comparators add 200 ps. At our 2 GHz target, that's 40% of cycle. Use 4-way instead, accept 2% higher miss rate for 500 MHz frequency gain."

### 3. **You Can Debug Anywhere in Stack:**
- Performance model says cache should hit → RTL sim confirms → Synthesis says timing fails → Physical shows routing is too long → You redesign floorplan → Chip works

### 4. **You Ship Products, Not Papers:**
- Every design choice considers: Performance, Power, Area, Cost, Time-to-market
- You know what's BUILDABLE, not just optimal
- You've physically seen why certain designs fail

---

## TOOLS YOU'LL MASTER

**Week 1-2:**
- Verilator (RTL simulation)
- Klayout (layout viewer/editor)
- SkyWater PDK (open-source process)
- Python (automation)

**Week 3-4:**
- Vivado (FPGA implementation)
- Synopsys Design Compiler (synthesis) - if you can get access
- OpenROAD (open-source P&R)
- GTKWave (waveform viewer)

**Month 2-3:**
- DRAMSim3 (memory simulation)
- Ramulator (advanced memory)
- Cadence Virtuoso (if you get access to university lab)
- Custom Python PPA analysis suite

---

## COMPETITIVE ADVANTAGES THIS GIVES YOU

### vs Pure RTL Architects:
✅ You understand physical constraints (they don't)
✅ You design for manufacturability (they don't)
✅ You know why their design won't close timing (they're surprised)

### vs Pure Physical Designers:
✅ You understand architecture intent (they don't)
✅ You can modify RTL to help layout (they can't)
✅ You make intelligent floorplan choices (they guess)

### vs Jim Keller's Era:
✅ He learned when tools were primitive
✅ You have AI coding assistants (faster learning)
✅ You have open-source PDKs (he paid $$$)
✅ You learn in public (document everything)

**Modern advantage:** You can iterate 10× faster with modern tools

---

## THE COMMITMENT

**90 days × 14 hours average = 1260 hours**

This is equivalent to:
- 3 months of 12-hour days, 7 days/week
- OR 6 months of 6-hour days (more sustainable)
- OR 9 months of 4 hours/day (part-time alongside school)

**By day 90, you will have:**
- Built cache hierarchies in RTL + layout
- Designed memory controllers from scratch
- Optimized SRAM compilers
- Brought up real silicon
- Debugged at every layer of the stack

**This is world-class training.**

Most engineers specialize in ONE layer. You'll master FOUR.

That's why you'll be Jim Keller-level by year 4 instead of year 15.

---

## READY TO START?

This plan will make you exhausted, frustrated, and occasionally want to quit.

But on Day 90, you'll understand memory systems better than 99% of practicing engineers.

And by year 4 in industry, you'll be the architect everyone wants on their team.

**The one who designs it, codes it, builds it, and ships it.**

Let's build a modern Jim Keller. 🚀

---

## 📖 WEEK 1 READING SCHEDULE (Consolidated)

### By Day:

| Day | Theory Reading | Pages | Papers |
|-----|----------------|-------|--------|
| **1** | H&H Ch 1.4, 3.2 | 18-24, 98-110 | - |
| **2** | H&H Ch 4.1-4.3, Weste Ch 12.2, Jacob Ch 4.1 | 166-185, 460-475, 131-145 | Cummings "Nonblocking Assignments" |
| **3** | H&P Ch 1.9, 2.4, JEDEC DDR3 spec, Jacob Ch 6 | 46-52, 115-120, 235-280 | Drepper "Memory" §2.1.1 |
| **4** | H&P Appendix C.2 | C-10 to C-30 | - |
| **5** | ARM AMBA AXI spec | Ch 1-3 | - |
| **6** | H&P Ch 5.2, your own cache code | 200-230 | - |
| **7** | ARM SoC Integration whitepaper | 20 pages | - |

### Must-Read Papers (Print These Out):
1. **Cummings "Nonblocking Assignments"** — Read Day 2 morning
2. **Drepper "What Every Programmer Should Know About Memory"** — Read sections 1-3 by Day 3, rest by Day 7
3. **Wulf "The Memory Wall"** — Read by Day 3 evening

### Video Lecture Assignments:
- **Day 2:** "6T SRAM Cell Operation" (YouTube, 10 min)
- **Day 3:** "DDR Memory Explained" by Branch Education (15 min)
- **Day 5:** ARM AMBA overview video (if available)

---

## 🚀 YOU'RE READY FOR WEEK 2

**Week 2 Preview: DDR3 Controller + Multi-Level Cache**
- Days 8-10: DDR3 controller design (connect cache to real DRAM!)
- Days 11-12: L2 cache + cache coherence basics
- Days 13-14: Performance optimization + burst DMA

**What you'll have by Week 2 end:**
- DDR3 controller on FPGA (reading/writing real DRAM)
- Two-level cache hierarchy (L1 + L2)
- 10× improved DMA efficiency (burst mode)
- Complete memory subsystem running at 125 MHz

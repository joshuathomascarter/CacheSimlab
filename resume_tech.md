# JOSHUA CARTER

**Montreal, QC** | josh.carter@email.com | (XXX) XXX-XXXX | linkedin.com/in/joshcarter | github.com/joshcarter

---

## SUMMARY

Computer Engineering student at Concordia University with hands-on experience designing embedded hardware controllers and low-level systems software in C/C++ and SystemVerilog. Built cycle-accurate memory subsystem simulators from scratch — including FSM-based controllers, DDR protocol timing enforcement, and automated hardware validation pipelines. Strong foundation in object-oriented design, debugging complex systems, embedded logic, Linux/Unix development, and RTL-to-software cross-validation. Seeking embedded software, systems engineering, hardware validation, or software development internships.

---

## TECHNICAL PROJECTS

### Embedded Memory Subsystem Simulator & Hardware Validation Suite
**Personal Project / Ongoing** | C/C++17, SystemVerilog, Verilog, Python, CMake, Make, GDB, Linux/macOS

Designed and built a full-stack memory subsystem simulation and validation platform from scratch — combining object-oriented C++ software models, RTL hardware descriptions, and automated cross-validation tooling. ~10,000+ lines across C++, SystemVerilog, and Python.

**Cache Subsystem (C++17, OOP, Python):**
- Implemented **direct-mapped** and **4-way set-associative** cache simulators using object-oriented C++17 with polymorphic eviction policies (LRU, FIFO, Random) via class hierarchies and virtual dispatch
- Designed modular architecture using **smart pointers**, **RAII patterns**, and clean header/source separation for maintainability
- Built address decomposition logic (tag/index/offset bit-field parsing) and performance statistics tracking
- Developed **Python cross-validation framework** to independently verify C++ model correctness — automated regression testing across configurations
- Implemented **reuse-distance analysis** and **working-set estimation** tools for memory hierarchy research

**DRAM Controller & Embedded Protocol Engine (C++17, FSM Design):**
- Designed a **cycle-accurate DRAM bank state machine** (5-state FSM: IDLE → ACTIVATING → ACTIVE → PRECHARGING → REFRESHING) — core embedded controller design pattern
- Implemented **DDR protocol timing enforcement** (tRCD, tCAS, tRP, tRFC, tREFI) — validating signal-level timing constraints analogous to communication protocol compliance
- Built **FCFS and FR-FCFS memory scheduling** algorithms with request queuing, priority handling, and comparative benchmarking
- Designed an **IDD current-based power model** computing active, standby, and refresh power dissipation — hardware-aware energy analysis
- Implemented **refresh controllers** (per-bank and all-bank) with bandwidth-loss measurement
- Created a **memory channel coordinator** managing rank/bank-level parallelism and multi-channel arbitration
- Built Python **visualization and analysis tools**: timing diagrams (Matplotlib), power breakdown charts, scheduling policy comparison dashboards

**SRAM Array — RTL Design & Hardware Validation (SystemVerilog, Verilog, C++):**
- Wrote a **cycle-accurate SRAM behavioral model** in C++ as a golden-reference for hardware validation
- Designed **SRAM array** in Verilog and SystemVerilog — both behavioral and **gate-level explicit** variants
- Built an **SRAM controller with read/write FSM** in SystemVerilog — embedded controller with state-machine-driven I/O
- Created **end-to-end hardware validation pipeline**: C++ model generates test vectors → RTL testbench consumes them → Python script diffs outputs and flags mismatches
- Validated RTL correctness using **golden test vector methodology** — systematic debugging of hardware/software discrepancies

**Build Systems, Testing & Debugging:**
- Configured **CMake and Makefile** build systems with modular multi-target project structure
- Wrote automated **test harnesses** in C++ with systematic pass/fail verification and regression testing
- Debugged complex multi-language systems using **GDB/LLDB**, log-based tracing, and differential analysis
- Cross-language validation (C++ ↔ Python ↔ SystemVerilog) as a core design principle — catching bugs across abstraction layers
- Version-controlled entire project with **Git** — branching, commit history, structured development workflow

---

### DRAM Architecture Interview Deep-Dive Series
**Self-Directed Research** | Technical Writing

- Produced 5 in-depth technical write-ups on DRAM architecture topics: bank FSM design, memory scheduling trade-offs, refresh overhead analysis, power modeling (IDD parameters), and 3D stacking / HBM bandwidth calculations
- Each write-up cross-references actual simulator code with detailed engineering rationale

---

## TECHNICAL SKILLS

| Category | Technologies |
|---|---|
| **Languages** | C, C++17, Python, SystemVerilog, Verilog, Bash/Shell scripting |
| **OOP & Design** | Class hierarchies, polymorphism, RAII, smart pointers, modular architecture, design patterns |
| **Embedded / Hardware** | FSM design, SRAM/DRAM controllers, RTL design, DDR protocol timing, SoC memory subsystems, testbench development |
| **Memory & Systems** | Cache hierarchies, memory management, address mapping, scheduling algorithms, power modeling, HBM/3D stacking |
| **Debugging & Testing** | GDB/LLDB, log-based tracing, golden-reference validation, regression testing, cross-validation pipelines |
| **Tools & Platforms** | Git, CMake, Make, GCC/Clang, Linux/macOS terminal, VS Code, command-line development |
| **Python Ecosystem** | NumPy, Matplotlib, data visualization, automation scripting, trace analysis |

---

## EDUCATION

### Bachelor of Computer Engineering (In Progress)
**Concordia University** — Montreal, QC | 2025 – Present

Relevant coursework: Digital Logic Design, Computer Architecture, Data Structures & Algorithms, Circuit Analysis, Object-Oriented Programming, Linear Algebra

### High School Diploma
**Graduated November 2023** — Gold Coast, Australia
- **Academic Dean's List — Every Year, 2017–2023**

---

## ADDITIONAL EXPERIENCE

### STEM Tutor — A-Team Tutoring | Gold Coast, Australia
**July 2024 – January 2026**
- Tutored mathematics, physics, and computing subjects in-person and online — translating complex technical concepts for diverse learners
- Managed scheduling, session planning, and progress tracking independently across multiple students

### Labourer / Procurement — Gold Coast Bricklaying | Gold Coast, Australia
**June 2023 – October 2024**
- Coordinated materials procurement and inventory logistics; worked effectively in a multidisciplinary team environment

### Barista | Gold Coast, Australia
**January 2021 – April 2023**
- High-volume operations in fast-paced environment; trained new staff, managed inventory systems

---

## LEADERSHIP & ATHLETICS

- **Australian National Cricket Representative** — Under-16s & Under-17s
- **Captain, Victoria State Cricket Team** — 2023 Season
- Elite-level competitive sport demonstrating leadership, discipline, performance under pressure, and team coordination

---

## REFERENCES

Available upon request.

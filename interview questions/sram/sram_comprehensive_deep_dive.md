# Comprehensive SRAM Deep Dive: SNM Analysis, SkyWater SKY130 PDK, Capacitance, IR Drop & Logical Effort

## Source Material
- **Paper:** "Static Noise Margin Analysis of SRAM Cell for High Speed Application" — Mukherjee, Shovan, Banerjee (2010, IJCSI Vol.7 Issue 4)
- **PDK:** SkyWater SKY130 Open-Source PDK (130nm CMOS process)
- **Textbook:** Weste & Harris, *CMOS VLSI Design*, 4th Ed. — Chapter 12.2 (SRAM) and Chapter 4.4–4.5 (Logical Effort / Linear Delay Model)

---

# PART 1: STATIC NOISE MARGIN ANALYSIS (The Paper)

## 1.1 What Is Static Noise Margin (SNM)?

SNM is the **maximum amount of DC noise voltage that can be tolerated at the inputs of a cross-coupled inverter pair (the SRAM cell) before the cell flips state**. It is the single most important metric for determining whether an SRAM cell will reliably hold data.

### Why SNM Matters — The Physical Intuition

An SRAM cell is two inverters connected in a loop. Each inverter's output drives the other's input. The cell is stable because of **positive feedback**: if Q is high and Q̄ is low, each inverter reinforces the other. But noise can push the voltages toward the metastable point (where both inverters are in the transition region), and if noise is large enough, the cell flips.

```
          VDD                    VDD
           │                      │
       ┌───┤ P1                P2 ├───┐
       │   │                      │   │
  Q ───┤   ├───── Q̄    Q̄ ────┤   ├─── Q
       │   │                      │   │
       └───┤ N1                N2 ├───┘
           │                      │
          GND                    GND

 Inverter 1: Input=Q̄, Output=Q
 Inverter 2: Input=Q, Output=Q̄
```

SNM answers: **"How much noise voltage can I inject between these inverters before the cell loses its data?"**

### The Butterfly Diagram Method

The butterfly diagram is how we measure SNM graphically. Here's how it works step-by-step:

**Step 1:** Take Inverter 1. Sweep its input (Q̄) from 0 to VDD. Plot Q (output) vs Q̄ (input). This gives you the **Voltage Transfer Characteristic (VTC)** of Inverter 1.

**Step 2:** Take Inverter 2. Sweep its input (Q) from 0 to VDD. Plot Q̄ (output) vs Q (input). This gives you the VTC of Inverter 2.

**Step 3:** Mirror/rotate the second VTC so both are plotted on the same axes (Q vs Q̄). The two curves form a "butterfly" shape.

**Step 4:** The two curves intersect at **three points**:
- Two **stable** operating points (one near (VDD, 0) and one near (0, VDD))
- One **metastable** point (near (VDD/2, VDD/2))

**Step 5:** Find the **largest square** that fits inside each of the two "eyes" (lobes) of the butterfly. The side length of the **smaller** square = SNM.

```
  Q (Volts)
  VDD ─────────────────────────╮ Stable point: Q=VDD, Q̄=0
   │               ╱──────────╯
   │              ╱ ┌──────┐
   │             ╱  │ SNM  │ ← Largest square that fits
   │            ╱   │square│     in this "eye"
   │           ╱    └──────┘
   │     ─────╱─────────── ← Metastable point
   │    ┌────╱─┐
   │    │SNM╱  │ ← Other eye's square
   │    └──╱───┘
   │      ╱
   │─────╱
   0──────────────────────────── Q̄ (Volts)
   0                           VDD

   SNM = min(side of square in eye 1, side of square in eye 2)
```

### Mathematical Definition of SNM

The formal definition comes from Seevinck (1987):

$$SNM = \max(V_n) \text{ such that the cell remains bistable when noise sources } \pm V_n \text{ are inserted}$$

For a symmetric cell (both inverters identical), this simplifies to finding where the slope of the VTC equals −1:

$$SNM \approx V_{DD} \cdot \left(\frac{\beta_R - 1}{\beta_R + 1}\right) - V_{th}$$

Where:
- $\beta_R$ = Cell Ratio (explained below)
- $V_{th}$ = threshold voltage of the access/drive NMOS

This is an approximation. The exact value requires SPICE simulation or the graphical butterfly method.

## 1.2 The 6T SRAM Cell — Transistor-Level Analysis

### Cell Schematic with Sizing Variables

```
                    VDD                         VDD
                     │                           │
                ┌────┤ P1 (W4/L4)        P2 (W3/L3) ├────┐
                │    │                               │    │
           Q ───┤    ├─────────┐   ┌─────────────────┤    ├─── Q̄
                │    │         │   │                  │    │
                └────┤ N1      │   │          N2 ├────┘
                     │(W1/L1)  │   │     (W2/L2)│
                     │         │   │            │
                    GND        │   │           GND
                               │   │
                     WL ───┬───┘   └───┬─── WL
                           │           │
                    N5 (W5/L5)   N6 (W6/L6)
                    (Access)      (Access)
                           │           │
                          BL          BL̄

    N1, N2 = Pull-down (driver) NMOS transistors
    P1, P2 = Pull-up PMOS transistors
    N5, N6 = Access (pass-gate) transistors
```

The paper uses this naming convention:
- **W1/L1** = Pull-down NMOS (driver) — the transistor that pulls the storage node to GND
- **W4/L4** = Pull-up PMOS — the transistor that pulls the storage node to VDD
- **W5/L5** = Access NMOS (pass-gate) — connects storage node to bitline when WL is active

### The Three Critical Ratios

#### Cell Ratio (CR) — Controls Read Stability

$$CR = \frac{W_1/L_1}{W_5/L_5} = \frac{\text{Driver W/L}}{\text{Access W/L}}$$

**What this means physically:**

During a **read operation**, the wordline goes high, turning on the access transistors N5 and N6. Suppose Q=1 (VDD) and Q̄=0 (GND). The bitlines are precharged to VDD. When N5 turns on, nothing much happens on the Q side (both Q and BL are at VDD). But when N6 turns on, there's a problem:

BL̄ is at VDD, Q̄ is at 0. Current flows from BL̄ through N6 into node Q̄. This **raises** Q̄ above ground. If Q̄ rises above the trip point of Inverter 1, the cell **flips** — you've destroyed your data just by reading it!

The Cell Ratio determines **how much Q̄ rises**. The voltage at Q̄ during read (called $V_{read}$) is set by the voltage divider between N6 (access) and N2 (driver):

$$V_{read} = V_{DD} \cdot \frac{R_{N2}}{R_{N2} + R_{N6}} = V_{DD} \cdot \frac{1/g_{m,N2}}{1/g_{m,N2} + 1/g_{m,N6}}$$

In the linear region (since N2 operates near VDS ≈ 0):

$$V_{read} \approx V_{DD} \cdot \frac{(W_5/L_5)}{(W_1/L_1) + (W_5/L_5)} = \frac{V_{DD}}{1 + CR}$$

**Worked example from the paper (180nm):**
```
VDD = 1.8V, CR = 1.0:
V_read = 1.8 / (1 + 1.0) = 0.90V → This is ABOVE Vth (0.45V)! Cell flips!

CR = 1.6:
V_read = 1.8 / (1 + 1.6) = 0.69V → Still dangerously high

CR = 2.0:
V_read = 1.8 / (1 + 2.0) = 0.60V → Marginal

CR = 3.0:
V_read = 1.8 / (1 + 3.0) = 0.45V → Equals Vth, barely safe
```

**Rule of thumb:** CR ≥ 1.5 for reliable reads at 180nm. At advanced nodes (7nm), CR ≥ 2.0 because Vth/VDD ratio is higher.

#### Pull-up Ratio (PR) — Controls Write Ability

$$PR = \frac{W_4/L_4}{W_6/L_6} = \frac{\text{Pull-up W/L}}{\text{Access W/L}}$$

**What this means physically:**

During a **write operation**, we want to flip the cell. Suppose Q=1 and we want to write 0. We drive BL=0, BL̄=1, and raise WL. Now current flows from Q (at VDD) through N5 to BL (at 0). But P1 is fighting us — it's trying to keep Q at VDD by sourcing current from VDD.

For the write to succeed, **N5 must overpower P1**. The access transistor must be able to pull Q below the trip point despite P1 fighting back. A **lower PR** means a weaker pull-up (P1), making writes easier.

The voltage that N5 can pull Q down to:

$$V_{write} = V_{DD} \cdot \frac{R_{N5}}{R_{N5} + R_{P1}} \approx V_{DD} \cdot \frac{(W_4/L_4)}{(W_4/L_4) + (W_6/L_6)} = \frac{V_{DD} \cdot PR}{PR + 1}$$

For a successful write, $V_{write}$ must be **below** the trip point of Inverter 2:

$$V_{write} < V_{trip} \approx \frac{V_{DD}}{2}$$

$$\frac{V_{DD} \cdot PR}{PR + 1} < \frac{V_{DD}}{2}$$

$$PR < 1$$

**But wait** — if PR < 1, the pull-up is weaker than the access transistor, which also weakens hold stability. This is the **read-write conflict** — the fundamental tradeoff in 6T SRAM design:

```
    ┌──────────────────────────────────────────────────┐
    │              THE SRAM DESIGN CONFLICT             │
    │                                                  │
    │  READ wants:  Strong driver, Weak access         │
    │               → Large CR (big N1, small N5)      │
    │               → Makes cell hard to flip (write)  │
    │                                                  │
    │  WRITE wants: Strong access, Weak pull-up        │
    │               → Small PR (small P1, big N5)      │
    │               → Makes cell easy to flip (bad!)   │
    │                                                  │
    │  ACCESS transistor N5 has conflicting demands:    │
    │  - Read wants N5 SMALL (less disturbance)        │
    │  - Write wants N5 LARGE (more pull-down force)   │
    └──────────────────────────────────────────────────┘
```

#### Data Retention Voltage (DRV)

DRV is the **minimum supply voltage at which the SRAM cell can still hold data**. Below DRV, the cell loses its bistable behavior — both inverters operate in the subthreshold region and can't maintain distinct 0/1 states.

$$DRV \approx 2 \cdot V_{th,sub} \cdot \ln(2) \cdot n$$

Where:
- $V_{th,sub}$ = subthreshold slope voltage (≈ kT/q × ideality factor)
- $n$ = subthreshold ideality factor (typically 1.3–1.5)

For 180nm technology: **DRV ≈ 0.6V** (from the paper)

This means you CANNOT lower VDD below 0.6V during retention (sleep mode) at 180nm without risking data loss.

**DRV scales with technology:**
```
Technology    Typical DRV    VDD       DRV/VDD
─────────────────────────────────────────────
180nm         0.6V           1.8V      33%
130nm         0.45V          1.2V      38%
65nm          0.35V          1.0V      35%
45nm          0.30V          0.9V      33%
28nm          0.25V          0.85V     29%
7nm           0.20V          0.75V     27%
```

## 1.3 Paper Results — SNM vs Design Parameters

### SNM vs Cell Ratio (Table from Paper)

The paper simulated a 180nm SRAM cell at VDD = 1.8V with varying CR:

```
Cell Ratio (CR)    SNM (mV)     ΔV_read (mV)     Notes
──────────────────────────────────────────────────────────────
0.8                205          —                 Below minimum, cell unreliable
1.0                210          900               V_read too high
1.2                215          692               Marginal
1.4                219          643               Acceptable
1.6                223          600               Good
1.8                225          563               Better
2.0                227          —                 Target for reliability
2.5                230          —                 Diminishing returns
```

**Key insight:** SNM increases with CR but with **diminishing returns** beyond CR ≈ 2.0. Going from CR=0.8 to CR=1.6 gains 18mV, but going from 1.6 to 2.5 only gains 7mV. The area cost of larger drivers isn't worth minimal SNM improvement.

### SNM vs Pull-up Ratio

```
Pull-up Ratio (PR)    SNM (mV)    Write Margin Impact
────────────────────────────────────────────────────────
3.0                   210         Hard to write
3.2                   215         Moderate
3.4                   220         Improving
3.6                   223         Good balance
3.8                   225         Easy to write
4.0                   227         Very easy, hold margin drops
```

**Important:** The paper defines PR = W4/W6 where W4 is pull-up and W6 is access. A LARGER PR means the pull-up is stronger relative to access — this makes writes HARDER. The SNM increases with PR because stronger pull-up improves hold stability, but at the cost of write margin.

### SNM vs Supply Voltage

```
VDD (V)    SNM (mV)    SNM/VDD    Comments
──────────────────────────────────────────────────
0.6        ~0          0%         At DRV, no margin
0.8        50          6.25%     Barely functional
1.0        100         10%       Marginal
1.2        140         11.7%     Low-power mode
1.4        175         12.5%     Approaching target
1.6        205         12.8%     Good
1.8        225         12.5%     Nominal VDD
2.0        240         12%       Overvoltage
```

**The SNM scales approximately linearly with VDD** above DRV:

$$SNM \approx k \cdot (V_{DD} - DRV)$$

Where $k \approx 0.19$ for this 180nm technology.

### SNM vs Threshold Voltage

```
Vth (mV)    SNM (mV)    Analysis
─────────────────────────────────────────────
200         140         Low Vth → fast but noisy
250         170         
300         195         
350         215         
400         230         
450         243         Nominal 180nm
500         250         High Vth → slow but stable
```

**Higher Vth → Higher SNM** because the noise must overcome a bigger barrier to trip the inverter. But higher Vth means slower transistors (less overdrive). This is the **speed-stability tradeoff**.

## 1.4 Write Margin Analysis (from Paper)

The paper uses the **BL sweeping method** for write margin:

**Definition:** Write Noise Margin (WNM) is the maximum BL voltage at which the cell can still be flipped.

**Procedure:**
1. Hold WL = VDD (access transistors on)
2. Hold BL̄ = VDD (the side we're NOT writing)
3. Sweep BL from VDD down to 0
4. Monitor when the cell flips (Q crosses Q̄)
5. WNM = VDD − V_BL_at_flip

```
  Q, Q̄
  VDD ─── Q (storing '1') ─────────╲
   │                                 ╲──── Q drops
   │                                       below trip
   │                                   ╱── Q̄ rises
   │                                 ╱      above trip
   0 ─── Q̄ (storing '0') ────────╱
                                    │
       VDD ─────────────────── 0   BL voltage sweep
       
       Write margin = VDD - BL voltage at crossover
```

**Write margin results from the paper:**
```
CR     PR     WNM (mV)    Read SNM (mV)    Total Margin
────────────────────────────────────────────────────────
1.0    3.0    850         210              Balanced
1.6    3.0    750         223              Read-biased
1.0    4.0    700         210              Write-biased
1.6    4.0    650         227              Area-expensive
2.0    3.0    680         227              Production target
```

**The tradeoff is clear:** Increasing CR improves read SNM but degrades write margin. The designer must find the sweet spot.

---

# PART 2: SKYWATER SKY130 PDK — REAL PROCESS PARAMETERS

## 2.1 Process Overview

The SkyWater SKY130 is a **130nm CMOS process** — Google's open-source PDK in partnership with SkyWater Technology. It's significant because it's the first production-grade PDK released fully open-source.

**Key Process Parameters:**
```
Parameter                  Value           Notes
──────────────────────────────────────────────────────────
Technology node            130nm           (~0.13μm)
Operating voltage          1.8V            Nominal VDD
Gate oxide thickness       ~5.7nm          For 1.8V devices
Minimum poly gate length   0.15μm         Drawn; effective ~0.13μm
Metal layers               5               M1–M5
Local interconnect         Yes (LI)        Between contacts and M1
Substrate                  P-type          ~1e15 cm⁻³
```

## 2.2 Complete Metal Stack — Physical Dimensions

This is CRITICAL for calculating parasitic capacitance and resistance in SRAM arrays.

### Layer Dimensions

```
Layer              Min Width    Thickness    Min Space    Purpose
───────────────────────────────────────────────────────────────────────
Diffusion (diff)   0.15μm       —            0.27μm      Active regions
Poly               0.15μm       0.18μm       0.21μm      Transistor gates
Local Int. (LI)    0.17μm       0.10μm       0.17μm      Local routing
Metal 1 (M1)       0.14μm       0.35μm       0.14μm      First global
Metal 2 (M2)       0.14μm       0.35μm       0.14μm      Second global
Metal 3 (M3)       0.30μm       0.80μm       0.30μm      Intermediate
Metal 4 (M4)       0.30μm       0.80–2.0μm   0.30μm      Intermediate
Metal 5 (M5)       1.60μm       1.20–2.0μm   1.60μm      Top/Power
```

### Via and Contact Sizes

```
Contact/Via        Size         Resistance      Purpose
─────────────────────────────────────────────────────────
LICON              0.17μm       15 kΩ (each)     LI to Diff/Poly
MCON               0.17μm       152 kΩ (each)    LI to M1
VIA (via)          0.15μm       4,500 Ω (each)   M1 to M2
VIA2               0.20μm       3,410 Ω (each)   M2 to M3
VIA3               0.20μm       3,410 Ω (each)   M3 to M4
VIA4               0.80μm       380 Ω (each)     M4 to M5
```

**CRITICAL NOTE on via resistance:** These are per-via resistances. A single via has thousands of ohms of resistance! This is why you **ALWAYS** use multiple vias in parallel for power connections. If you only use 1 via for a VDD connection carrying 1mA:

$$V_{drop} = I \times R_{via} = 1\text{mA} \times 4500\Omega = 4.5\text{V} \quad \text{← CATASTROPHIC!}$$

Using 10 vias in parallel:

$$V_{drop} = I \times \frac{R_{via}}{N} = 1\text{mA} \times \frac{4500}{10} = 0.45\text{V} \quad \text{← Still 25% of VDD!}$$

Using 100 vias:

$$V_{drop} = 1\text{mA} \times \frac{4500}{100} = 0.045\text{V} = 45\text{mV} \quad \text{← Acceptable (2.5% of VDD)}$$

## 2.3 Sheet Resistance Values — THE KEY TABLE

Sheet resistance ($R_{sh}$) is the resistance of a **square** of material, regardless of the square's size. The resistance of a rectangular wire is:

$$R_{wire} = R_{sh} \times \frac{L}{W}$$

Where $L$ = length, $W$ = width of the wire. The ratio $L/W$ is called "number of squares."

```
Layer                    Sheet Resistance (mΩ/sq)    Ω/sq       Notes
──────────────────────────────────────────────────────────────────────────
Poly                     48,200                      48.2       Very high! Minimize poly routing
Local Interconnect (LI)  12,800                      12.8       High — keep traces short
Metal 1 (M1)             125                         0.125      Standard routing
Metal 2 (M2)             125                         0.125      Standard routing
Metal 3 (M3)             47                          0.047      Thicker → lower R
Metal 4 (M4)             47                          0.047      Thicker → lower R
Metal 5 (M5)             29                          0.029      Thickest → power routing
─────────────────────────────────────────────────────────────────────────
N-diffusion              120,000                     120        Never route on diffusion!
P-diffusion              197,000                     197        Even worse
Deep N-well              2,200,000                   2,200      Substrate isolation
N-well                   1,700,000                   1,700      Body connection
P-well (in deep nwell)   3,050,000                   3,050      Body connection
```

### Worked Example: Wordline Resistance in SKY130

A wordline in a 1024-column SRAM array, routed on poly (worst case) vs M1:

**On Poly (traditional but terrible):**
```
Wordline length = 1024 cells × 0.5μm pitch = 512μm
Poly width = 0.15μm (minimum)
Number of squares = 512 / 0.15 = 3413 squares

R_wordline = 48.2 Ω/sq × 3413 = 164,507 Ω ≈ 164.5 kΩ

With wordline capacitance ≈ 0.5 fF/cell × 1024 = 512 fF:
τ = RC = 164.5kΩ × 512fF = 84.3 ns ← WAY too slow!
```

**On Metal 1 (strapped wordline):**
```
Same length = 512μm
M1 width = 0.14μm (minimum)
Number of squares = 512 / 0.14 = 3657 squares

R_wordline = 0.125 Ω/sq × 3657 = 457 Ω

τ = RC = 457Ω × 512fF = 0.234 ns ← 360× faster!
```

**This is why modern SRAM always straps wordlines with metal.** Even at 130nm, pure poly wordlines are unusable for arrays > ~64 columns.

**On Metal 3 (wide bus for large arrays):**
```
M3 width = 0.30μm
Number of squares = 512 / 0.30 = 1707 squares
R = 0.047 × 1707 = 80.2 Ω
τ = 80.2 × 512fF = 0.041 ns ← Excellent
```

## 2.4 Parasitic Capacitance Values — THE OTHER KEY TABLE

Capacitance in an IC comes from three sources:

### Source 1: Parallel Plate Capacitance (Area Capacitance)

When two conductors overlap, they form a parallel plate capacitor. The capacitance per unit area is:

$$C_{pp} = \frac{\epsilon_0 \cdot \epsilon_r}{t_{ILD}}$$

Where:
- $\epsilon_0$ = 8.854 × 10⁻¹² F/m (permittivity of free space)
- $\epsilon_r$ = relative permittivity of dielectric (SiO₂ ≈ 3.9, low-k ≈ 2.5–3.0)
- $t_{ILD}$ = thickness of inter-layer dielectric (the insulator between the two metal layers)

**SKY130 Parallel Plate Capacitance (aF/μm²):**

```
                    Substrate   LI        M1        M2        M3        M4        M5
────────────────────────────────────────────────────────────────────────────────────────
Poly                94.16       44.81     24.50     16.06     10.01     7.21      —
Local Interconnect  —           114.20    37.56     20.79     11.67     8.03      —
Metal 1             —           —         133.86    34.54     15.03     9.48      —
Metal 2             —           —         —         86.19     20.33     11.34     —
Metal 3             —           —         —         —         84.03     19.63     —
Metal 4             —           —         —         —         —         68.33     —
```

**How to read this table:** The value at (row=M1, col=M2) = 133.86 aF/μm² means: if you have 1 μm² of M1 directly below 1 μm² of M2 (overlapping), the capacitance between them is 133.86 aF (attofarads, 10⁻¹⁸ F).

**Units:** aF/μm² = attofarads per square micrometer. 1 aF = 10⁻¹⁸ F = 10⁻³ fF.

### Source 2: Fringe Capacitance (Edge Capacitance)

When a conductor's edge is near another conductor, the electric field "fringes" around the edge, creating additional capacitance. This is per unit **length** of the edge, not per unit area.

**Fringe Downward (aF/μm)** — larger plate is BELOW the smaller plate:

```
                    Substrate   LI        M1        M2        M3        M4        M5
────────────────────────────────────────────────────────────────────────────────────────
Local Interconnect  51.85       —         —         —         —         —         —
Metal 1             46.72       59.50     —         —         —         —         —
Metal 2             41.22       46.28     67.05     —         —         —         —
Metal 3             43.53       46.71     54.81     69.85     —         —         —
Metal 4             38.11       39.71     42.56     46.38     70.52     —         —
Metal 5             39.91       41.15     43.19     45.59     54.15     82.82     —
```

**Fringe Upward (aF/μm)** — larger plate is ABOVE the smaller plate:

```
                    LI        M1        M2        M3        M4        M5
─────────────────────────────────────────────────────────────────────────
Poly                25.14     16.69     11.17     9.18      6.35      6.49
Local Interconnect  —         34.70     21.74     15.08     10.14     7.64
Metal 1             —         —         48.19     26.68     16.42     12.02
Metal 2             —         —         —         44.43     22.33     15.69
Metal 3             —         —         —         —         42.64     27.84
Metal 4             —         —         —         —         —         46.98
```

### Source 3: Wire-to-Wire (Coupling) Capacitance

Same-layer coupling between adjacent parallel wires. Not in the RCX tables but can be estimated:

$$C_{coupling} \approx \epsilon_0 \epsilon_r \frac{t_{metal}}{s} \times L$$

Where $t_{metal}$ = metal thickness, $s$ = spacing between wires, $L$ = parallel run length.

For M1 in SKY130 (t=0.35μm, s=0.14μm minimum):
$$C_{coupling}/L \approx 3.9 \times 8.854 \times 10^{-12} \times \frac{0.35}{0.14} \approx 86 \text{ aF/μm}$$

### Practical Capacitance Calculation: SRAM Bitline in SKY130

Let's calculate the parasitic capacitance of a **bitline** in a 256-row SRAM array in SKY130, routed on Metal 2:

**Parameters:**
```
Bitline length = 256 rows × 1.0μm cell height = 256μm
Bitline width = 0.14μm (minimum M2)
Bitline runs over M1 (wordlines) and near other M2 bitlines
```

**Step 1: Parallel plate cap to M1 (wordlines below):**
```
Area of BL = 256μm × 0.14μm = 35.84 μm²
C_pp(M2-M1) = 133.86 aF/μm² (from table, but this is M1-to-M2... 
               but we need M2-to-M1, which is the same value)

Wait — recheck: The table shows M1 row, M2 column = 133.86
Actually, parallel plate cap is symmetric: C(M1,M2) = C(M2,M1)

But the BL doesn't overlap ALL of M1 — it only crosses over wordlines.
Assume wordlines are 0.14μm wide, 1 per row:
Overlap area per crossing = 0.14 × 0.14 = 0.0196 μm² per crossing
Total overlap = 0.0196 × 256 = 5.02 μm²

C_pp(M2→M1) = 5.02 × 133.86 = 671.7 aF ≈ 0.67 fF
```

**Wait — this ONLY counts overlap. But actually, the BL also has area cap to the M1 ground plane (VDD/VSS rails). Let's assume 50% of BL area is over M1:**
```
C_pp = 0.5 × 35.84 × 133.86 = 2,398 aF ≈ 2.4 fF
```

**Step 2: Fringe cap from M2 down to M1:**
```
BL perimeter = 2 × (256 + 0.14) ≈ 512.28 μm
But only the long edges fringe (short edges negligible):
Long edge length = 2 × 256 = 512 μm

C_fringe_down(M2→M1) = 512 × 67.05 aF/μm = 34,330 aF ≈ 34.3 fF
```

**Step 3: Fringe cap from M2 down to substrate through everything:**
```
C_fringe_down(M2→sub) = 512 × 41.22 aF/μm = 21,105 aF ≈ 21.1 fF
```

**Wait — we don't double-count.** The fringe values already account for the dominant coupling to the nearest layer. We use the M2→M1 fringe as the primary, since M1 shields the substrate.

**Step 4: Coupling to adjacent bitlines (M2 to M2):**
```
Adjacent BL spacing ≈ 0.14μm (minimum)
Parallel run length = 256μm
M2 thickness = 0.35μm

C_coupling ≈ ε₀εᵣ × (t/s) × L
           = 3.9 × 8.854e-12 × (0.35/0.14) × 256e-6
           = 3.9 × 8.854e-12 × 2.5 × 256e-6
           = 22.1 fF per neighbor
Two neighbors: 44.2 fF
```

**Step 5: Junction capacitance at access transistor drain:**
```
Per cell: C_drain ≈ 0.5 fF (typical for 130nm)
Total from all cells sharing this BL: This is the transistor off-capacitance.
Only 1 cell is active at a time, but all 256 cells contribute junction cap:
C_junction = 256 × 0.1 fF ≈ 25.6 fF (off-state drain junction)
```

**Total bitline capacitance:**
```
C_BL = C_pp + C_fringe + C_coupling + C_junction
     ≈ 2.4 + 34.3 + 44.2 + 25.6
     = 106.5 fF
```

**This matters because:** The sense amplifier must detect a voltage difference on this capacitance. The bitline development equation is:

$$\Delta V_{BL} = \frac{C_{cell}}{C_{BL}} \times V_{DD}$$

With $C_{cell}$ ≈ 1–2 fF (storage node cap), $C_{BL}$ ≈ 106 fF:

$$\Delta V_{BL} = \frac{1.5}{106.5} \times 1.8 \approx 25\text{ mV}$$

The sense amplifier needs to detect this 25 mV signal on a noisy 1.8V bitline. This is why sense amplifiers are one of the most critical analog circuits in an SRAM.

---

# PART 3: CAPACITANCE IN SRAM — COMPLETE EQUATION GUIDE

## 3.1 The Four Types of Capacitance in CMOS

Every wire and transistor in an SRAM has capacitance. Understanding all four types is essential:

### Type 1: Gate Capacitance ($C_{gate}$)

This is the capacitance of the MOSFET gate — the MOS capacitor itself.

$$C_{gate} = C_{ox} \times W \times L$$

Where:
- $C_{ox}$ = gate oxide capacitance per unit area = $\frac{\epsilon_{ox}}{t_{ox}}$
- $W$ = gate width
- $L$ = gate length
- $\epsilon_{ox}$ = $\epsilon_0 \times \epsilon_r$ = 3.45 × 10⁻¹¹ F/m (for SiO₂)
- $t_{ox}$ = gate oxide thickness

**For SKY130 (t_ox ≈ 5.7nm):**

$$C_{ox} = \frac{3.45 \times 10^{-11}}{5.7 \times 10^{-9}} = 6.05 \text{ fF/μm²}$$

**Minimum-sized transistor gate cap:**

$$C_{gate,min} = 6.05 \times 0.15 \times 0.15 = 0.136 \text{ fF}$$

**A transistor with W=0.5μm, L=0.15μm:**

$$C_{gate} = 6.05 \times 0.5 \times 0.15 = 0.454 \text{ fF}$$

**Why this matters for SRAM:** The wordline must charge all the gate capacitances of the access transistors in a row:

$$C_{WL} = N_{columns} \times C_{gate,access} + C_{wire,WL}$$

For 256 columns with W=0.36μm access transistors:
$$C_{gate,access} = 6.05 \times 0.36 \times 0.15 = 0.327 \text{ fF each}$$
$$C_{WL,gates} = 256 \times 0.327 = 83.7 \text{ fF}$$

Plus the wire capacitance calculated in Section 2.4.

### Type 2: Diffusion (Junction) Capacitance ($C_{diff}$)

This is the capacitance of the source/drain PN junctions. It has two components:

$$C_{diff} = C_{bottom} + C_{sidewall}$$

$$C_{bottom} = C_{j0} \times A_D \times \left(1 + \frac{V_R}{V_{bi}}\right)^{-m_j}$$

$$C_{sidewall} = C_{jsw0} \times P_D \times \left(1 + \frac{V_R}{V_{bi}}\right)^{-m_{jsw}}$$

Where:
- $C_{j0}$ = zero-bias junction capacitance per unit area (≈ 1–2 fF/μm² for 130nm)
- $A_D$ = drain area
- $V_R$ = reverse bias voltage across junction
- $V_{bi}$ = built-in potential (≈ 0.7–0.9V)
- $m_j$ = grading coefficient (≈ 0.3–0.5; 0.5 for abrupt, 0.33 for graded)
- $C_{jsw0}$ = zero-bias sidewall capacitance per unit perimeter
- $P_D$ = drain perimeter

**Simplified for quick estimates:**
$$C_{diff} \approx C_{j0} \times W \times L_{diff} + C_{jsw0} \times (2L_{diff} + W)$$

For SKY130 (typical values):
```
C_j0 ≈ 1.5 fF/μm²
C_jsw0 ≈ 0.35 fF/μm
L_diff (source/drain length) ≈ 0.15-0.25 μm
```

**Minimum transistor drain cap:**
$$C_{diff} = 1.5 \times 0.15 \times 0.15 + 0.35 \times (2 \times 0.15 + 0.15) = 0.034 + 0.158 = 0.191 \text{ fF}$$

### Type 3: Wire Capacitance ($C_{wire}$)

Already covered in detail in Section 2.4. The total wire capacitance is:

$$C_{wire} = C_{area} + C_{fringe} + C_{coupling}$$

$$C_{area} = C_{pp} \times W \times L$$
$$C_{fringe} = C_{f} \times 2L$$
$$C_{coupling} = C_{cc} \times L \text{ (per neighbor)}$$

### Type 4: Overlap Capacitance ($C_{ov}$)

The gate electrode overlaps slightly over the source and drain regions due to lateral diffusion during manufacturing. This creates a parallel-plate capacitance:

$$C_{ov} = C_{ov,per\mu m} \times W$$

Typical $C_{ov}$ ≈ 0.2–0.4 fF/μm for 130nm.

**Why this matters:** Overlap capacitance couples the gate signal to the source and drain. When the wordline switches, the overlap capacitance of N5 (access transistor) injects charge into the storage node (through gate-drain overlap). This is called **charge injection** or **clock feedthrough** and can disturb the stored voltage:

$$\Delta V_{storage} = \frac{C_{ov}}{C_{storage} + C_{ov}} \times \Delta V_{WL}$$

For a typical SRAM cell:
$$\Delta V_{storage} = \frac{0.1}{1.5 + 0.1} \times 1.8 = 0.113\text{V} = 113\text{ mV}$$

This 113 mV glitch can reduce the effective SNM. It's one reason why the wordline should not have excessively fast edges.

## 3.2 Total Storage Node Capacitance ($C_{cell}$)

The SRAM cell's storage node capacitance determines how much charge is stored and the bitline development:

$$C_{cell} = C_{gate,inv2} + C_{diff,N1} + C_{diff,P1} + C_{diff,N5} + C_{wire,internal} + C_{ov,N5}$$

Breakdown for a SKY130 SRAM cell (W_n=0.36μm, W_p=0.18μm, W_acc=0.27μm):

```
Component               Formula                              Value (fF)
──────────────────────────────────────────────────────────────────────────
Gate cap of inverter 2  C_ox × (W_N2 + W_P2) × L             0.49
N1 drain diffusion      C_j0 × A_D + C_jsw × P_D             0.25
P1 drain diffusion      C_j0 × A_D + C_jsw × P_D             0.20
N5 source diffusion     C_j0 × A_D + C_jsw × P_D             0.18
Internal wiring (LI)    ~0.5μm of LI routing                  0.05
N5 overlap cap          C_ov × W                              0.08
──────────────────────────────────────────────────────────────────────────
TOTAL C_cell            ═══════════════════════════            ≈ 1.25 fF
```

This ~1.25 fF storage capacitance is typical for 130nm. Compare:
```
Technology   C_cell (fF)    C_BL (fF)    C_cell/C_BL
───────────────────────────────────────────────────────
130nm        1.25           100          1.25%
65nm         0.8            80           1.00%
28nm         0.4            50           0.80%
7nm          0.15           30           0.50%
```

**The ratio gets worse at every node.** This is why modern SRAMs need increasingly sensitive (and carefully designed) sense amplifiers.

## 3.3 The RC Delay Equation — Elmore Delay Model

The Elmore delay model gives the propagation delay of an RC network (a wire):

$$t_{pd} = \sum_{i} R_i \times C_{downstream,i}$$

For a uniform wire of length $L$, width $W$, sheet resistance $R_{sh}$, and capacitance per unit length $c$:

$$R_{total} = R_{sh} \times \frac{L}{W}$$
$$C_{total} = c \times L$$

The Elmore delay is:

$$t_{Elmore} = \frac{1}{2} R_{total} \times C_{total} = \frac{R_{sh} \times c \times L^2}{2W}$$

**Note the $L^2$ dependence!** Doubling the wire length quadruples the delay. This is why long wires (wordlines, bitlines) are the performance bottleneck in large SRAMs.

### Distributed RC Model (More Accurate)

A wire can be modeled as a chain of small segments, each with resistance $dR$ and capacitance $dC$:

```
    R/N    R/N    R/N    R/N    R/N
  ┤─┤──┬──┤──┬──┤──┬──┤──┬──┤──┬──
        │     │     │     │     │
       C/N   C/N   C/N   C/N   C/N
        │     │     │     │     │
      ─────────────────────────────
```

The 50% propagation delay for a distributed RC line:

$$t_{50\%} = 0.38 \times R \times C$$

The 90% delay:

$$t_{90\%} = 0.9 \times R \times C$$

### Repeated Buffer Insertion

For very long wires, buffer insertion reduces delay from $O(L^2)$ to $O(L)$:

$$t_{buffered} = N \times (t_{buffer} + t_{wire,segment})$$

Where $N$ = number of buffers, and each segment has length $L/N$.

The optimal number of buffers:

$$N_{opt} = L \sqrt{\frac{r_{wire} \times c_{wire}}{t_{buffer,intrinsic}}}$$

Where $r_{wire}$ and $c_{wire}$ are per-unit-length values.

---

# PART 4: IR DROP IN SRAM — COMPLETE ANALYSIS

## 4.1 What Is IR Drop?

IR drop is the voltage difference between the power supply pad and the transistor due to current flowing through the finite resistance of the power distribution network.

$$\Delta V = I \times R_{path}$$

Where:
- $I$ = current flowing through the path
- $R_{path}$ = total resistance from supply pad to transistor

**Why IR drop matters for SRAM:**

1. **Reduces SNM:** The effective VDD at the cell is lower → butterfy curves compress → SNM drops
2. **Increases delay:** Lower VDD means slower transistors → longer read/write time
3. **Creates spatial variation:** Cells in the center of the array see more IR drop than cells at the edges → performance/reliability variation across the array
4. **Can cause functional failure:** If IR drop is too large, cells can lose data

### The IR Drop Equation — Detailed

The power delivery network is a resistive mesh from the supply pad to each cell:

```
     VDD pad
       │
   R_package (bond wire/bump)
       │
   R_top_metal (M5 power bus)
       │
   R_via4
       │
   R_M4 (power stripe)
       │
   R_via3
       │
   R_M3 (if used)
       │
   R_via2
       │
   R_M2 (if used)
       │
   R_via (M1→M2)
       │
   R_M1 (cell VDD rail)
       │
   R_mcon (to LI)
       │
   R_LI (local connection)
       │
   R_licon (to diffusion)
       │
   Transistor Source
```

Total IR drop:

$$\Delta V_{DD} = I \times \sum_{k} R_k = I \times (R_{pkg} + R_{M5} + R_{via4} + R_{M4} + R_{via3} + R_{M3} + R_{via2} + R_{M2} + R_{via} + R_{M1} + R_{mcon} + R_{LI} + R_{licon})$$

## 4.2 Calculating IR Drop in SKY130 — Worked Example

### Scenario: 256×256 SRAM array, reading all columns simultaneously

**Current estimation:**
```
Per cell read current ≈ 50 μA (during read, current flows through access + driver)
Number of cells reading simultaneously = 256 (one row)
Total current from VDD = 256 × 50 μA = 12.8 mA

This current must flow through the power grid from pad to cells.
```

**Power grid topology:**
```
M5 runs horizontally (top-level VDD bus)
  Width = 5μm, Length = 256μm (across array)
  Via4 connections every 10 cells

M4 runs vertically (power straps)
  Width = 0.8μm, Length = 256μm
  One M4 strap every 8 columns = 32 straps

M1 runs horizontally (cell-level VDD rail)
  Width = 0.14μm, Length = cell pitch = 0.5μm per cell
```

**Step 1: M5 bus resistance**
```
R_M5 = R_sh(M5) × L/W = 0.029 Ω/sq × (256/5) = 0.029 × 51.2 = 1.48 Ω
```

But current distributes — the cell in the center gets half the total bus resistance:
```
R_M5,eff ≈ R_M5 / 3 = 0.49 Ω (for uniformly distributed load)
```

The factor of 3 comes from the distributed load analysis: for a uniform load along a bus, the effective resistance seen by the center is R/3, and the end sees R.

**Step 2: Via4 resistance (M4 to M5)**
```
Via4 resistance per via = 380 Ω
Using 4 vias in parallel per connection: R_via4 = 380/4 = 95 Ω
```

**Step 3: M4 strap resistance**
```
M4 strap length from M5 to center of array = 128μm (half)
M4 width = 0.8μm
R_M4 = 0.047 × (128/0.8) = 0.047 × 160 = 7.52 Ω
Effective (distributed): R_M4,eff = 7.52/3 = 2.51 Ω
```

**Step 4: VIA resistance (M1 to M4, through M2 and M3)**
```
VIA (M1→M2): 4500 Ω per via, use 2 in parallel → 2250 Ω
VIA2 (M2→M3): 3410 Ω per via, use 2 in parallel → 1705 Ω
VIA3 (M3→M4): 3410 Ω per via, use 2 in parallel → 1705 Ω
Total via stack: 2250 + 1705 + 1705 = 5660 Ω per connection point
```

**BUT** — there are many via stacks in parallel (one per strap intersection). With 32 M4 straps feeding the same M1 row rail, the effective via resistance seen by a cell in the middle is much lower:

```
R_via_stack,eff ≈ 5660 / 4 = 1415 Ω (cell sees ~4 nearby connections)
```

**Step 5: M1 cell rail resistance**
```
M1 rail in cell: length ≈ 0.5μm, width = 0.14μm
R_M1_cell = 0.125 × (0.5/0.14) = 0.45 Ω per cell
Worst case (cell between two via stacks, 4 cells away):
R_M1_path = 4 × 0.45 = 1.8 Ω
```

**Step 6: MCON + LI + LICON**
```
MCON: 152,000 Ω per contact! Use 2 in parallel → 76,000 Ω
Wait — is this right? 152 kΩ per MCON?
```

**IMPORTANT NOTE ON UNITS:** The SKY130 RCX page lists via/contact resistance values. Let me reconsider the units. Looking at the data again:

```
LICON contact:  15,000    ← These might be in mΩ/contact
MCON contact:   152,000   ← = 152 Ω/contact? Not 152 kΩ!
VIA:            4,500     ← = 4.5 Ω? or 4,500 Ω?
VIA2:           3,410
VIA3:           3,410
VIA4:           380
```

The sheet resistance values were given in mΩ/sq (e.g., M1 = 125 mΩ/sq = 0.125 Ω/sq), so these contact/via values are likely also in **mΩ per contact**:

```
CORRECTED:
LICON: 15,000 mΩ = 15 Ω per contact
MCON:  152,000 mΩ = 152 Ω per contact
VIA:   4,500 mΩ = 4.5 Ω per via
VIA2:  3,410 mΩ = 3.41 Ω per via
VIA3:  3,410 mΩ = 3.41 Ω per via
VIA4:  380 mΩ = 0.38 Ω per via
```

**This makes MUCH more sense for a real process.** Let's redo the calculation:

### Corrected IR Drop Calculation

```
Component          Per-element R    # Parallel    Effective R
────────────────────────────────────────────────────────────────
M5 bus (eff)       —                —             0.49 Ω
Via4               0.38 Ω           4             0.095 Ω
M4 strap (eff)     —                —             2.51 Ω
Via3               3.41 Ω           2             1.71 Ω
M3 (short)         ~0.5 Ω           —             0.5 Ω
Via2               3.41 Ω           2             1.71 Ω
M2 (short)         ~0.5 Ω           —             0.5 Ω
Via (M1→M2)        4.5 Ω            2             2.25 Ω
M1 cell rail       1.8 Ω            —             1.8 Ω
MCON               152 Ω            2             76 Ω
LI (short)         ~1 Ω             —             1.0 Ω
LICON              15 Ω             2             7.5 Ω
────────────────────────────────────────────────────────────────
TOTAL                                             ≈ 96 Ω
```

**The MCON dominates!** This is why the cell-level contacts (MCON, LICON) are the biggest resistance problem.

**Current per VDD connection point** (not total array current — current distributes):
```
Cells per VDD tap ≈ 2 (shared between cells)
I per tap = 2 × 50 μA = 100 μA

IR drop = 100 μA × 96 Ω = 9.6 mV (0.53% of VDD)

Actually, the total via stack resistance matters more in aggregate.
Let's compute the worst case for the cell in the center of the array:

I through M5 to center: 12.8 mA × (128/256) = 6.4 mA
IR_M5 = 6.4 mA × 0.49 Ω = 3.14 mV

I through one M4 strap (serves ~8 columns × 256 rows / 2):
I_M4 = (8/256) × 12.8 mA = 0.4 mA per strap
IR_M4 = 0.4 mA × 2.51 Ω = 1.0 mV

I through via stack (serves ~2 cells):
I_via = 100 μA
IR_via = 100 μA × (1.71 + 0.5 + 1.71 + 0.5 + 2.25) = 100 μA × 6.67 Ω = 0.67 mV

I through MCON (per cell):
I_cell = 50 μA
IR_MCON = 50 μA × 76 Ω = 3.8 mV

IR_LI = 50 μA × 1.0 Ω = 0.05 mV
IR_LICON = 50 μA × 7.5 Ω = 0.375 mV
IR_M1 = 50 μA × 1.8 Ω = 0.09 mV

────────────────────────────────────────────
TOTAL IR DROP ≈ 3.14 + 1.0 + 0.67 + 3.8 + 0.05 + 0.375 + 0.09
             ≈ 9.1 mV (0.51% of 1.8V VDD)
```

**9.1 mV of IR drop is acceptable** — it's well under the 5% of VDD (90 mV) budget. But this is for a 256×256 array with a well-designed power grid. For larger arrays or worse grids, IR drop gets much worse.

## 4.3 IR Drop Impact on SNM

SNM degrades approximately linearly with effective VDD reduction:

$$SNM_{effective} = SNM_{nominal} - k_{IR} \times \Delta V_{IR}$$

Where $k_{IR}$ ≈ 0.5–1.0 (empirical, depends on cell design).

For our example:
```
SNM_nominal (at 1.8V, CR=1.6) = 223 mV (from the paper)
IR drop = 9.1 mV
k_IR ≈ 0.8

SNM_effective = 223 - 0.8 × 9.1 = 223 - 7.3 = 215.7 mV
```

An 8 mV SNM degradation (3.3%) — acceptable.

But in a 1024×1024 array with poor power grid:
```
IR drop could be 50-100 mV
SNM_effective = 223 - 0.8 × 80 = 159 mV
```

A 29% SNM degradation — potentially dangerous at worst-case conditions.

## 4.4 IR Drop Design Rules from SKY130

The SKY130 PDK has specific design rules for IR drop:

```
Rule              Description
──────────────────────────────────────────────────────────────────
via.irdrop.1-4    Via connections to power must meet IR drop limits
via2.irdrop.1-4   Via2 connections to power must meet IR drop limits
via3.irdrop.1-4   Via3 connections to power must meet IR drop limits
via4.irdrop.1-4   Via4 connections to power must meet IR drop limits
```

These rules typically require:
- **Multiple vias** for all power connections (never a single via)
- **Minimum via density** in power connections
- **Maximum current per via** limits to prevent electromigration
- **Power strap width and spacing** requirements

### Electromigration (EM) — The Other Current Constraint

Even if IR drop is acceptable, too much current through a wire causes **electromigration** — the physical movement of metal atoms due to electron momentum transfer. This eventually creates voids (opens) or hillocks (shorts).

The maximum current density:

$$J_{max} = \frac{I_{max}}{W \times t}$$

Typical EM limits for copper at 105°C:
```
Layer       J_max (mA/μm of width)
──────────────────────────────────────
M1          1.0
M2          1.0
M3          2.0
M4          2.0
M5          3.0
Via         0.2 mA per via
```

For the M5 bus in our example:
```
I_total = 12.8 mA
M5 width needed ≥ 12.8/3.0 = 4.27 μm → Use 5μm ✓ (we chose 5μm)
```

---

# PART 5: SRAM ARRAY ARCHITECTURE (Weste Ch. 12.2)

## 5.1 Row Circuitry — Address Decoding

### Basic Decoder

An N-bit address selects one of $2^N$ wordlines. The simplest decoder uses one AND gate per wordline:

**NAND Decoder (most common):**
```
A0──┐
A1──┤ NAND ──── INV ──── WL0  (when A[1:0] = 00)
    │
A̅0──┐
A1──┤ NAND ──── INV ──── WL1  (when A[1:0] = 01)
    │
A0──┐
A̅1──┤ NAND ──── INV ──── WL2  (when A[1:0] = 10)
    │
A̅0──┐
A̅1──┤ NAND ──── INV ──── WL3  (when A[1:0] = 11)
```

**Problem:** For a 10-bit address (1024 rows), each NAND gate is 10 inputs wide. A 10-input NAND is extremely slow (stacked NMOS transistors).

### Predecoded Design (Hierarchical)

Split the address into groups and AND the partial decodes:

```
A[1:0] → 2-to-4 predecoder → 4 predecode lines (P0-P3)
A[3:2] → 2-to-4 predecoder → 4 predecode lines (P4-P7)
A[4]   → 1-to-2 predecoder → 2 predecode lines (P8-P9)

Each wordline = 3-input AND (one from each group):
WL0 = P0 · P4 · P8
WL1 = P1 · P4 · P8
...

Total: 3-input NANDs instead of 5-input NANDs!
```

**The general rule:** An N-bit address is split into groups of 2–3 bits. This minimizes:
- NAND gate input count (≤ 4 inputs)
- Total number of predecode wires
- Decoder delay

For N=10 bits, optimal grouping is {2, 2, 2, 2, 2} → 5 groups of 4 = 20 predecode lines, and each wordline uses a 5-input AND (still large).

Better: {3, 3, 4} → 8 + 8 + 16 = 32 predecode lines, 3-input ANDs.

### Dynamic Decoders

Static decoders waste power. Dynamic decoders use precharge-evaluate:

```
         VDD
          │
     ┌────┤ P_precharge (ON when CLK=0)
     │    │
     │    ├─── WL
     │    │
     └────┤ N_A0 (stacked NMOS, ON when all address bits match)
          │
     ┌────┤ N_A1
     │    │
     └────┤ N_A2
          │
         GND
```

**Phase 1 (Precharge, CLK=0):** WL precharged to VDD
**Phase 2 (Evaluate, CLK=1):** If all address bits match, WL stays high. If any mismatch, WL discharges to GND.

**Problem with dynamic decoders:** Since ALL wordlines precharge to VDD, ALL cells in the row are briefly disturbed during precharge. Solution: **Self-resetting domino decoder** — the precharge pulse is very brief, and a feedback keeper holds the selected wordline high.

### Sum-Addressed Decoder

For maximum speed, the **sum-addressed decoder** combines the row decoder with the address adder. Instead of: Address → Adder → Register → Decoder → WL, it does: Address bits → Combined predecoder/adder → WL, saving one pipeline stage.

## 5.2 Column Circuitry

### Bitline Precharge

Before a read, both BL and BL̄ are precharged to the same voltage (typically VDD or VDD/2):

```
         VDD
          │
     ┌────┤ P_eq (equalize)─────────────┐
     │    │                              │
     │    ├── BL    BL̄ ──┤              │
     │    │                │             │
     ├────┤ P_pre1        P_pre2 ├───────┘
     │    │                │
     │    ├── BL          BL̄
```

**Why precharge to VDD (not VDD/2)?**
- VDD precharge: BL and BL̄ start at VDD. During read, one drops. Half-swing read (saves power for the "1" bitline since it stays at VDD).
- VDD/2 precharge: Both bitlines swing +/- equally. Better common-mode rejection in sense amp. Used in some DRAMs.

Most SRAMs use **VDD precharge**.

### Sense Amplifiers — Large Signal vs Small Signal

**Large-signal sensing:** Wait for the bitline to swing to a full logic level. No sense amp needed, just a regular inverter/buffer.

```
    BL voltage during large-signal read:
    
    VDD ───────────────────────────
                                    ╲
                                     ╲ Slow discharge
                                      ╲ through cell
                                       ╲
    VDD/2 ─────────────────────────     ╲────── Finally crosses
                                               inverter trip point
    Time: "Slow" — 5-10 ns at 130nm
```

**Delay:** $t_{read} = \frac{C_{BL} \times \Delta V}{I_{cell}}$

With $C_{BL}$ = 106 fF, $\Delta V$ = 0.9V (half swing), $I_{cell}$ = 50 μA:
$$t_{read} = \frac{106 \times 10^{-15} \times 0.9}{50 \times 10^{-6}} = 1.91 \text{ ns}$$

**Small-signal sensing (differential sense amplifier):** Only need ΔV ≈ 50–100 mV between BL and BL̄. Much faster!

$$t_{read} = \frac{C_{BL} \times \Delta V_{sense}}{I_{cell}}$$

With $\Delta V_{sense}$ = 50 mV:
$$t_{read} = \frac{106 \times 10^{-15} \times 0.05}{50 \times 10^{-6}} = 0.106 \text{ ns}$$

**18× faster!** But requires a precise, offset-calibrated sense amplifier.

### The Cross-Coupled Sense Amplifier

```
         VDD
          │
     ┌────┤ P3─ SAE_n
     │    │
     ├────┤ P1          P2 ├────┐
     │    │                 │    │
     │    ├─── out    out̄ ──┤    │
     │    │                 │    │
     └────┤ N1          N2 ├────┘
          │                 │
          ├────────┬────────┤
          │        │        │
          │   N4 ──┤── SAE  │
          │        │        │
                  GND
```

**Operation:**
1. **Precharge:** SAE=0, both outputs equalized
2. **Develop:** Small ΔV develops between BL/BL̄ (≈50-100mV)
3. **Sense:** SAE goes high → positive feedback amplifies ΔV to full swing

**Sense amp offset** is the minimum ΔV it can reliably detect. Caused by threshold voltage mismatch in N1/N2 and P1/P2:

$$V_{offset} = \frac{\Delta V_{th}}{\sqrt{2}} \approx \frac{\sigma_{Vth}}{\sqrt{2}}$$

For SKY130 with $\sigma_{Vth}$ ≈ 10 mV: $V_{offset}$ ≈ 7 mV.

The minimum detectable signal must be > offset:
$$\Delta V_{BL,min} > V_{offset} + V_{margin} \approx 7 + 30 = 37 \text{ mV}$$

### Replica Bitline Timing

**Problem:** When do you fire the sense amplifier enable (SAE)? Too early → insufficient signal → wrong data. Too late → wasted time.

**Solution:** Use a **replica bitline** — a dummy column with a known cell state that generates a timing signal when the bitline has developed enough:

```
    Real BL         Replica BL
    │               │
    ├── Cell 0      ├── Dummy cell (always "1")
    ├── Cell 1      ├── (no other cells)
    ├── Cell 2      │
    ├── ...         │
    │               │
    └── SA          └── Comparator → SAE trigger
```

The replica BL has the same capacitance as a real BL (same length, same wire). The dummy cell discharges it. When the replica BL drops below a reference voltage (VDD - ΔV_target), the SAE fires.

### Column Multiplexing (Muxing)

Not all columns need separate sense amps. If you only read one byte at a time from a 256-column array:

```
Columns:  0   1   2   3   4   5   6   7   ...  255
          │   │   │   │   │   │   │   │        │
          └───┼───┼───┘   └───┼───┼───┘        │
              │   │           │   │             │
          4:1 MUX         4:1 MUX          4:1 MUX
              │               │                │
             SA0             SA1            SA63
```

**4:1 mux:** 256 columns / 4 = 64 sense amps needed instead of 256.

**Tradeoff:** Multiplexing adds MUX transistor resistance/capacitance to the bitline path, slightly increasing read delay. But it saves massive area (sense amps are large).

Common mux ratios: 4:1, 8:1, 16:1. Beyond 16:1, the added capacitance usually negates the area savings.

## 5.3 Multiported SRAM and Register Files

### 8T SRAM Cell — Separate Read Port

The 6T cell's fundamental problem is the read disturbance. The 8T cell solves this by adding a separate read port:

```
        Standard 6T section              Read port
      VDD            VDD                 VDD
       │              │                   │
  ┌────┤P1        P2──┤────┐          ┌───┤ P_rd
  │    │              │    │          │   │
  Q────┤              ├────Q̄    Q ───┤   ├─── RBL (Read Bitline)
  │    │              │    │          │   │
  └────┤N1        N2──┤────┘          └───┤ N_rd_top
       │              │                   │
      GND            GND             ┌───┤ N_rd_bot ──── RWL (Read Wordline)
                                     │   │
       WL─────N5──────N6────WL      GND
              │        │
             BL       BL̄
       (Write only)
```

**How it works:**
- **Write:** Same as 6T — use WL, BL, BL̄
- **Read:** Activate RWL. If Q=1, both N_rd_top and N_rd_bot turn on → RBL discharges. If Q=0, N_rd_top is off → RBL stays high.
- **No read disturbance!** The storage nodes Q and Q̄ are isolated from the read port.

**Cost:** 33% more transistors, ~30% more area per cell. But eliminates the read-stability problem entirely.

### Multi-Read, Multi-Write Register Files

For processor register files (32 registers, 2 read + 1 write ports), each port needs its own access transistors and bitlines. A 2R1W register file cell has:

$$\text{Transistors} = 6 (\text{core}) + 2 \times 2 (\text{read ports}) + 1 \times 2 (\text{write port}) = 12\text{T}$$

For a 6-read, 3-write ARM register file:

$$\text{Transistors} = 6 + 6 \times 2 + 3 \times 2 = 24\text{T per cell}$$

The area explosion is why large register files use **banking** or **time-division multiplexing** instead of true multi-porting.

## 5.4 Large SRAM Organization — Banks, Subarrays, Hierarchy

### Why We Can't Just Make One Big Array

A 1 MB SRAM at 130nm has:
```
1 MB = 8,388,608 bits
If arranged as one array: √(8M) ≈ 2896 rows × 2896 columns
Wordline length = 2896 × 0.5μm = 1,448 μm = 1.45 mm
Bitline length = 2896 × 1.0μm = 2,896 μm = 2.9 mm
```

Wordline RC delay:
$$t_{WL} = 0.38 \times R_{WL} \times C_{WL}$$
$$R_{WL} = 0.125 \times (1448/0.14) = 1,293 \text{ Ω}$$
$$C_{WL} = 2896 \times 0.327 + 1448 \times 0.2 = 947 + 290 = 1,237 \text{ fF}$$
$$t_{WL} = 0.38 \times 1293 \times 1237 \times 10^{-15} = 0.608 \text{ ns}$$

Bitline development:
$$\Delta V_{BL} = \frac{C_{cell}}{C_{BL}} = \frac{1.25}{2896 \times 0.4 + 2896 \times 0.1} = \frac{1.25}{1448} = 0.00086 = 0.86 \text{ mV}$$

**0.86 mV signal — far below any sense amp's capability.** The sense amp offset alone is 7 mV.

### Solution: Divide into Subarrays

```
Typical organization: Bank → Subarray → Mat

1 MB SRAM:
  4 Banks × 256 KB each
  Each Bank: 8 subarrays × 32 KB each
  Each Subarray: 256 rows × 128 columns × 8 (byte) = 32 KB

  Subarray BL length: 256 cells → C_BL ≈ 100 fF
  ΔV_BL = 1.25/100 = 12.5 mV → Detectable ✓
```

### Hierarchical Bitlines

For even better signal, use **hierarchical bitlines**:

```
Global Bitline (GBL) — runs full array height on M3 or M4
    │
    ├── Subarray 0 ── Local BL (LBL) — 32-64 cells on M2
    │
    ├── Subarray 1 ── Local BL
    │
    ├── Subarray 2 ── Local BL
    │
    └── Subarray 3 ── Local BL
```

Only one subarray is active at a time. The local BL is short (32–64 cells), giving excellent signal:
$$\Delta V_{LBL} = \frac{1.25}{32 \times 0.4 + 64 \times 0.1} = \frac{1.25}{19.2} = 65 \text{ mV}$$

This is amplified by a local sense amp and driven onto the global bitline.

## 5.5 Low-Power SRAM Techniques

### Minimum Operating Voltage ($V_{min}$)

$V_{min}$ is the lowest VDD at which the SRAM still functions correctly. It's set by:

$$V_{min} = \max(V_{min,read}, V_{min,write}, V_{min,hold})$$

Where:
- $V_{min,read}$: Cell must have SNM > 0 plus margin for offset
- $V_{min,write}$: Must be able to flip the cell
- $V_{min,hold}$: DRV — cell must retain data

With process variation (σ_Vth = 30 mV at 130nm):
$$V_{min,6\sigma} \approx DRV + 6\sigma_{Vth} = 0.45 + 0.18 = 0.63\text{V for SKY130}$$

### Read Assist Techniques

Help the cell survive reads without flipping:

1. **Wordline voltage reduction:** Drive WL to 0.8×VDD instead of VDD → weaker access transistor → less read disturbance. Cost: slower read.

2. **Negative bitline:** Pre-discharge BL to slightly below GND (−50mV) → increases the "0" storage node's holding strength. Complex circuit.

3. **Cell VDD boosting:** Momentarily boost the cell's VDD during read → more SNM. Requires charge pump.

### Write Assist Techniques

Help overpower the cell during writes:

1. **Negative WL:** When NOT writing, drive WL to −100mV → cut off access transistor more aggressively → better write margin for the row being written.

2. **Cell VDD collapse:** Lower the cell's VDD during write → weakens pull-up → easier to flip. Simple to implement with a header switch.

3. **Boosted WL:** Drive WL above VDD → stronger access transistor → more write current. Requires charge pump.

### Leakage Control

In standby, cells must retain data with minimum leakage:

1. **Supply collapse:** Reduce VDD to just above DRV during sleep. Leakage ∝ VDD² in active mode, but subthreshold leakage is exponential:

$$I_{leak} = I_0 \times e^{(V_{GS} - V_{th}) / (n \cdot V_T)}$$

Reducing VDD reduces $V_{GS}$ → exponential leakage reduction.

2. **Negative wordline bias:** Hold WL at −100mV in standby → more negative $V_{GS}$ for access transistors → exponential leakage reduction.

3. **Body bias:** Apply reverse body bias → increases $V_{th}$ → reduces leakage. Forward body bias when active for speed.

---

# PART 6: LOGICAL EFFORT AND DELAY MODELING (Weste Ch. 4.4–4.5)

## 6.1 The Linear Delay Model

The delay of a logic gate can be expressed as:

$$d = f + p$$

Where:
- $d$ = total delay (in units of $\tau$ = FO4 inverter delay / ~5)
- $f$ = effort delay (also called "stage effort") — depends on load
- $p$ = parasitic delay — intrinsic delay of the gate, independent of load

The effort delay is:

$$f = g \times h$$

Where:
- $g$ = **logical effort** — inherent slowness of the gate type relative to an inverter
- $h$ = **electrical effort** (fanout) = $C_{out} / C_{in}$

So the total delay of a single gate is:

$$d = g \times h + p = g \times \frac{C_{out}}{C_{in}} + p$$

## 6.2 Logical Effort Values — Complete Table

**Logical effort $g$** measures how much worse a gate is than an inverter at delivering current, for the same input capacitance:

```
Gate Type        Logical Effort (g)    Parasitic Delay (p)    Notes
──────────────────────────────────────────────────────────────────────────────
Inverter         1                     1                      Reference gate
NAND2            4/3 ≈ 1.33            2                      2-input NAND
NAND3            5/3 ≈ 1.67            3                      3-input NAND
NAND4            6/3 = 2.00            4                      4-input NAND
NOR2             5/3 ≈ 1.67            2                      2-input NOR
NOR3             7/3 ≈ 2.33            3                      3-input NOR
NOR4             9/3 = 3.00            4                      4-input NOR
XOR2 (CMOS)      4                     4                      Complex gate
XNOR2            4                     4                      Complex gate
2:1 MUX          2                     4                      Transmission gate mux
Tristate buffer  2                     2                      With enable
```

### Why Does NAND2 Have g = 4/3?

Consider an inverter with NMOS width $W_n$ and PMOS width $W_p = 2W_n$ (for equal rise/fall). Total input capacitance = $C_{in} = 3W_n$ (in units of minimum transistor cap).

Now consider a NAND2. To match the inverter's pull-down current, each NMOS must be $2W_n$ (because they're stacked in series — two in series has half the drive). The PMOS transistors are $2W_n$ each (parallel, same as inverter pull-up). Total input capacitance per input = $2W_n + 2W_n = 4W_n$.

But the output drive is the same as the inverter (by design). So:

$$g_{NAND2} = \frac{C_{in,NAND2}}{C_{in,INV}} \times \frac{\text{INV drive}}{\text{NAND2 drive}} = \frac{4W_n}{3W_n} = \frac{4}{3}$$

The gate is 4/3 times slower than an inverter because it has 4/3 times more input capacitance for the same output drive.

### Why Does NOR2 Have g = 5/3?

For a NOR2: NMOS transistors are parallel (each $W_n$). PMOS transistors are stacked in series — each must be $2 \times 2W_n = 4W_n$ to match the inverter's pull-up (series doubles, plus mobility ratio). But actually:

Without mobility compensation: stacked PMOS each need $2 \times W_p = 4W_n$. Total per input = $W_n + 4W_n = 5W_n$.

$$g_{NOR2} = \frac{5W_n}{3W_n} = \frac{5}{3}$$

**This is why NAND gates are preferred over NOR gates** — NAND has lower logical effort (4/3 vs 5/3) because stacking NMOS (higher mobility) is less painful than stacking PMOS (lower mobility).

## 6.3 Multi-Stage Path Delay

For a path of $N$ stages:

$$D = \sum_{i=1}^{N} (g_i \times h_i + p_i) = \sum_{i=1}^{N} f_i + \sum_{i=1}^{N} p_i$$

Define the **path effort:**

$$F = G \times B \times H$$

Where:
- $G$ = **path logical effort** = $\prod_{i=1}^{N} g_i$ (product of all logical efforts)
- $B$ = **branching effort** = $\prod_{i=1}^{N} b_i$ where $b_i = \frac{C_{on-path} + C_{off-path}}{C_{on-path}}$
- $H$ = **path electrical effort** = $\frac{C_{out,final}}{C_{in,first}}$ (total fanout from first input to last output)

### Minimum Delay — The Optimal Stage Effort

The minimum delay occurs when each stage has the same effort:

$$\hat{f} = f_1 = f_2 = ... = f_N = F^{1/N}$$

The total minimum delay:

$$D_{min} = N \times F^{1/N} + P$$

Where $P = \sum p_i$.

### The Magic Number: ρ ≈ 3.59

The optimal stage effort (effort per stage) that minimizes delay is approximately:

$$\hat{f}_{opt} \approx 3.59$$

This comes from solving:
$$\frac{dD}{dN} = 0 \text{ where } D = N \times F^{1/N} + N \times p_{avg}$$

With $p_{avg} = 1$ (inverter parasitic), the optimal per-stage effort is:

$$\hat{f}_{opt} = e^{1 + p/g} \approx e^{1+1} \approx e \times e^1 \approx 3.59$$

(The exact value is the solution to $\hat{f} \times \ln(\hat{f}) = \hat{f} + p/g$ where $p=g=1$.)

**Practical rule:** Each stage should drive about **3.6× its input capacitance**. If you need to drive more, add more stages. If less, combine stages.

### Best Number of Stages

$$N_{opt} = \log_\rho(F) = \frac{\ln(F)}{\ln(3.59)} = \frac{\ln(F)}{1.28}$$

Round to the nearest integer. If $N_{opt}$ is even/odd, choose even for non-inverting, odd for inverting.

### Worked Example: SRAM Wordline Driver

**Problem:** Drive a wordline with $C_{WL}$ = 500 fF from a predecoder output with $C_{in}$ = 2 fF. The predecoder output is a NAND3.

```
Path: NAND3 → Buffer chain → Wordline

H = C_WL / C_in = 500/2 = 250
G = g_NAND3 × (g_INV)^(N-1) = (5/3) × 1^(N-1) = 5/3  (for inverter chain)
B = 1 (no branching — one wordline per decoder output)

F = G × B × H = (5/3) × 1 × 250 = 417

N_opt = ln(417) / ln(3.59) = 6.03 / 1.28 = 4.7 → Use N = 5 stages

Optimal stage effort: f̂ = 417^(1/5) = 3.38 (close to 3.59 ✓)
```

**Sizing each stage:**
```
Stage 5 (drives WL): C_out = 500 fF → C_in,5 = 500/3.38 = 148 fF
Stage 4:             C_out = 148 fF → C_in,4 = 148/3.38 = 43.8 fF
Stage 3:             C_out = 43.8 fF → C_in,3 = 43.8/3.38 = 13.0 fF
Stage 2:             C_out = 13.0 fF → C_in,2 = 13.0/3.38 = 3.84 fF
Stage 1 (NAND3):     C_out = 3.84 fF → C_in,1 = 3.84/3.38 = 1.14 fF

Check: C_in,1 ≈ 1.14 fF vs our budget of 2 fF → Good, use 2 fF (faster)
```

**Total delay:**
```
D = N × f̂ + P
P = p_NAND3 + 4 × p_INV = 3 + 4(1) = 7
D = 5 × 3.38 + 7 = 16.9 + 7 = 23.9 τ

Convert to time: τ ≈ 15 ps at 130nm (FO4 ≈ 75 ps, τ = FO4/5)
D = 23.9 × 15 ps = 359 ps ≈ 0.36 ns
```

## 6.4 FO4 Delay — The Universal Metric

**FO4** = Fanout-of-4 inverter delay. It's the delay of an inverter driving 4 copies of itself.

$$FO4 = g_{INV} \times 4 + p_{INV} = 1 \times 4 + 1 = 5\tau$$

**Why FO4 is universal:** It scales with technology. All other delays can be expressed in FO4 units:

```
Technology   FO4 (ps)    τ (ps)     Pipeline stage budget
──────────────────────────────────────────────────────────
180nm        100         20         15-20 FO4
130nm        75          15         15-20 FO4
65nm         35          7          15-20 FO4
45nm         25          5          15-20 FO4
28nm         15          3          12-15 FO4
7nm          7           1.4        8-12 FO4
```

**Rule of thumb from Jim Keller and others:** A pipeline stage should be about **15-20 FO4 delays** for optimal performance. Fewer FO4 → too many pipeline stages (overhead). More FO4 → clock frequency limited.

## 6.5 The CACTI Model — SRAM Delay Estimation

CACTI (from HP Labs) models cache delay as:

$$D_{cache} = (1.5 + \sqrt{C_{bits}}) \times FO4$$

Where $C_{bits}$ is the total number of bits in the cache (not bytes!).

**Wait — this is a simplified version.** The more detailed CACTI model breaks down the delay into:

$$D_{total} = D_{decoder} + D_{wordline} + D_{bitlines} + D_{senseamp} + D_{output}$$

Each component is modeled using logical effort and wire delay.

**Simplified approximation for small SRAMs:**

$$D_{SRAM} = 4^{\left(\log_4\frac{n+2}{3}\right)} \quad \text{(in FO4 units, for n words)}$$

For common sizes:
```
SRAM Size    Bits        CACTI Delay (FO4)    At 130nm (ns)
──────────────────────────────────────────────────────────────
1 KB         8,192       ~93 FO4              ~7.0
4 KB         32,768      ~182 FO4             ~13.6
16 KB        131,072     ~363 FO4             ~27.2
64 KB        524,288     ~725 FO4             ~54.4
256 KB       2,097,152   ~1449 FO4            ~108.7
```

**Wait — these values seem high. Let's use the simplified formula correctly:**

$$D = 1.5 + \sqrt{C}$$

```
8,192 bits:  D = 1.5 + √8192 = 1.5 + 90.5 = 92 FO4
32,768 bits: D = 1.5 + √32768 = 1.5 + 181 = 183 FO4
```

These are total access times including all components. Actual caches use pipelining — the access is broken into 2–4 pipeline stages, each < 20 FO4.

**A more practical estimate for the critical path (one pipeline stage):**

$$D_{critical} \approx \frac{D_{total}}{N_{pipe\_stages}}$$

For a 32 KB L1 cache with 3-stage pipeline at 130nm:
$$D_{critical} = \frac{183 \times 75\text{ps}}{3} = \frac{13.7\text{ns}}{3} = 4.6\text{ns} \quad \Rightarrow f_{max} \approx 220\text{MHz}$$

## 6.6 Velocity Saturation Effects on Logical Effort

At advanced nodes (below 130nm), transistors operate in **velocity saturation** rather than the square-law region. This changes the delay model:

**Square-law (long channel):** $I_{DS} \propto (V_{GS} - V_{th})^2$
**Velocity saturation (short channel):** $I_{DS} \propto (V_{GS} - V_{th})$

**Impact on logical effort:**

In velocity saturation, current is proportional to width (W), not W/L. This means:
- Stacked transistors (NAND/NOR) have the same drive as a single transistor with twice the width
- The logical effort of gates **decreases** toward the ideal:

```
Gate        g (long channel)    g (velocity sat.)    Change
────────────────────────────────────────────────────────────
Inverter    1                   1                    Same
NAND2       4/3 = 1.33          ~1.17                -12%
NAND3       5/3 = 1.67          ~1.25                -25%
NOR2        5/3 = 1.67          ~1.33                -20%
NOR3        7/3 = 2.33          ~1.50                -36%
```

**This is good news for circuit designers at advanced nodes** — complex gates become less of a speed penalty compared to inverters.

### Parasitic Delay in Velocity Saturation

Parasitic delay also changes because the transistor drive curve changes:

$$p_{vel.sat} \approx p_{long.channel} \times \frac{V_{DD}}{2(V_{DD} - V_{th})}$$

At 130nm (VDD=1.8V, Vth=0.45V):
$$p_{factor} = \frac{1.8}{2(1.8-0.45)} = \frac{1.8}{2.7} = 0.67$$

So parasitic delays are ~33% lower in velocity saturation.

---

# PART 7: PUTTING IT ALL TOGETHER — SRAM DESIGN FLOW

## 7.1 Design Checklist for a 130nm SRAM (SKY130)

```
Step    Task                        Key Equation / Reference
──────────────────────────────────────────────────────────────────────────────
1       Choose cell ratios          CR ≥ 1.5, PR ≤ 1.2 (from SNM analysis)
2       Simulate SNM                Butterfly diagram → target SNM > 200 mV
3       Calculate bitline cap       C_BL = C_area + C_fringe + C_coupling
4       Size sense amplifiers       ΔV_BL > V_offset + margin
5       Design decoder              Logical effort: N = ln(F)/ln(3.59)
6       Design WL driver            Size for RC delay target
7       Verify IR drop              ΔV < 5% VDD (90 mV at 1.8V)
8       Check electromigration      J < J_max for all wires
9       Verify hold/write margins   Simulate at worst-case corners
10      Check DRV                   Retention voltage > sleep VDD
11      Layout physical design      DRC/LVS clean to SKY130 rules
12      PEX extraction              Extract R/C parasitics with RCX rules
```

## 7.2 Key Equations Summary Table

```
Equation                                        Purpose                     Section
──────────────────────────────────────────────────────────────────────────────────────
SNM ≈ VDD(βR-1)/(βR+1) - Vth                   Read noise margin           1.1
CR = (W1/L1)/(W5/L5)                            Cell ratio                  1.2
PR = (W4/L4)/(W6/L6)                            Pull-up ratio               1.2
V_read = VDD/(1+CR)                              Read disturbance voltage    1.2
DRV ≈ 2·Vth_sub·ln(2)·n                         Data retention voltage      1.2
R_wire = R_sh × L/W                              Wire resistance             2.3
C_pp = ε₀εr/t_ILD × Area                        Parallel plate cap          2.4
C_gate = C_ox × W × L                           Gate capacitance            3.1
C_ox = ε_ox / t_ox                               Oxide cap per area          3.1
C_diff = Cj0·AD + Cjsw·PD                       Junction capacitance        3.1
ΔV_BL = C_cell/C_BL × VDD                       Bitline signal              3.2
t_Elmore = 0.5 × R × C                          Wire delay                  3.3
ΔV_IR = I × ΣR_k                                IR drop                     4.1
SNM_eff = SNM - k_IR × ΔV_IR                    IR-degraded SNM             4.3
d = g×h + p                                      Gate delay                  6.1
f̂_opt ≈ 3.59                                    Optimal stage effort        6.3
N_opt = ln(F)/ln(ρ)                              Optimal stage count         6.3
FO4 = 5τ                                         Universal delay metric      6.4
D_cache = (1.5 + √C) × FO4                      CACTI model                 6.5
```

## 7.3 Cross-Reference: Paper (180nm) → SKY130 (130nm) → Modern (7nm)

```
Parameter           180nm (Paper)   130nm (SKY130)    7nm (Modern)
──────────────────────────────────────────────────────────────────────
VDD                 1.8V            1.8V              0.75V
Vth (NMOS)          ~0.45V          ~0.45V            ~0.25V
SNM (CR=1.6)        223 mV          ~200 mV           ~80 mV
DRV                 0.6V            0.45V             0.20V
Min gate length     0.18μm          0.15μm            0.007μm
Metal layers        4-5             5                 12-15
FO4 delay           ~100 ps         ~75 ps            ~7 ps
C_BL (256 rows)     ~150 fF         ~100 fF           ~30 fF
C_cell              ~2 fF           ~1.25 fF          ~0.15 fF
ΔV_BL               ~24 mV          ~22 mV            ~9 mV
Typical cell area   ~5 μm²          ~3 μm²            ~0.03 μm²
```

## 7.4 Interview Answer Framework

When asked about SRAM in an interview, structure your answer as:

**Level 1 — Circuit (Always start here):**
- 6T cell topology, CR/PR ratios, SNM via butterfly diagram
- Read/write conflict, voltage divider equations

**Level 2 — Parasitics (Show depth):**
- Capacitance: C_gate, C_diff, C_wire (area + fringe + coupling)
- Resistance: Sheet R, via R, wordline/bitline RC delay
- IR drop: Power grid R×I, impact on SNM

**Level 3 — Architecture (Show systems thinking):**
- Subarrays, hierarchical bitlines, column muxing
- Sense amp design, replica bitline timing
- Decoder design using logical effort

**Level 4 — Low-power/Reliability (Show maturity):**
- Read/write assist circuits
- Vmin, DRV, leakage control
- Process variation (σ_Vth), 6σ design methodology

**Level 5 — Physical Design (Show PDK knowledge):**
- Metal stack constraints (SKY130: M1 0.14μm, M5 1.6μm)
- RCX extraction, parasitic-aware simulations
- DRC/LVS verification flow

---

## Appendix A: Unit Conversions

```
1 fF = 10⁻¹⁵ F = 1000 aF
1 aF = 10⁻¹⁸ F = 0.001 fF
1 pF = 10⁻¹² F = 1000 fF
1 μm = 10⁻⁶ m = 1000 nm
1 nm = 10⁻⁹ m = 10 Å
1 mΩ/sq = 10⁻³ Ω/sq
1 kΩ = 10³ Ω
mA/μm = milliamps per micrometer of gate width
```

## Appendix B: Physical Constants

```
ε₀ = 8.854 × 10⁻¹² F/m     (permittivity of free space)
εr(SiO₂) = 3.9               (relative permittivity of gate oxide)
εr(Si₃N₄) = 7.5              (silicon nitride)
εr(low-k) = 2.5-3.0          (low-k dielectric, advanced nodes)
kT/q = 26 mV at 300K         (thermal voltage)
q = 1.602 × 10⁻¹⁹ C         (electron charge)
k = 1.381 × 10⁻²³ J/K       (Boltzmann's constant)
μn(Si) ≈ 400 cm²/V·s         (electron mobility at 130nm)
μp(Si) ≈ 150 cm²/V·s         (hole mobility at 130nm)
```

## Appendix C: SKY130 Quick Reference Card

```
PROCESS: 130nm bulk CMOS, 5-metal, 1.8V nominal

TRANSISTORS:
  Min L = 0.15μm, Min W = 0.42μm (standard), 0.15μm (I/O)
  t_ox ≈ 5.7nm, C_ox ≈ 6.05 fF/μm²

METAL STACK (width / thickness / R_sh):
  LI:  0.17μm / 0.10μm / 12.8 Ω/sq
  M1:  0.14μm / 0.35μm / 0.125 Ω/sq
  M2:  0.14μm / 0.35μm / 0.125 Ω/sq
  M3:  0.30μm / 0.80μm / 0.047 Ω/sq
  M4:  0.30μm / 0.80μm / 0.047 Ω/sq
  M5:  1.60μm / 1.20μm / 0.029 Ω/sq

CONTACTS/VIAS (size / resistance):
  LICON: 0.17μm / 15 Ω
  MCON:  0.17μm / 152 Ω
  VIA:   0.15μm / 4.5 Ω
  VIA2:  0.20μm / 3.41 Ω
  VIA3:  0.20μm / 3.41 Ω
  VIA4:  0.80μm / 0.38 Ω

KEY CAPACITANCES (parallel plate, aF/μm²):
  M1-M2: 133.9    M2-M3: 86.2     M3-M4: 84.0
  M1-sub: ~250    Poly-sub: 94.2   LI-M1: 114.2
```

---

*Document compiled from: Mukherjee et al. (2010) IJCSI, SkyWater SKY130 PDK v0.0.0, Weste & Harris CMOS VLSI Design 4th Ed. Chapters 4.4-4.5 and 12.2*

#### **Bank FSM Questions:**
1. "Draw the bank state machine from memory. What are the 7 states?"
2. "Why can't you go directly from IDLE → READING? What's the intermediate state?"
3. "What's tRCD and why does it exist? (Hint: physics of charge transfer)"
4. "Explain tFAW in one sentence. Why is it 4 activations, not 3 or 5?"
5. "Why is tRRD different for same bank group vs different bank groups?"

#### **Refresh Questions:**
6. "Why does tREFI become 3.9μs at 95°C instead of 7.8μs? (DRAM physics)"
7. "Explain the RAIDR 10% timing window constraint. Why not 50%?"
8. "Why did weak rows at 7k retention fail but 16k succeed?"
9. "What's the difference between tRFC (all-bank) and tRFCpb (per-bank)?"
10. "Why does refresh consume 3.12% of time? Show the math."

#### **Power Model Questions:**
11. "Why did write-back save 68.72% energy? Explain the DRAM activate/precharge cost."
12. "What's IDD0 vs IDD2N vs IDD3N? Which is highest power?"
13. "Why does temperature affect power consumption?"
14. "Explain thermal feedback loop: power → temp → timing → ?"
15. "Why is 90% safety margin needed for retention? Why not 100%?"

**Rule:** Answer in **your own words**, no code. If stuck after 2 min, mark it "WEAK" and move on.

---

### **Hour 3-4 (8:00-10:00 PM): Code Deep Dive with Questions**

Now **review your actual code** while answering:

#### **Review `dram_bank_fsm.cpp`:**
- Find the tFAW enforcement logic. Trace through an example.
- Find tRRD validation. Why do you track last activation per bank group?
- **Question:** "Could you implement this differently? What are tradeoffs?"

#### **Review `refresh_controller.cpp`:**
- Trace through one RAIDR postponement decision (cycle 11,296)
- Find the `can_safely_postpone()` logic. Why does it check min_retention?
- **Question:** "What happens if you remove the 10% window check? Would it break?"

#### **Review `power_model.cpp`:**
- Find where you calculate energy per command. Why multiply by cycles?
- Trace through thermal feedback. How does temperature affect timing?
- **Question:** "How would you add voltage scaling to the power model?"

**Goal:** Not just "what does this do" but "WHY did I design it this way?"

---

### **Hour 5-6 (10:00 PM-12:00 AM): Question Bank Practice**

#### **Architecture Questions:**

**Q1:** "You have a workload with 80% row buffer hits. Should you use open-page or close-page policy? Why?"

**Q2:** "A bank is in ACTIVE state with row 0x42 open. A request arrives for row 0x100 in the same bank. Walk me through the state transitions and timing."

**Q3:** "Why does DDR4 have bank groups? What problem do they solve?"

**Q4:** "Your DRAM is consuming 500mW but you need <300mW. List 5 knobs you can turn and their power impact."

**Q5:** "Explain why tRAS_min exists. What breaks if you precharge too early?"

**Q6:** "You're at 95°C and refresh overhead is 6%. How do you reduce it without losing data?"

**Q7:** "Walk me through a write-back cache eviction from L1 → DRAM. What DRAM commands are issued?"

**Q8:** "Why does RAIDR only save 2.64% in your test but the paper claims 74%? Where's the gap?"

**Q9:** "Explain the command bus constraint. Why can't you issue activate to bank 0 and read from bank 1 simultaneously?"

**Q10:** "You measure average DRAM latency of 100 cycles but tRCD+tCAS = 38 cycles. Where are the other 62 cycles?"

**Rule:** Write full paragraph answers like you're explaining to an interviewer.

---

### **Hour 7 (12:00-1:00 AM): Synthesis & Tomorrow's Plan**

#### **Create Study Artifacts:**

**File: `dram/ARCHITECTURAL_INSIGHTS.md`**
```markdown
# Deep Understanding from Day 1-2 Implementation

## Bank FSM Core Insights
- tRCD exists because... [physical reason]
- tFAW prevents... [what failure mode]
- Bank groups solve... [which bottleneck]

## Refresh Architecture
- RAIDR timing window constraint exists because... [why]
- Weak vs strong row distribution in real DRAM is... [realistic %]
- Temperature scaling factor (2x @ 95°C) is driven by... [physics]

## Power Modeling
- Write-back saves energy by... [mechanism]
- Thermal feedback loop works via... [cycle]
- Dominant power component is... [which IDD state]

## Interview Stories I Can Tell
1. "I implemented RAIDR and discovered..."
2. "Write-back vs write-through taught me..."
3. "Debugging tFAW violations revealed..."
```

**File: `dram/WEAK_AREAS.md`**
```markdown
# Topics I Need to Review Again
- [ ] tWTR timing (why write-to-read has delay)
- [ ] Bank group address mapping
- [ ] Exact power calculation for refresh
- [ ] etc.
```

#### **Plan Tomorrow:**
- If you answered 12+/15 questions confidently → **Start Day 3** tomorrow
- If you answered 8-11/15 → **Another review session**, then Day 3
- If you answered <8/15 → **Problem: moving too fast, need deeper study**

---

## 📊 The Architect Test

**After tonight's review, you should be able to:**

✅ Draw bank state machine on whiteboard from memory
✅ Explain RAIDR timing window constraint without notes
✅ Calculate refresh overhead for any temperature
✅ Explain write-back energy savings to a 5-year-old
✅ Debug a tFAW violation given a command trace
✅ Answer "why does tRCD exist?" with DRAM physics
✅ Describe thermal feedback loop with diagram
✅ Compare your power model to DRAMPower (why 0.00% error?)

**If you can do 6+/8 above:** You're ready for Day 3 tomorrow
**If you can do 4-5/8:** Another review session needed
**If you can do <4/8:** Day 1-2 wasn't deep enough, revisit


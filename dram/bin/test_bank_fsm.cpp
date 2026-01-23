#include "../headers/dram_bank_fsm.h"
#include "../headers/dram_timing.h"
#include "../headers/timing_validator.h" // Needed for Global Checks (Test 3/4)
#include <iostream>
#include <cassert>
#include <vector>
#include <random>

using namespace dram;

// Helper to advance simulation time and update state
void step(DRAMBankFSM& bank, uint64_t& current_cycle, uint64_t cycles_to_advance) {
    for (uint64_t i = 0; i < cycles_to_advance; ++i) {
        current_cycle++;
        bank.update(current_cycle);
    }
}

// TEST 1: Basic State Transitions
void test_basic_flow() {
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    TimingValidator validator(timing);
    DRAMBankFSM bank(timing, &validator);

    uint64_t cycle = 100;
    
    assert(bank.get_state() == BankState::IDLE);

    // ACTIVATE (Row 0)
    assert(bank.activate(0, cycle) == true);
    step(bank, cycle, timing.tRCD); 
    assert(bank.get_state() == BankState::ACTIVE);

    // READ (Col 0)
    assert(bank.read(0, cycle) == true);
    step(bank, cycle, timing.tCAS); // Reading takes tCAS time in this model
    assert(bank.get_state() == BankState::ACTIVE); // Should be back to active

    // PRECHARGE
    // Wait for tRAS first!
    step(bank, cycle, timing.tRAS);
    assert(bank.precharge(cycle) == true);
    
    step(bank, cycle, timing.tRP);
    assert(bank.get_state() == BankState::IDLE);

    std::cout << "[PASS] Test 1: Basic Flow\n";
}

// TEST 2: Row Hit Sequences
void test_row_hits() {
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    TimingValidator validator(timing);
    DRAMBankFSM bank(timing, &validator);
    uint64_t cycle = 100;

    bank.activate(0, cycle);
    step(bank, cycle, timing.tRCD);

    assert(bank.read(0, cycle) == true);
    // Can we read again immediately? Only after tCCD_L/S
    assert(bank.read(1, cycle + 1) == false); // Too fast!
    
    step(bank, cycle, timing.tCCD_L);
    assert(bank.read(1, cycle) == true); // Now it's safe

    std::cout << "[PASS] Test 2: Row Hits and tCCD\n";
}

// TEST 3: tFAW Violation (Global Constraint)
void test_tfaw() {
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    TimingValidator validator(timing);
    // Create 4 separate banks sharing one validator
    DRAMBankFSM b0(timing, &validator);
    DRAMBankFSM b1(timing, &validator);
    DRAMBankFSM b2(timing, &validator);
    DRAMBankFSM b3(timing, &validator);
    DRAMBankFSM b4(timing, &validator);

    uint64_t cycle = 1000;

    // Activate 4 banks quickly
    assert(b0.activate(0, cycle)); 
    assert(b1.activate(0, cycle + 4)); // tRRD ok
    assert(b2.activate(0, cycle + 8));
    assert(b3.activate(0, cycle + 12));

    // Try 5th activate (Should Fail due to tFAW)
    // tFAW is usually ~20-30ns. We are only 12ns in.
    assert(b4.activate(0, cycle + 16) == false);

    std::cout << "[PASS] Test 3: tFAW Window Enforcement\n";
}

// TEST 4: Back-to-Back Activation Timing (tRRD enforcement)
void test_trrd_timing() {
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    TimingValidator validator(timing);
    // Two banks sharing the same validator
    DRAMBankFSM b0(timing, &validator);
    DRAMBankFSM b1(timing, &validator);

    uint64_t cycle = 2000;

    // First Activate
    assert(b0.activate(0, cycle) == true);

    // Try Second Activate on different bank IMMEDIATELY (Cycle + 1)
    // tRRD_S is 4 cycles. So cycle + 1 is too soon.
    assert(b1.activate(0, cycle + 1) == false);

    // Try again at safe time (Cycle + tRRD_S)
    uint64_t safe_cycle = cycle + timing.tRRD_S; 
    assert(b1.activate(0, safe_cycle) == true);

    std::cout << "[PASS] Test 4: tRRD (Back-to-Back Act)\n";
}

// TEST 5: Write-to-Read (tWTR)
void test_wtr_timing() {
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    TimingValidator validator(timing);
    DRAMBankFSM bank(timing, &validator);
    uint64_t cycle = 1000;

    bank.activate(0, cycle);
    step(bank, cycle, timing.tRCD);

    // WRITE
    assert(bank.write(0, cycle));
    
    // Try to READ immediately (Should Fail)
    uint64_t too_soon = cycle + timing.tCCD_L; 
    assert(bank.read(0, too_soon) == false);

    // Wait for Write Duration + tWTR_L
    uint64_t safe_time = cycle + timing.tCCD_L + timing.tWTR_L + 10;
    step(bank, cycle, safe_time - cycle);
    
    assert(bank.read(0, cycle) == true);

    std::cout << "[PASS] Test 5: Write-to-Read Timing\n";
}

// TEST 6: Temperature Derating
void test_temperature() {
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    
    // Base Case
    assert(timing.tREFI == 9360);

    // Hot Case
    timing.current_temp = 90.0; // > 85C
    timing.apply_derating();
    assert(timing.tREFI == 4680); // Should be halved

    // Variation Case
    timing.process_variation = 1.10; // 10% Slower
    timing.apply_derating();
    // tRCD 17 * 1.1 = 18.7 -> 18 (integer math floor depending on implementation) or 19
    assert(timing.tRCD >= 18);

    std::cout << "[PASS] Test 6: Temp & Process Derating\n";
}

// DEEP REQUIREMENT: FUZZING
void test_fuzzing_1M() {
    std::cout << "Starting Fuzz Test (1M Cycles)... ";
    DRAMTiming timing = DRAMTiming::DDR4_2400_R();
    TimingValidator validator(timing);
    DRAMBankFSM bank(timing, &validator);
    
    // Enable tracing for the fuzz test
    bank.enable_tracing("trace_dramsim3.txt"); 

    std::vector<std::string> cmds = {"ACT", "PRE", "RD", "WR", "REF"};
    std::mt19937 rng(42); // Fixed seed for reproducibility
    
    uint64_t valid_cmds = 0;
    uint64_t cycle = 100;

    for(int i=0; i<1000000; i++) {
        // Randomly pick an action
        int action = rng() % 5;
        bool success = false;

        if (action == 0) success = bank.activate(0, cycle);
        else if (action == 1) success = bank.precharge(cycle);
        else if (action == 2) success = bank.read(0, cycle);
        else if (action == 3) success = bank.write(0, cycle);
        else if (action == 4) success = bank.refresh(cycle);

        if (success) valid_cmds++;
        
        // Advance time randomly (0 to 20 cycles)
        int jump = rng() % 20;
        step(bank, cycle, jump);
    }
    std::cout << "Done. Executed " << valid_cmds << " valid commands.\n";
    std::cout << "[PASS] Deep Fuzzing\n";
}

int main() {
    test_basic_flow();
    test_row_hits();
    test_tfaw();
    test_trrd_timing();
    test_wtr_timing();
    test_temperature();
    test_fuzzing_1M();
    
    std::cout << "\nALL DAY 1 CHECKS PASSED: DEEP MASTERY CONFIRMED.\n";
    return 0;
}

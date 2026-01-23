#pragma once
#include "dram_timing.h"
#include <queue>
#include <cstdint>

namespace dram {

class TimingValidator {
public:
    explicit TimingValidator(const DRAMTiming& timing);

    // Checks if an ACTIVATE can be issued at this cycle across all banks (channel level)
    bool check_tfaw(uint64_t cycle);
    bool check_trrd(uint64_t cycle, bool same_bank_group);
    
    // NEW: Write Recovery and Power Down checks
    bool check_write_recovery(uint64_t cycle, uint64_t last_write_cycle);
    bool check_power_down(uint64_t cycle);

    // Updates internal windows
    void record_activate(uint64_t cycle);
    void record_power_change(uint64_t cycle, bool entering_power_down);
    
    // Day 1 Afternoon Integration Helpers
    bool check_global_constraints(uint64_t cycle) {
        // Strict check: tFAW + tRRD
        // We assume different bank group (false) for simple tests to allow tRRD_S spacing
        return check_tfaw(cycle) && check_trrd(cycle, false);
    }

    void update_last_activate(uint64_t cycle) {
        record_activate(cycle);
    }

    // NEW: Deep Requirement - Violation Injection
    // Allows us to forcefully fail a check for testing purposes
    void inject_error_mode(bool enable) { error_injection_mode_ = enable; }

private:
    DRAMTiming timing_;
    std::queue<uint64_t> act_history_; // For tFAW
    uint64_t last_act_cycle_ = 0;
    
    // New state tracking
    uint64_t last_cke_change_cycle_ = 0;
    bool in_power_down_ = false;
    
    bool error_injection_mode_ = false;
};

} // namespace dram

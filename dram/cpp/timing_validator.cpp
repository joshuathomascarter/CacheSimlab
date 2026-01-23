#include "../headers/timing_validator.h"

namespace dram {

TimingValidator::TimingValidator(const DRAMTiming& timing) 
    : timing_(timing), last_act_cycle_(0), last_cke_change_cycle_(0) {}

bool TimingValidator::check_tfaw(uint64_t cycle) {
    if (error_injection_mode_) return false; // Force failure for testing

    // Remove activations outside the window
    while (!act_history_.empty() && act_history_.front() + timing_.tFAW <= cycle) {
        act_history_.pop();
    }
    
    // DDR4 constraint: max 4 activates in tFAW window
    return act_history_.size() < 4;
}

bool TimingValidator::check_trrd(uint64_t cycle, bool same_bank_group) {
    if (error_injection_mode_) return false;

    // FIX: logic was swapped previously.
    // SAME group = Long delay (resource contention)
    // DIFF group = Short delay
    uint64_t delay = same_bank_group ? timing_.tRRD_L : timing_.tRRD_S;
    
    // Avoid underflow on first cycle
    if (last_act_cycle_ == 0 && act_history_.empty()) return true;

    return (cycle >= last_act_cycle_ + delay);
}

// NEW: Write Recovery (tWR)
// Ensures we don't Precharge too soon after a Write
bool TimingValidator::check_write_recovery(uint64_t cycle, uint64_t last_write_cycle) {
    if (error_injection_mode_) return false;
    
    // tWR is counted from the END of the write burst
    // Standard approx: Write Time + Burst + tWR
    uint64_t min_precharge_time = last_write_cycle + timing_.tBL + timing_.tWR;
    return cycle >= min_precharge_time;
}

// NEW: CKE (Clock Enable) Power Down Constraint
bool TimingValidator::check_power_down(uint64_t cycle) {
    // You cannot toggle power state faster than tCKE
    if (last_cke_change_cycle_ > 0 && (cycle < last_cke_change_cycle_ + timing_.tCKE)) {
        return false; 
    }
    return true;
}

void TimingValidator::record_activate(uint64_t cycle) {
    act_history_.push(cycle);
    last_act_cycle_ = cycle;
}

void TimingValidator::record_power_change(uint64_t cycle, bool entering_power_down) {
    last_cke_change_cycle_ = cycle;
    in_power_down_ = entering_power_down;
}

} // namespace dram

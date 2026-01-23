#include "../headers/dram_bank_fsm.h"
#include "../headers/timing_validator.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace dram {

DRAMBankFSM::DRAMBankFSM(const DRAMTiming& timing, TimingValidator* validator)
    : timing_(timing), state_(BankState::IDLE), open_row_(0), 
      state_end_cycle_(INVALID_CYCLE), validator_(validator) {
        // Apply scaling logic immediately on creation
        // Note: timing_ is a copy so modifying it is safe
        // In real arch, we might call timing_.apply_derating();
    }

bool DRAMBankFSM::can_activate(uint64_t cycle) const {
    if (state_ != BankState::IDLE) return false;

    // Check global constraints via Validator (e.g., tFAW, tRRD)
    if (validator_ && !validator_->check_global_constraints(cycle)) {
        return false;
    }

    // tFAW check (Four Activation Window) using simplified local history
    // (Note: In full Rank system, this would be a Rank-level check)
    // Here we assume activation_window_ is maintained
    if (activation_window_.size() >= 4) {
        uint64_t oldest_act = activation_window_.front();
        if (cycle < oldest_act + timing_.tFAW) return false;
    }

    uint64_t last_pre = get_last_cmd_cycle("PRECHARGE");
    if (last_pre != INVALID_CYCLE && cycle < last_pre + timing_.tRP) return false;

    // tRC check
    uint64_t last_act = get_last_cmd_cycle("ACTIVATE");
    if (last_act != INVALID_CYCLE && cycle < last_act + timing_.tRC) return false;

    return true;
}

bool DRAMBankFSM::activate(uint32_t row, uint64_t cycle) {
    if (!can_activate(cycle)) return false;

    state_ = BankState::ACTIVATING;
    open_row_ = row;
    state_end_cycle_ = cycle + timing_.tRCD;
    record_command("ACTIVATE", cycle, row);

    
    // Notify validator of activation
    if (validator_) {
        validator_->update_last_activate(cycle);
    }

    // Maintain tFAW window
    activation_window_.push(cycle);
    while (activation_window_.size() > 4) {
        activation_window_.pop();
    }
    
    return true;
}

bool DRAMBankFSM::can_read(uint64_t cycle) const {
    // Allow pipelining: Can read while already reading/writing if timing constraints are met
    if (state_ != BankState::ACTIVE && state_ != BankState::READING && state_ != BankState::WRITING) return false;

    // tRCD check (covered by state check + update)
    // tCCD check
    uint64_t last_read = get_last_cmd_cycle("READ");
    uint64_t last_write = get_last_cmd_cycle("WRITE");
    if (last_read != INVALID_CYCLE && cycle < last_read + timing_.tCCD_L) return false;
    if (last_write != INVALID_CYCLE && cycle < last_write + timing_.tWTR_L) return false;

    return true;
}

bool DRAMBankFSM::read(uint32_t col, uint64_t cycle) {
    if (!can_read(cycle)) return false;

    state_ = BankState::READING;
    state_end_cycle_ = cycle + timing_.tCAS;
    record_command("READ", cycle);
    return true;
}

bool DRAMBankFSM::can_write(uint64_t cycle) const {
    if (state_ != BankState::ACTIVE && state_ != BankState::READING && state_ != BankState::WRITING) return false;

    uint64_t last_write = get_last_cmd_cycle("WRITE");
    uint64_t last_read = get_last_cmd_cycle("READ");
    if (last_write != INVALID_CYCLE && cycle < last_write + timing_.tCCD_L) return false;
    if (last_read != INVALID_CYCLE && cycle < last_read + timing_.tCAS) return false; 

    return true;
}

bool DRAMBankFSM::write(uint32_t col, uint64_t cycle) {
    if (!can_write(cycle)) return false;

    state_ = BankState::WRITING;
    state_end_cycle_ = cycle + timing_.tCAS; // simplified, often WL + BL/2 + tWR
    record_command("WRITE", cycle);
    return true;
}

bool DRAMBankFSM::can_precharge(uint64_t cycle) const {
    if (state_ != BankState::ACTIVE) return false;

    // tRAS check
    uint64_t last_act = get_last_cmd_cycle("ACTIVATE");
    if (last_act != INVALID_CYCLE && cycle < last_act + timing_.tRAS) return false;

    // tWR check after write
    uint64_t last_write = get_last_cmd_cycle("WRITE");
    if (last_write != INVALID_CYCLE && cycle < last_write + timing_.tWR) return false;

    return true;
}

bool DRAMBankFSM::precharge(uint64_t cycle) {
    if (!can_precharge(cycle)) return false;

    state_ = BankState::PRECHARGING;
    state_end_cycle_ = cycle + timing_.tRP;
    record_command("PRECHARGE", cycle);
    return true;
}

bool DRAMBankFSM::can_refresh(uint64_t cycle) const {
    return state_ == BankState::IDLE;
}

bool DRAMBankFSM::refresh(uint64_t cycle) {
    if (!can_refresh(cycle)) return false;

    state_ = BankState::REFRESHING;
    state_end_cycle_ = cycle + timing_.tRFC;
    record_command("REFRESH", cycle);
    return true;
}

void DRAMBankFSM::update(uint64_t cycle) {
    // Only update if we have a valid end time
    if (state_end_cycle_ != INVALID_CYCLE && cycle >= state_end_cycle_) {
        if (state_ == BankState::ACTIVATING || state_ == BankState::READING || state_ == BankState::WRITING) {
            state_ = BankState::ACTIVE;
        } else if (state_ == BankState::PRECHARGING || state_ == BankState::REFRESHING) {
            state_ = BankState::IDLE;
        }
        // Clear the timer so we don't re-trigger
        state_end_cycle_ = INVALID_CYCLE;
    }
}

void DRAMBankFSM::enable_tracing(const std::string& filename) {
    trace_stream_.open(filename);
    if (trace_stream_.is_open()) {
        trace_stream_ << "Cycle,Command,Bank,Row" << std::endl;
    }
}

void DRAMBankFSM::record_command(const std::string& cmd, uint64_t cycle, uint32_t row) {
    // Use deque (push_front logic if preferred, or push_back/pop_front)
    // Using simple history maintenance
    history_.push_back({cmd, cycle, row});
    if (history_.size() > 100) history_.pop_front();

    if (trace_stream_.is_open()) {
        trace_stream_ << cycle << "," << cmd << ",0," << row << std::endl;
    }
}

uint64_t DRAMBankFSM::get_last_cmd_cycle(const std::string& cmd) const {
    for (auto it = history_.rbegin(); it != history_.rend(); ++it) {
        if (it->cmd == cmd) return it->cycle;
    }
    return INVALID_CYCLE;
}

} // namespace dram

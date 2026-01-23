#pragma once
#include "dram_timing.h"
#include <vector>
#include <string>
#include <deque>
#include <iostream>
#include <fstream>

namespace dram {

enum class BankState {
    IDLE,
    ACTIVATING,
    ACTIVE,
    READING,
    WRITING,
    PRECHARGING,
    REFRESHING
};

struct CommandRecord {
    std::string cmd;
    uint64_t cycle;
    uint32_t row;
};

class TimingValidator; // Forward declaration

class DRAMBankFSM {
public:
    explicit DRAMBankFSM(const DRAMTiming& timing, TimingValidator* validator);

    // Command Interface
    bool activate(uint32_t row, uint64_t cycle);
    bool read(uint32_t col, uint64_t cycle);
    bool write(uint32_t col, uint64_t cycle);
    bool precharge(uint64_t cycle);
    bool refresh(uint64_t cycle);

    // State Updates
    void update(uint64_t cycle);

    // Getters
    BankState get_state() const { return state_; }
    uint32_t get_open_row() const { return open_row_; }
    const std::deque<CommandRecord>& get_history() const { return history_; }

    // Timing Validation Interface
    bool can_activate(uint64_t cycle) const;
    bool can_read(uint64_t cycle) const;
    bool can_write(uint64_t cycle) const;
    bool can_precharge(uint64_t cycle) const;
    bool can_refresh(uint64_t cycle) const;

    // Trace control
    void enable_tracing(const std::string& filename);
    
    // Close trace when done
    ~DRAMBankFSM() {
        if (trace_stream_.is_open()) trace_stream_.close();
    }

private:
    DRAMTiming timing_;
    BankState state_;
    uint32_t open_row_;
    uint64_t state_end_cycle_;
    TimingValidator* validator_;
    
    // History for debugging and tFAW/tRRD checks
    std::deque<CommandRecord> history_;
    std::queue<uint64_t> activation_window_;
    std::ofstream trace_stream_;

    void record_command(const std::string& cmd, uint64_t cycle, uint32_t row = 0);
    uint64_t get_last_cmd_cycle(const std::string& cmd) const;

    static constexpr uint64_t INVALID_CYCLE = 0xFFFFFFFFFFFFFFFF;
};

} // namespace dram

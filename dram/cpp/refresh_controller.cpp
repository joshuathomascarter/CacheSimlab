/**
 * @file refresh_controller.cpp
 * @brief Implementation of DRAM Refresh Controller
 * 
 * Production-grade refresh controller with:
 * - Dynamic mode switching
 * - Temperature-aware scheduling
 * - Retention profiling (RAIDR algorithm)
 * - Power optimization through clustering
 * 
 * References:na
 * - JEDEC JESD79-4C DDR4 Specification
 * - Liu et al., "RAIDR: Retention-Aware Intelligent DRAM Refresh", ISCA 2012
 * - Micron Technical Note TN-40-46
 */

#include "../headers/refresh_controller.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dram {

// ========== JEDEC DDR4 Constants ==========

// Base refresh interval at 85°C (in nanoseconds)
constexpr uint64_t BASE_TREFI_NS = 7800;  // 7.8μs per JEDEC

// Refresh command time for different modes (8Gb device)
constexpr uint64_t TRFC_ALL_BANK_NS = 260;  // All-bank refresh
constexpr uint64_t TRFC_PER_BANK_NS = 140;  // Per-bank refresh

// Maximum postponement: 8x tREFI per JEDEC
constexpr uint32_t MAX_POSTPONEMENT_FACTOR = 8;

// Temperature thresholds (°C)
constexpr double TEMP_NORMAL_MAX = 85.0;
constexpr double TEMP_EXTENDED_85C_MAX = 95.0;

// Retention profiling bins
constexpr uint8_t NUM_RETENTION_BINS = 8;

// ========== Constructor ==========

RefreshController::RefreshController(const DRAMTiming& timing,
                                    uint32_t num_banks,
                                    uint32_t rows_per_bank)
    : timing_(timing)
    , num_banks_(num_banks)
    , rows_per_bank_(rows_per_bank)
    , current_mode_(RefreshMode::ALL_BANK)
    , thermal_range_(ThermalRange::NORMAL)
    , current_temperature_(25.0)
    , retention_profiling_enabled_(false)
    , clustering_enabled_(false)
    , total_refreshes_(0)
    , postponed_refreshes_(0)
    , max_postponement_cycles_(0)
    , total_refresh_cycles_(0)
{
    // Calculate base refresh interval in cycles
    // Assuming typical frequency of 1600 MHz (0.625ns per cycle)
    double cycle_time_ns = 1000.0 / 1600.0;  // ~0.625ns
    base_trefi_ = static_cast<uint64_t>(BASE_TREFI_NS / cycle_time_ns);
    current_trefi_ = base_trefi_;
    current_trfc_ = static_cast<uint64_t>(TRFC_ALL_BANK_NS / cycle_time_ns);
    max_postponement_ = base_trefi_ * MAX_POSTPONEMENT_FACTOR;
    cluster_window_ = base_trefi_ / 10;
    
    // fix vector init and typos
    bank_states_.resize(num_banks_);
    for (auto& state : bank_states_) {
        state.last_refresh_cycle = 0;
        state.next_refresh_cycle = base_trefi_;
        state.current_row = 0;
        state.refresh_in_progress = false;
        state.postponement_count = 0;
    }
    
    retention_profiles_.resize(num_banks_);
    for (auto& bank_profiles : retention_profiles_) {
        bank_profiles.resize(rows_per_bank_);
        for (uint32_t row = 0; row < rows_per_bank_; ++row) {
            auto& profile = bank_profiles[row];
            profile.row_address = row;
            profile.measured_min_retention_cycles = UINT64_MAX; // Start at max, will be updated
            profile.safe_retention_limit_cycles = base_trefi_ * MAX_POSTPONEMENT_FACTOR; // JEDEC 8x
            profile.last_refresh_cycle = 0;
            profile.retention_bin = NUM_RETENTION_BINS / 2;
            profile.weak_cell_count = 0;
            profile.requires_frequent_refresh = false;
        }
    }
}

// ========== Core Refresh Operations ==========

bool RefreshController::is_refresh_needed(uint64_t current_cycle) const {
    // Check if any bank needs refresh
    for (const auto& state : bank_states_) {
        if (current_cycle >= state.next_refresh_cycle) {
            return true;
        }
    }
    
    // Check if any queued refreshes are overdue
    if (!refresh_queue_.empty()) {
        const RefreshCommand& top = refresh_queue_.top();
        // Only wake up if the highest-priority command is urgent or overdue
        if (top.urgency_level >= 200 || top.is_overdue(current_cycle)) {
            return true;
        }
    }
    
    return false;
}

std::unique_ptr<RefreshCommand> RefreshController::get_next_refresh(uint64_t current_cycle) {
    // Priority 1: Check queue for high-urgency refreshes
    if (!refresh_queue_.empty()) {
        RefreshCommand cmd = refresh_queue_.top();
        if (cmd.urgency_level >= 200 || cmd.is_overdue(current_cycle)) {
            refresh_queue_.pop();
            return std::make_unique<RefreshCommand>(cmd);
        }
    }
    
    // Priority 2: Check banks for scheduled refreshes
    for (uint32_t bank = 0; bank < num_banks_; ++bank) {
        auto& state = bank_states_[bank];
        
        if (state.refresh_in_progress) {
            continue; // Bank already refreshing
        }
        
        if (current_cycle >= state.next_refresh_cycle) {
            // Generate refresh command for this bank
            RefreshCommand cmd = generate_refresh_command(bank, current_cycle);
            
            // Update bank state
            state.refresh_in_progress = true;
            state.last_refresh_cycle = current_cycle;
            
            return std::make_unique<RefreshCommand>(cmd);
        }
    }
    
    return nullptr; // No refresh needed right now
}

void RefreshController::complete_refresh(const RefreshCommand& cmd, 
                                        uint64_t completion_cycle) {
    total_refreshes_++;
    total_refresh_cycles_ += current_trfc_;
    
    // Update bank state
    if (cmd.bank_id != 0xFF) { // Not all-bank
        update_bank_state(cmd.bank_id, completion_cycle);
    } else { // All-bank refresh
        for (uint32_t bank = 0; bank < num_banks_; ++bank) {
            update_bank_state(bank, completion_cycle);
        }
    }
    
    // Update retention profiles if enabled
    if (retention_profiling_enabled_) {
        for (uint32_t row = cmd.row_start; 
             row < cmd.row_start + cmd.row_count; ++row) {
            if (cmd.bank_id != 0xFF) {
                retention_profiles_[cmd.bank_id][row].last_refresh_cycle = completion_cycle;
            } else {
                for (uint32_t bank = 0; bank < num_banks_; ++bank) {
                    retention_profiles_[bank][row].last_refresh_cycle = completion_cycle;
                }
            }
        }
    }
}

bool RefreshController::try_postpone_refresh(uint64_t current_cycle) {
    // Find refresh candidates that are due soon (within 10% of deadline)
    uint32_t candidate_bank = num_banks_;
    int64_t min_slack = INT64_MAX;
    
    for (uint32_t bank = 0; bank < num_banks_; ++bank) {
        auto& state = bank_states_[bank];
        
        if (state.refresh_in_progress) {
            continue;
        }
        
        // Calculate time until refresh is due
        int64_t cycles_until_refresh = static_cast<int64_t>(state.next_refresh_cycle) - static_cast<int64_t>(current_cycle);
        
        // Consider postponing if refresh is coming up soon (within 10% of tREFI) or already due
        if (cycles_until_refresh <= static_cast<int64_t>(current_trefi_ / 10)) {
            // Check if we can safely postpone
            if (can_safely_postpone(bank, current_cycle)) {
                if (cycles_until_refresh < min_slack) {
                    min_slack = cycles_until_refresh;
                    candidate_bank = bank;
                }
            }
        }
    }
    
    if (candidate_bank < num_banks_) {
        // Postpone this bank's refresh
        auto& state = bank_states_[candidate_bank];
        state.next_refresh_cycle += current_trefi_ / 4; // Postpone by 1/4 interval
        state.postponement_count++;
        
        postponed_refreshes_++;
        max_postponement_cycles_ = std::max(max_postponement_cycles_, 
                                           state.postponement_count * (current_trefi_ / 4));
        
        return true;
    }
    
    return false; // Cannot postpone safely
}

// ========== Mode and Temperature Management ==========

void RefreshController::set_refresh_mode(RefreshMode mode) {
    if (mode == current_mode_) {
        return;
    }
    
    current_mode_ = mode;
    
    // Update timing parameters based on mode
    double cycle_time_ns = 1000.0 / 1600.0;
    
    switch (mode) {
        case RefreshMode::ALL_BANK:
            current_trfc_ = static_cast<uint64_t>(TRFC_ALL_BANK_NS / cycle_time_ns);
            break;
            
        case RefreshMode::PER_BANK:
            current_trfc_ = static_cast<uint64_t>(TRFC_PER_BANK_NS / cycle_time_ns);
            break;
            
        case RefreshMode::FINE_GRAIN_1X:
            current_trefi_ = base_trefi_;
            break;
            
        case RefreshMode::FINE_GRAIN_2X:
            current_trefi_ = base_trefi_ / 2;
            break;
            
        case RefreshMode::FINE_GRAIN_4X:
            current_trefi_ = base_trefi_ / 4;
            break;
    }
}

void RefreshController::update_temperature(double temp_celsius) {
    current_temperature_ = temp_celsius;
    
    // Calculate what new_range would be
    ThermalRange new_range;
    if (temp_celsius <= TEMP_NORMAL_MAX) {
        new_range = ThermalRange::NORMAL;
    } else if (temp_celsius <= TEMP_EXTENDED_85C_MAX) {
        new_range = ThermalRange::EXTENDED;
    } else {
        new_range = ThermalRange::HIGH_TEMP;
    }
    
    // Early exit if no change needed
    if (new_range == thermal_range_) {
        return;  // No work needed!
    }
    
    // Range changed - now update tREFI and banks
    thermal_range_ = new_range;
    current_trefi_ = calculate_trefi(temp_celsius);  // Use the helper function
    
    // Update all bank next refresh times
    for (auto& state : bank_states_) {
        state.next_refresh_cycle = state.last_refresh_cycle + current_trefi_;
    }
}

// ========== Retention Profiling ==========

void RefreshController::enable_retention_profiling(bool enable) {
    retention_profiling_enabled_ = enable;
    
    if (enable) {
        std::cout << "Retention profiling enabled (RAIDR mode)\n";
    }
}

void RefreshController::update_row_profile(uint32_t bank, uint32_t row,
                                          uint64_t retention_time) {
    if (!retention_profiling_enabled_ || bank >= num_banks_ || row >= rows_per_bank_) {
        return;
    }
    
    auto& profile = retention_profiles_[bank][row];
    
    // Update minimum retention time (use min for first update, then track minimum)
    if (profile.measured_min_retention_cycles == UINT64_MAX) {
        // First measurement - just set it
        profile.measured_min_retention_cycles = retention_time;
    } else {
        // Subsequent measurements - track minimum
        profile.measured_min_retention_cycles = std::min(profile.measured_min_retention_cycles, retention_time);
    }
    
    // Categorize into retention bin
    uint64_t bin_threshold = base_trefi_ * 64 / NUM_RETENTION_BINS;
    profile.retention_bin = static_cast<uint8_t>(
        std::min(retention_time / bin_threshold, 
                static_cast<uint64_t>(NUM_RETENTION_BINS - 1))
    );
    
    // Mark if requires frequent refresh (RAIDR: bottom 10% of retention distribution)
    // Use 0.5x tREFI as threshold for weak rows (significantly below normal)
    profile.requires_frequent_refresh = (retention_time < base_trefi_ / 2);
    
    // Count weak cells (simplified model)
    if (profile.requires_frequent_refresh) {
        profile.weak_cell_count++;
    }
}

// ========== Power Optimization ==========

void RefreshController::enable_refresh_clustering(bool enable) {
    clustering_enabled_ = enable;
}

double RefreshController::get_refresh_overhead() const {
    if (total_refreshes_ == 0) {
        return 0.0;
    }
    
    // Calculate percentage of time spent in refresh
    // Standard calculation: (tRFC / tREFI) * 100%
    // For DDR4-8Gb: (260ns / 7.8μs) * 100% ≈ 3.33%
    
    double overhead_per_refresh = static_cast<double>(current_trfc_) / 
                                  static_cast<double>(current_trefi_);
    
    return overhead_per_refresh * 100.0;
}

// ========== Statistics ==========

void RefreshController::reset_statistics() {
    total_refreshes_ = 0;
    postponed_refreshes_ = 0;
    max_postponement_cycles_ = 0;
    total_refresh_cycles_ = 0;
}

void RefreshController::export_refresh_trace(const std::string& filename) const {
    std::ofstream out(filename);
    
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file for refresh trace export");
    }
    
    out << "# DRAM Refresh Trace Export\n";
    out << "# Total Refreshes: " << total_refreshes_ << "\n";
    out << "# Postponed: " << postponed_refreshes_ << "\n";
    out << "# Overhead: " << get_refresh_overhead() << "%\n";
    out << "# Mode: ";
    
    switch (current_mode_) {
        case RefreshMode::ALL_BANK: out << "ALL_BANK\n"; break;
        case RefreshMode::PER_BANK: out << "PER_BANK\n"; break;
        case RefreshMode::FINE_GRAIN_1X: out << "FINE_GRAIN_1X\n"; break;
        case RefreshMode::FINE_GRAIN_2X: out << "FINE_GRAIN_2X\n"; break;
        case RefreshMode::FINE_GRAIN_4X: out << "FINE_GRAIN_4X\n"; break;
    }
    
    out << "\nBank,LastRefreshCycle,NextRefreshCycle,PostponementCount\n";
    
    for (uint32_t bank = 0; bank < num_banks_; ++bank) {
        const auto& state = bank_states_[bank];
        out << bank << ","
            << state.last_refresh_cycle << ","
            << state.next_refresh_cycle << ","
            << static_cast<int>(state.postponement_count) << "\n";
    }
    
    out.close();
}

// ========== Private Helper Methods ==========

uint64_t RefreshController::calculate_trefi(double temp_celsius) const {
    if (temp_celsius <= TEMP_NORMAL_MAX) {
        return base_trefi_;
    } else if (temp_celsius <= TEMP_EXTENDED_85C_MAX) {
        return base_trefi_ / 2;
    } else {
        return base_trefi_ / 4;
    }
}

uint8_t RefreshController::calculate_urgency(uint64_t current_cycle,
                                            uint64_t deadline_cycle) const {
    if (current_cycle >= deadline_cycle) {
        return 255; // Critical - already overdue
    }
    
    uint64_t slack = deadline_cycle - current_cycle;
    uint64_t slack_ratio = (slack * 255) / current_trefi_;
    
    return static_cast<uint8_t>(255 - std::min(slack_ratio, static_cast<uint64_t>(255)));
}

RefreshCommand RefreshController::generate_refresh_command(uint32_t bank,
                                                           uint64_t current_cycle) {
    RefreshCommand cmd;
    
    if (current_mode_ == RefreshMode::ALL_BANK) {
        cmd.bank_id = 0xFF; // Special marker for all-bank
    } else {
        cmd.bank_id = bank;
    }
    
    cmd.row_start = 0;
    cmd.row_count = rows_per_bank_;
    cmd.deadline_cycle = bank_states_[bank].next_refresh_cycle + max_postponement_;
    cmd.scheduled_cycle = current_cycle;
    cmd.urgency_level = calculate_urgency(current_cycle, cmd.deadline_cycle);
    cmd.mode = current_mode_;
    
    return cmd;
}

void RefreshController::update_bank_state(uint32_t bank, uint64_t completion_cycle) {
    auto& state = bank_states_[bank];
    
    state.last_refresh_cycle = completion_cycle;
    state.next_refresh_cycle = completion_cycle + current_trefi_;
    state.refresh_in_progress = false;
    state.postponement_count = 0; // Reset after successful refresh
    
    // Advance row counter for rolling refresh
    state.current_row = (state.current_row + 1) % rows_per_bank_;
}

bool RefreshController::can_safely_postpone(uint32_t bank, uint64_t current_cycle) const {
    const auto& state = bank_states_[bank];
    
    // Cannot postpone if already at max
    if (state.postponement_count >= MAX_POSTPONEMENT_FACTOR) {
        return false;
    }
    
    // Cannot postpone if deadline would be exceeded
    uint64_t potential_next = state.next_refresh_cycle + current_trefi_ / 4;
    if (potential_next > state.next_refresh_cycle + max_postponement_) {
        return false;
    }
    
    // Check retention profiles if enabled
    if (retention_profiling_enabled_) {
        // RAIDR Key Insight: Only check the WEAKEST rows in this bank
        // Strong rows (90%+) can safely wait longer
        
        uint64_t min_retention_in_bank = UINT64_MAX;
        bool has_initialized_rows = false;
        
        for (const auto& profile : retention_profiles_[bank]) {
            if (profile.measured_min_retention_cycles != UINT64_MAX) {
                min_retention_in_bank = std::min(min_retention_in_bank, 
                                                 profile.measured_min_retention_cycles);
                has_initialized_rows = true;
            }
        }
        
        // If we have retention data, check the weakest row
        if (has_initialized_rows) {
            uint64_t time_since_last_refresh = current_cycle - state.last_refresh_cycle;
            uint64_t postponement_time = current_trefi_ / 4;
            uint64_t safety_threshold = min_retention_in_bank * 9 / 10;
            
            // Can we safely wait another quarter-interval?
            // Conservative: don't exceed 90% of weakest row's retention time
            if (time_since_last_refresh + postponement_time > safety_threshold) {
                return false; // Too risky - weakest row approaching limit
            }
        }
    }
    
    return true;
}

RetentionStats RefreshController::get_retention_stats() const {
    RetentionStats stats{0, 0};
    uint64_t total_retention = 0;
    uint32_t total_rows = 0;
    
    // Iterate through all banks and rows
    for (const auto& bank_profiles : retention_profiles_) {
        for (const auto& profile : bank_profiles) {
            uint64_t retention = profile.measured_min_retention_cycles;
            
            // Skip uninitialized rows
            if (retention == UINT64_MAX) {
                continue;
            }
            
            // Count weak rows (retention < half of base tREFI)
            // base_trefi_ is ~12,480 cycles, so threshold is ~6,240
            // Weak rows should have retention < 6,240 (from /3 operation: 5,000-25,000 / 3 = 1,667-8,333)
            if (retention < base_trefi_ / 2) {
                stats.weak_rows++;
            }
            
            total_retention += retention;
            total_rows++;
        }
    }
    
    // Calculate average
    stats.avg_retention = total_rows > 0 ? total_retention / total_rows : 0;
    
    return stats;
}

} // namespace dram

/**
 * @file refresh_controller.h
 * @brief DRAM Refresh Controller with Advanced Power Optimization
 * 
 * Production-grade refresh controller implementing:
 * - JEDEC DDR4/DDR5 compliant refresh modes
 * - Temperature-aware scheduling (85°C/95°C thresholds)
 * - Retention profiling (RAIDR algorithm)
 * - Power optimization through clustering and postponement
 * - Per-bank refresh for DDR5 compatibility
 * - Comprehensive statistics and trace export
 * 
 * Architecture Standards:
 * - Cycle-accurate timing simulation
 * - Robust error handling with validation
 * - Comprehensive logging and debugging support
 * - Performance metrics and power analysis
 * 
 * Based on:
 * - JEDEC JESD79-4C DDR4 Specification
 * - JEDEC JESD79-5 DDR5 Specification
 * - Liu et al., "RAIDR: Retention-Aware Intelligent DRAM Refresh", ISCA 2012
 * - Micron Technical Note TN-40-46 "DDR4 Power Management"
 * 
 * @author Josh Carter
 * @date January 2026
 * @version 2.0.0
 * @standard Apple Memory Architecture Team coding guidelines
 */

#pragma once

#include "dram_timing.h"
#include <vector>
#include <memory>
#include <string>
#include <queue>

namespace dram {

// ========== Forward Declarations ==========

class PowerModel;
class TimingValidator;

// ========== Refresh Mode Configuration ==========

/**
 * @brief Refresh operation modes (scope and frequency)
 */
enum class RefreshMode {
    ALL_BANK,      ///< Standard all-bank refresh (tRFC = 260ns @ 8Gb)
    PER_BANK,      ///< Per-bank refresh (tRFCpb = 140ns @ 8Gb) 
    FINE_GRAIN_1X, ///< Normal refresh rate (tREFI = 7.8μs)
    FINE_GRAIN_2X, ///< 2x refresh rate (tREFI = 3.9μs)
    FINE_GRAIN_4X  ///< 4x refresh rate (tREFI = 1.95μs)
};

/**
 * @brief Thermal operating ranges
 */
enum class ThermalRange {
    NORMAL,    ///< 0-85°C (standard tREFI)
    EXTENDED,  ///< 85-95°C (2x refresh rate)
    HIGH_TEMP  ///< >95°C (4x refresh rate)
};

/**
 * @brief Retention classification for profiling
 */
enum class RetentionClass {
    STRONG,   ///< >1s retention (reduce refresh)
    MEDIUM,   ///< 100ms-1s retention (normal refresh)
    WEAK      ///< <100ms retention (increase refresh)
};

/**
 * @brief Retention statistics structure
 */
struct RetentionStats {
    uint32_t weak_rows;
    uint64_t avg_retention;
};
struct RowRetentionProfile {
    uint32_t row_address;
    uint64_t measured_min_retention_cycles; // min observed retention (0 = unknown)
    uint64_t safe_retention_limit_cycles;   // JEDEC 8x limit (base_trefi_*8)
    uint64_t last_refresh_cycle;
    uint32_t weak_cell_count;
    uint8_t  retention_bin;
    bool     requires_frequent_refresh;
};

/**
 * @brief Refresh command structure
 */
struct RefreshCommand {
    uint32_t bank_id;              ///< Target bank (0xFF = all banks)
    uint32_t row_start;            ///< Starting row address
    uint32_t row_count;            ///< Number of rows to refresh
    uint64_t deadline_cycle;       ///< Latest completion cycle (safety)
    uint64_t scheduled_cycle;      ///< Planned execution cycle
    uint8_t  urgency_level;        ///< Priority (0=low, 255=critical)
    RefreshMode mode;              ///< Refresh mode for this operation
    
    RefreshCommand() 
        : bank_id(0), row_start(0), row_count(0)
        , deadline_cycle(0), scheduled_cycle(0), urgency_level(0)
        , mode(RefreshMode::ALL_BANK) {}
    
    /**
     * @brief Check if this refresh command is overdue
     * @param current_cycle Current simulation cycle
     * @return true if current_cycle > deadline_cycle
     */
    bool is_overdue(uint64_t current_cycle) const {
        return current_cycle > deadline_cycle;
    }
};

/**
 * @brief Priority queue comparator for refresh commands
 */
struct RefreshPriorityComparator {
    bool operator()(const RefreshCommand& a, const RefreshCommand& b) const {
        if (a.urgency_level != b.urgency_level) {
            return a.urgency_level < b.urgency_level; // Higher urgency first
        }
        return a.deadline_cycle > b.deadline_cycle; // Earlier deadline first
    }
};

// ========== Main Refresh Controller Class ==========

/**
 * @brief Advanced DRAM Refresh Controller
 * 
 * Implements intelligent refresh scheduling with:
 * - Multiple refresh modes (all-bank, per-bank, fine-grain)
 * - Temperature-aware frequency scaling
 * - Retention profiling for power optimization
 * - Postponement with safety limits
 * - Power clustering for efficiency
 * 
 * Thread Safety: Not thread-safe (single controller per memory channel)
 * Performance: O(1) for most operations, O(N) for full profiling
 */
class RefreshController {
public:
    // ========== Constructor and Configuration ==========
    
    /**
     * @brief Construct refresh controller
     * 
     * @param timing DRAM timing parameters (tRFC, tREFI, etc.)
     * @param num_banks Number of banks to manage
     * @param rows_per_bank Rows per bank for retention profiling
     * 
     * @throws std::invalid_argument if parameters are invalid
     */
    RefreshController(const DRAMTiming& timing, uint32_t num_banks, uint32_t rows_per_bank);
    
    /**
     * @brief Destructor
     */
    ~RefreshController() = default;
    
    // ========== Core Refresh Operations ==========
    
    /**
     * @brief Check if refresh is needed
     * 
     * @param current_cycle Current simulation cycle
     * @return True if any bank needs refresh
     */
    bool is_refresh_needed(uint64_t current_cycle) const;
    
    /**
     * @brief Get next refresh command
     * 
     * @param current_cycle Current simulation cycle
     * @return Refresh command or nullptr if none needed
     */
    std::unique_ptr<RefreshCommand> get_next_refresh(uint64_t current_cycle);
    
    /**
     * @brief Complete refresh operation
     * 
     * @param cmd Completed refresh command
     * @param completion_cycle Cycle when refresh finished
     */
    void complete_refresh(const RefreshCommand& cmd, uint64_t completion_cycle);
    
    /**
     * @brief Try to postpone refresh for performance
     * 
     * @param current_cycle Current simulation cycle
     * @return True if refresh was safely postponed
     */
    bool try_postpone_refresh(uint64_t current_cycle);
    
    // ========== Configuration Management ==========
    
    /**
     * @brief Set refresh mode
     * 
     * @param mode New refresh mode
     */
    void set_refresh_mode(RefreshMode mode);
    
    /**
     * @brief Get current refresh mode
     */
    RefreshMode get_refresh_mode() const { return current_mode_; }
    
    /**
     * @brief Update operating temperature
     * 
     * @param temp_celsius Temperature in Celsius
     */
    void update_temperature(double temp_celsius);
    
    /**
     * @brief Get current thermal range
     */
    ThermalRange get_thermal_range() const { return thermal_range_; }
    
    /**
     * @brief Get current refresh interval
     */
    uint64_t get_refresh_interval() const { return current_trefi_; }
    
    // ========== Retention Profiling (RAIDR) ==========
    
    /**
     * @brief Enable/disable retention profiling
     * 
     * @param enable True to enable RAIDR profiling
     */
    void enable_retention_profiling(bool enable);
    
    /**
     * @brief Update retention profile for a row
     * 
     * @param bank Bank ID
     * @param row Row address
     * @param retention_time Measured retention in cycles
     */
    void update_row_profile(uint32_t bank, uint32_t row, uint64_t retention_time);
    
    /**
     * @brief Get retention classification
     * 
     * @param bank Bank ID
     * @param row Row address
     * @return Retention class (STRONG/MEDIUM/WEAK)
     */
    RetentionClass get_retention_class(uint32_t bank, uint32_t row) const;
    
    /**
     * @brief Get retention statistics
     * 
     * @return RetentionStats struct with weak_rows and avg_retention
     */
    RetentionStats get_retention_stats() const;
    
    // ========== Power Optimization ==========
    
    /**
     * @brief Enable refresh clustering
     * 
     * @param enable True to enable clustering optimization
     */
    void enable_refresh_clustering(bool enable);
    
    /**
     * @brief Get refresh overhead percentage
     * 
     * @return Percentage of time spent in refresh
     */
    double get_refresh_overhead() const;
    
    // ========== Statistics and Monitoring ==========
    
    /**
     * @brief Get total number of refreshes executed
     */
    uint64_t get_total_refreshes() const { return total_refreshes_; }
    
    /**
     * @brief Get number of postponed refreshes
     */
    uint64_t get_postponed_refreshes() const { return postponed_refreshes_; }
    
    /**
     * @brief Get maximum postponement delay observed
     */
    uint64_t get_max_postponement_delay() const { return max_postponement_cycles_; }
    
    /**
     * @brief Reset all statistics
     */
    void reset_statistics();
    
    /**
     * @brief Export refresh trace for analysis
     * 
     * @param filename Output trace file path
     */
    void export_refresh_trace(const std::string& filename) const;

private:
    // ========== Timing and Configuration ==========
    const DRAMTiming& timing_;
    uint32_t num_banks_;
    uint32_t rows_per_bank_;
    RefreshMode current_mode_;
    ThermalRange thermal_range_;
    double current_temperature_;
    
    // ========== Timing Parameters ==========
    uint64_t base_trefi_;           ///< Base refresh interval (cycles)
    uint64_t current_trefi_;        ///< Current refresh interval
    uint64_t current_trfc_;         ///< Current refresh command time
    uint64_t max_postponement_;     ///< Maximum allowed postponement
    uint64_t cluster_window_;       ///< Clustering time window
    
    // ========== Per-Bank Refresh Tracking ==========
    struct BankRefreshState {
        uint64_t last_refresh_cycle;   ///< Last refresh timestamp
        uint64_t next_refresh_cycle;   ///< Next scheduled refresh
        uint32_t current_row;          ///< Current row counter for rolling refresh
        bool refresh_in_progress;      ///< True if refresh command active
        uint8_t postponement_count;    ///< Number of postponements
    };
    std::vector<BankRefreshState> bank_states_;
    
    // ========== Retention Profiling Data ==========
    bool retention_profiling_enabled_;
    std::vector<std::vector<RowRetentionProfile>> retention_profiles_; // [bank][row]
    
    // ========== Refresh Queue and Scheduling ==========
    std::priority_queue<RefreshCommand, std::vector<RefreshCommand>, 
                       RefreshPriorityComparator> refresh_queue_;
    
    // ========== Power Optimization ==========
    bool clustering_enabled_;       ///< Enable refresh clustering
    
    // ========== Statistics ==========
    uint64_t total_refreshes_;      ///< Total refresh operations executed
    uint64_t postponed_refreshes_;  ///< Number of postponed refreshes
    uint64_t max_postponement_cycles_; ///< Maximum postponement observed
    uint64_t total_refresh_cycles_; ///< Total cycles spent in refresh
    
    // ========== Private Helper Methods ==========
    
    /**
     * @brief Calculate refresh interval for temperature
     */
    uint64_t calculate_trefi(double temp_celsius) const;
    
    /**
     * @brief Calculate urgency level for refresh
     */
    uint8_t calculate_urgency(uint64_t current_cycle, uint64_t deadline_cycle) const;
    
    /**
     * @brief Generate refresh command for bank
     */
    RefreshCommand generate_refresh_command(uint32_t bank, uint64_t current_cycle);
    
    /**
     * @brief Update bank state after refresh
     */
    void update_bank_state(uint32_t bank, uint64_t completion_cycle);
    
    /**
     * @brief Check if refresh can be safely postponed
     */
    bool can_safely_postpone(uint32_t bank, uint64_t current_cycle) const;
};

} // namespace dram

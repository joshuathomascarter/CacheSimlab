/**
 * @file refresh_controller.h
 * @brief DRAM Refresh Controller - Architecture-Grade Implementation
 * 
 * Implements JEDEC DDR4 refresh modes with retention-aware optimizations
 * based on RAIDR (Liu et al., ISCA 2012) and Micron TN-40-46.
 * 
 * Supports:
 * - All-bank refresh (tRFC), per-bank refresh (tRFCpb)
 * - Fine-grained refresh (x2, x4 modes)
 * - Temperature-aware refresh interval scaling
 * - Postponement and urgency escalation
 * - Retention profiling for intelligent refresh scheduling
 * 
 * @author Josh Carter
 * @date January 2026
 * @standard DDR4 JESD79-4C compliant
 */

#ifndef DRAM_REFRESH_CONTROLLER_H
#define DRAM_REFRESH_CONTROLLER_H

#include "dram_timing.h"
#include <cstdint>
#include <vector>
#include <queue>
#include <memory>

namespace dram {

/**
 * @brief Refresh mode types per JEDEC DDR4 specification
 */
enum class RefreshMode {
    ALL_BANK,      ///< Standard all-bank refresh (tRFC = 260ns @ 8Gb)
    PER_BANK,      ///< Per-bank refresh (tRFCpb = 140ns @ 8Gb) 
    FINE_GRAIN_1X, ///< Normal refresh rate (tREFI = 7.8μs)
    FINE_GRAIN_2X, ///< 2x refresh rate (tREFI = 3.9μs)
    FINE_GRAIN_4X  ///< 4x refresh rate (tREFI = 1.95μs)
};

/**
 * @brief Temperature operating ranges per JEDEC specifications
 */
enum class ThermalRange {
    NORMAL,        ///< 0-85°C: Standard tREFI = 7.8μs
    EXTENDED_85C,  ///< 85-95°C: tREFI = 3.9μs (2x refresh rate)
    EXTENDED_95C   ///< >95°C: tREFI = 1.95μs (4x refresh rate)
};

/**
 * @brief Per-row retention profiling data
 * 
 * Tracks retention characteristics for retention-aware refresh
 * optimization as described in RAIDR paper.
 */
struct RowRetentionProfile {
    uint32_t row_address;           ///< Physical row address
    uint64_t min_retention_time;    ///< Minimum retention time (cycles)
    uint64_t last_refresh_cycle;    ///< Last refresh timestamp
    uint32_t weak_cell_count;       ///< Number of weak cells (fast leakage)
    uint8_t  retention_bin;         ///< Binned retention category (0-7)
    bool     requires_frequent_refresh; ///< True if below normal retention
};

/**
 * @brief Refresh command with scheduling metadata
 */
struct RefreshCommand {
    uint32_t bank_id;              ///< Target bank (ALL_BANKS = 0xFF)
    uint32_t row_start;            ///< Starting row address
    uint32_t row_count;            ///< Number of rows to refresh
    uint64_t deadline_cycle;       ///< Latest allowed refresh cycle
    uint64_t scheduled_cycle;      ///< Actual scheduled cycle
    uint8_t  urgency_level;        ///< Priority: 0=low, 255=critical
    RefreshMode mode;              ///< Refresh mode for this command
    
    bool is_overdue(uint64_t current_cycle) const {
        return current_cycle > deadline_cycle;
    }
};

/**
 * @brief Comprehensive refresh controller for DDR4 DRAM
 * 
 * Manages all aspects of DRAM refresh including:
 * - Multi-mode refresh (all-bank, per-bank, fine-grained)
 * - Temperature-aware interval scaling
 * - Postponement with urgency tracking
 * - Retention profiling and optimization
 * - Power-aware refresh clustering
 */
class RefreshController {
public:
    /**
     * @brief Construct refresh controller
     * @param timing DRAM timing parameters
     * @param num_banks Number of banks in the device
     * @param rows_per_bank Number of rows per bank
     */
    RefreshController(const DRAMTiming& timing, 
                     uint32_t num_banks, 
                     uint32_t rows_per_bank);
    
    ~RefreshController() = default;

    // ========== Core Refresh Operations ==========
    
    /**
     * @brief Check if refresh is needed at current cycle
     * @param current_cycle Current simulation cycle
     * @return true if any refresh is due or overdue
     */
    bool is_refresh_needed(uint64_t current_cycle) const;
    
    /**
     * @brief Get next refresh command to execute
     * @param current_cycle Current simulation cycle
     * @return RefreshCommand if refresh needed, nullptr otherwise
     */
    std::unique_ptr<RefreshCommand> get_next_refresh(uint64_t current_cycle);
    
    /**
     * @brief Record completion of a refresh command
     * @param cmd Completed refresh command
     * @param completion_cycle Cycle when refresh completed
     */
    void complete_refresh(const RefreshCommand& cmd, uint64_t completion_cycle);
    
    /**
     * @brief Attempt to postpone refresh to reduce interference
     * @param current_cycle Current simulation cycle
     * @return true if postponement successful, false if must refresh now
     */
    bool try_postpone_refresh(uint64_t current_cycle);
    
    // ========== Mode and Temperature Management ==========
    
    /**
     * @brief Update refresh mode (can switch dynamically)
     * @param mode New refresh mode
     */
    void set_refresh_mode(RefreshMode mode);
    
    /**
     * @brief Get current active refresh mode
     */
    RefreshMode get_refresh_mode() const { return current_mode_; }
    
    /**
     * @brief Update operating temperature
     * 
     * Automatically adjusts refresh interval per JEDEC specifications:
     * - 0-85°C: tREFI = 7.8μs (normal)
     * - 85-95°C: tREFI = 3.9μs (2x rate)
     * - >95°C: tREFI = 1.95μs (4x rate)
     * 
     * @param temp_celsius Junction temperature in °C
     */
    void update_temperature(double temp_celsius);
    
    /**
     * @brief Get current thermal range
     */
    ThermalRange get_thermal_range() const { return thermal_range_; }
    
    /**
     * @brief Get current refresh interval in cycles
     */
    uint64_t get_refresh_interval() const { return current_trefi_; }
    
    // ========== Retention Profiling (RAIDR-based) ==========
    
    /**
     * @brief Enable retention-aware intelligent refresh
     * 
     * Profiles rows to identify weak retention and adjusts
     * refresh scheduling accordingly (RAIDR algorithm).
     */
    void enable_retention_profiling(bool enable);
    
    /**
     * @brief Update retention profile for a row
     * @param bank Bank ID
     * @param row Row address
     * @param retention_time Measured retention time
     */
    void update_row_profile(uint32_t bank, uint32_t row, 
                           uint64_t retention_time);
    
    /**
     * @brief Get retention profile statistics
     * @param weak_rows Number of rows requiring frequent refresh
     * @param avg_retention Average retention time across all rows
     */
    void get_retention_stats(uint32_t& weak_rows, 
                            uint64_t& avg_retention) const;
    
    // ========== Power Optimization ==========
    
    /**
     * @brief Enable refresh clustering to minimize overhead
     * 
     * Groups refresh commands temporally to reduce total
     * power delivery stress and improve burst efficiency.
     */
    void enable_refresh_clustering(bool enable);
    
    /**
     * @brief Get refresh power overhead percentage
     * @return Percentage of time spent in refresh (0.0-100.0)
     */
    double get_refresh_overhead() const;
    
    // ========== Statistics and Monitoring ==========
    
    /**
     * @brief Get total number of refreshes issued
     */
    uint64_t get_total_refreshes() const { return total_refreshes_; }
    
    /**
     * @brief Get number of postponed refreshes
     */
    uint64_t get_postponed_count() const { return postponed_refreshes_; }
    
    /**
     * @brief Get maximum postponement delay observed
     */
    uint64_t get_max_postponement() const { return max_postponement_cycles_; }
    
    /**
     * @brief Reset all statistics counters
     */
    void reset_statistics();
    
    /**
     * @brief Export refresh trace for analysis
     * @param filename Output file path
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
    
    // ========== Timing Parameters (cycle counts) ==========
    uint64_t current_trefi_;        ///< Current refresh interval
    uint64_t base_trefi_;           ///< Base tREFI at normal temp (7.8μs)
    uint64_t current_trfc_;         ///< Current refresh command time
    uint64_t max_postponement_;     ///< Maximum allowed postponement
    
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
    struct RefreshPriorityComparator {
        bool operator()(const RefreshCommand& a, const RefreshCommand& b) const {
            if (a.urgency_level != b.urgency_level)
                return a.urgency_level < b.urgency_level;
            return a.deadline_cycle > b.deadline_cycle;
        }
    };
    std::priority_queue<RefreshCommand, 
                       std::vector<RefreshCommand>,
                       RefreshPriorityComparator> refresh_queue_;
    
    // ========== Power Optimization ==========
    bool clustering_enabled_;
    uint64_t cluster_window_;       ///< Cycles to cluster refreshes within
    
    // ========== Statistics ==========
    uint64_t total_refreshes_;
    uint64_t postponed_refreshes_;
    uint64_t max_postponement_cycles_;
    uint64_t total_refresh_cycles_;  ///< Total cycles spent in refresh
    
    // ========== Private Helper Methods ==========
    
    /**
     * @brief Calculate refresh interval for current temperature
     */
    uint64_t calculate_trefi(double temp_celsius) const;
    
    /**
     * @brief Calculate urgency level for a refresh command
     */
    uint8_t calculate_urgency(uint64_t current_cycle, 
                             uint64_t deadline_cycle) const;
    
    /**
     * @brief Generate refresh command for bank
     */
    RefreshCommand generate_refresh_command(uint32_t bank, 
                                           uint64_t current_cycle);
    
    /**
     * @brief Update bank state after refresh
     */
    void update_bank_state(uint32_t bank, uint64_t completion_cycle);
    
    /**
     * @brief Check if postponement is safe
     */
    bool can_safely_postpone(uint32_t bank, uint64_t current_cycle) const;
};

} // namespace dram

#endif // DRAM_REFRESH_CONTROLLER_H

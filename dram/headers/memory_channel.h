#ifndef MEMORY_CHANNEL_H
#define MEMORY_CHANNEL_H

#include <vector>
#include <queue>
#include <memory>
#include <cstdint>
#include "dram_bank_fsm.h"
#include "dram_timing.h"
#include "timing_validator.h"

namespace dram {

// Forward declarations and type definitions
enum class CommandType {
    NOP,
    ACTIVATE,
    READ,
    WRITE,
    PRECHARGE,
    REFRESH
};

using DRAMBank = DRAMBankFSM;
using TimingParameters = DRAMTiming;

// Speed grade enum for convenience
enum class SpeedGrade {
    DDR4_2400
};

inline TimingParameters create_timing_parameters(SpeedGrade grade) {
    switch (grade) {
        case SpeedGrade::DDR4_2400:
            return DRAMTiming::DDR4_2400_R();
        default:
            return DRAMTiming::DDR4_2400_R();
    }
}

// Address mapping schemes for different workload optimizations
enum class AddressMappingScheme {
    ROW_BANK_COLUMN,           // Sequential mapping (poor BLP)
    BANK_ROW_COLUMN,           // Interleaved mapping (good BLP for streaming)
    XOR_INTERLEAVED,           // XOR-based for random access
    CACHE_LINE_INTERLEAVED     // Every cache line to different bank
};

// Bank group configuration for DDR4
struct BankGroupConfig {
    uint32_t num_banks_per_group;
    uint32_t num_bank_groups;
    uint32_t total_banks;
    
    // Bank group timing parameters
    uint32_t tCCD_S;  // CAS to CAS delay, same bank group (4 cycles)
    uint32_t tCCD_L;  // CAS to CAS delay, different bank group (6 cycles)
    uint32_t tRRD_S;  // Row activate to row activate, same group (4 cycles)
    uint32_t tRRD_L;  // Row activate to row activate, different group (6 cycles)
    uint32_t tWTR_S;  // Write to read, same bank group (2 cycles)
    uint32_t tWTR_L;  // Write to read, different bank group (6 cycles)
};

// 3D stacking configuration (HBM)
struct TSVTiming {
    uint32_t tTSV;           // TSV transfer delay (1-2 cycles)
    uint32_t tDIE_SWITCH;    // Die switching overhead (2-3 cycles)
};

struct ThermalConfig {
    double base_temp;         // Base temperature (Celsius)
    double die_temp_delta;    // Temperature increase per die (10-15°C)
    double throttle_temp;     // Temperature to start throttling (85°C)
    double critical_temp;     // Critical temperature (95°C)
};

// Per-die state tracking
struct DieState {
    uint32_t die_id;
    double current_temp;      // Current temperature
    uint64_t last_access;     // Last access cycle
    uint64_t total_accesses;  // Total accesses to this die
    bool throttled;           // Is die thermally throttled?
    std::vector<uint32_t> bank_ids;  // Banks on this die
};

// Physical address breakdown
struct PhysicalAddress {
    uint64_t raw_address;
    uint32_t column;
    uint32_t row;
    uint32_t bank;
    uint32_t bank_group;
    uint32_t rank;
    uint32_t channel;
    
    PhysicalAddress() : raw_address(0), column(0), row(0), 
                        bank(0), bank_group(0), rank(0), channel(0) {}
};

// Command bus entry
struct BusCommand {
    CommandType type;
    uint32_t bank_id;
    uint32_t bank_group_id;
    uint32_t row;
    uint64_t cycle_issued;
    uint64_t request_id;
    
    BusCommand() : type(CommandType::NOP), bank_id(0), bank_group_id(0),
                   row(0), cycle_issued(0), request_id(0) {}
};

// Bank-level parallelism statistics
struct BLPStatistics {
    uint64_t total_cycles;
    uint64_t total_active_banks;
    std::vector<uint64_t> active_bank_histogram;  // Histogram of active banks per cycle
    double average_blp;
    double peak_blp;
    uint64_t bank_conflicts;
    
    BLPStatistics() : total_cycles(0), total_active_banks(0), 
                      average_blp(0.0), peak_blp(0.0), bank_conflicts(0) {}
    
    void update(uint32_t num_active_banks) {
        total_cycles++;
        total_active_banks += num_active_banks;
        if (active_bank_histogram.size() < num_active_banks + 1) {
            active_bank_histogram.resize(num_active_banks + 1, 0);
        }
        active_bank_histogram[num_active_banks]++;
        average_blp = static_cast<double>(total_active_banks) / total_cycles;
        peak_blp = std::max(peak_blp, static_cast<double>(num_active_banks));
    }
};

// Memory channel coordinating multiple banks
class MemoryChannel {
private:
    // Bank organization
    std::vector<std::unique_ptr<DRAMBank>> banks_;
    BankGroupConfig bank_config_;
    TimingParameters timing_params_;
    std::unique_ptr<TimingValidator> timing_validator_;
    
    // Address mapping
    AddressMappingScheme mapping_scheme_;
    uint32_t column_bits_;
    uint32_t row_bits_;
    uint32_t bank_bits_;
    uint32_t bank_group_bits_;
    uint32_t rank_bits_;
    
    // Command bus (only one command per cycle)
    std::queue<BusCommand> command_bus_queue_;
    BusCommand last_command_;
    uint64_t current_cycle_;
    
    // Bank group timing tracking
    std::vector<uint64_t> last_cas_per_group_;
    std::vector<uint64_t> last_activate_per_group_;
    std::vector<uint64_t> last_write_per_group_;
    
    // Statistics
    BLPStatistics blp_stats_;
    uint64_t total_commands_;
    uint64_t bus_utilization_cycles_;
    
    // 3D-stacked architecture support
    bool is_3d_stacked_;
    uint32_t num_dies_;
    uint32_t die_bits_;
    uint32_t current_die_;           // Last accessed die
    uint64_t last_die_switch_cycle_; // For tDIE_SWITCH timing
    std::vector<DieState> dies_;
    TSVTiming tsv_timing_;
    ThermalConfig thermal_config_;
    
    // Helper functions
    uint32_t get_bank_group(uint32_t bank_id) const;
    bool check_bank_group_timing(const BusCommand& cmd);
    void update_bank_group_timing(const BusCommand& cmd);
    
public:
    MemoryChannel(const TimingParameters& timing, 
                  const BankGroupConfig& config,
                  AddressMappingScheme scheme = AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    
    ~MemoryChannel() = default;
    
    // Address mapping functions
    PhysicalAddress decode_address(uint64_t address) const;
    uint64_t encode_address(const PhysicalAddress& addr) const;
    void set_mapping_scheme(AddressMappingScheme scheme);
    
    // Command issuing
    bool can_issue_command(CommandType type, uint32_t bank_id, uint32_t row);
    bool issue_command(CommandType type, uint32_t bank_id, uint32_t row, uint64_t request_id);
    
    // Simulation
    void tick();
    void advance_cycle(uint64_t cycles = 1);
    uint64_t get_current_cycle() const { return current_cycle_; }
    
    // Bank queries
    BankState get_bank_state(uint32_t bank_id) const;
    uint32_t get_open_row(uint32_t bank_id) const;
    bool is_row_hit(uint32_t bank_id, uint32_t row) const;
    uint32_t get_num_banks() const { return bank_config_.total_banks; }
    
    // Statistics
    const BLPStatistics& get_blp_statistics() const { return blp_stats_; }
    double get_bus_utilization() const;
    uint64_t get_bank_conflicts() const { return blp_stats_.bank_conflicts; }
    void reset_statistics();
    
    // 3D-stacked support
    void enable_3d_stacking(uint32_t num_dies);
    bool is_3d_stacked() const { return is_3d_stacked_; }
    
    // Debug
    void print_bank_status() const;
    void export_trace(const std::string& filename) const;
};

// Helper function to create standard DDR4 configurations
BankGroupConfig create_ddr4_config(uint32_t num_bank_groups = 4, 
                                   uint32_t banks_per_group = 4);

// Address mapping helper functions
namespace address_mapping {
    // Calculate optimal bit positions for different mapping schemes
    struct BitLayout {
        uint32_t column_start, column_end;
        uint32_t bank_start, bank_end;
        uint32_t bank_group_start, bank_group_end;
        uint32_t row_start, row_end;
        uint32_t rank_start, rank_end;
    };
    
    BitLayout calculate_layout(AddressMappingScheme scheme,
                              uint32_t column_bits,
                              uint32_t row_bits,
                              uint32_t bank_bits,
                              uint32_t bank_group_bits,
                              uint32_t rank_bits = 1);
    
    // XOR-based interleaving for random access patterns
    uint32_t xor_hash_bank(uint64_t address, uint32_t num_banks);
    uint32_t xor_hash_bank_group(uint64_t address, uint32_t num_groups);
}

} // namespace dram

#endif // MEMORY_CHANNEL_H

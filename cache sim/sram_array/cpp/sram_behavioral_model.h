#ifndef SRAM_BEHAVIORAL_MODEL_H
#define SRAM_BEHAVIORAL_MODEL_H

#include <cstdint>
#include <vector>
#include <string>

// ============================================================================
// SRAM Configuration Constants (MOVED FROM .cpp)
// ============================================================================

namespace SRAM {
    // Architecture
    constexpr int DEPTH = 65536;           // 64K entries (16-bit address)
    constexpr int WIDTH = 32;              // 32-bit data width
    constexpr int ADDRESS_WIDTH = 16;      // 16-bit address bus
    
    // Physical architecture
    constexpr int NUM_BANKS = 4;           // 4 byte-wide banks
    constexpr int BITS_PER_BANK = 8;       // 8 bits per bank (1 byte)
    constexpr int ROWS_PER_BANK = 256;     // 256 rows per bank
    constexpr int COLS_PER_BANK = 256;     // 256 columns per bank
    
    // Area per cell (130nm SkyWater PDK)
    constexpr double CELL_SIZE_UM2 = 0.57; // 6T cell @ 130nm
    constexpr int TRANSISTORS_PER_CELL = 6;
    
    // Timing parameters (based on 130nm SkyWater PDK)
    constexpr double CLOCK_PERIOD_NS = 8.0;         // 125 MHz
    constexpr double READ_ACCESS_TIME_NS = 2.5;     // tAA - read access time
    constexpr double WRITE_CYCLE_TIME_NS = 3.0;     // tWC - write cycle time
    constexpr double PRECHARGE_TIME_NS = 1.5;       // Precharge pulse width
    constexpr double WORDLINE_DELAY_NS = 0.5;       // Wordline propagation
    constexpr double SENSE_AMP_DELAY_NS = 0.8;      // Sense amp settling
    
    // Power parameters
    constexpr double LEAKAGE_PER_CELL_UW = 0.02;    // 6T × subthreshold leakage
    constexpr double BITLINE_SWING_ENERGY_PJ = 2.5; // Per read/write
    constexpr double SENSE_AMP_ENERGY_PJ = 1.2;     // Per sense operation
}

// ============================================================================
// Structs (already defined before)
// ============================================================================

struct SRAMTimingSpec {
    double read_access_time_ns;
    double write_cycle_time_ns;
    double setup_time_ns;
    double hold_time_ns;
    double clock_period_ns;
    
    SRAMTimingSpec() :
        read_access_time_ns(SRAM::READ_ACCESS_TIME_NS),  // ← Use namespace constants
        write_cycle_time_ns(SRAM::WRITE_CYCLE_TIME_NS),
        setup_time_ns(0.5),
        hold_time_ns(0.3),
        clock_period_ns(SRAM::CLOCK_PERIOD_NS)
    {}
};

struct AreaEstimate {
    double cell_area_um2;
    double array_area_um2;
    double decoder_area_um2;
    double sense_amp_area_um2;
    double routing_area_um2;
    double total_area_um2;
    
    AreaEstimate() : 
        cell_area_um2(SRAM::CELL_SIZE_UM2),  // ← Use namespace constant
        array_area_um2(0), decoder_area_um2(0),
        sense_amp_area_um2(0), routing_area_um2(0), total_area_um2(0)
    {}
};

struct PowerEstimate {
    double read_energy_pj;
    double write_energy_pj;
    double leakage_power_uw;
    double avg_dynamic_power_mw;
    
    PowerEstimate() :
        read_energy_pj(0), write_energy_pj(0),
        leakage_power_uw(0), avg_dynamic_power_mw(0)
    {}
};

struct AccessResult {
    int cycle;
    bool is_write;
    uint16_t address;
    uint32_t data;
    bool hit;
    double latency_ns;
    
    AccessResult() : 
        cycle(0), is_write(false), address(0), 
        data(0), hit(false), latency_ns(0.0) 
    {}
};

// ============================================================================
// SRAM Behavioral Model Class
// ============================================================================

class SRAMBehavioralModel {
private:
    std::vector<uint8_t> bank0;
    std::vector<uint8_t> bank1;
    std::vector<uint8_t> bank2;
    std::vector<uint8_t> bank3;
    std::vector<bool> valid_bits;
    
    SRAMTimingSpec timing_spec;
    
    uint64_t read_count;
    uint64_t write_count;
    uint64_t read_hits;
    uint64_t read_misses;
    int current_cycle;
    
public:
    // Single constructor with default parameter (eliminates duplication)
    SRAMBehavioralModel(const SRAMTimingSpec& spec = SRAMTimingSpec());
    
    AccessResult read(uint16_t address);
    AccessResult write(uint16_t address, uint32_t data);
    
    uint32_t read_data(uint16_t address) const;
    bool is_valid(uint16_t address) const;
    
    double get_read_latency_ns() const { return timing_spec.read_access_time_ns; }
    double get_write_latency_ns() const { return timing_spec.write_cycle_time_ns; }
    int get_read_latency_cycles() const;
    int get_write_latency_cycles() const;
    
    AreaEstimate estimate_area() const;
    PowerEstimate estimate_power(double activity_factor = 0.1) const;
    
    uint64_t get_read_count() const { return read_count; }
    uint64_t get_write_count() const { return write_count; }
    uint64_t get_read_hits() const { return read_hits; }
    uint64_t get_read_misses() const { return read_misses; }
    double get_hit_rate() const;
    
    void reset_statistics();
    void print_statistics() const;
    
    void advance_cycle() { current_cycle++; }
    int get_current_cycle() const { return current_cycle; }
    void reset_cycle() { current_cycle = 0; }
};

#endif // SRAM_BEHAVIORAL_MODEL_H
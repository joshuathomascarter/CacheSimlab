/**
 * SRAM Behavioral Model - 6T SRAM Array Simulator
 * Predicts timing, area, and power for SRAM implementations
 * Golden reference for RTL cross-validation
 * 
 * THIS IS THE SINGLE SOURCE OF TRUTH FOR SRAM BEHAVIOR
 * All testbenches should link against this, not duplicate the logic.
 * 
 * Author: Josh Carter (Memory Architect Path)
 * Target: Apple-grade memory system verification
 */

#include "sram_behavioral_model.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>

// ============================================================================
// Constructor (single version with default parameter)
// ============================================================================

SRAMBehavioralModel::SRAMBehavioralModel(const SRAMTimingSpec& spec)
    : bank0(SRAM::DEPTH, 0),      // Initialize 4 byte-wide banks
      bank1(SRAM::DEPTH, 0),
      bank2(SRAM::DEPTH, 0),
      bank3(SRAM::DEPTH, 0),
      valid_bits(SRAM::DEPTH, false),
      timing_spec(spec),          // Use provided or default timing
      read_count(0), 
      write_count(0),
      read_hits(0), 
      read_misses(0),
      current_cycle(0)
{
    // All initialization done in member initializer list
    // No code duplication!
}

// ============================================================================
// Core Operations
// ============================================================================

AccessResult SRAMBehavioralModel::read(uint16_t address) {
    assert(address < SRAM::DEPTH);  // ← Use constant from header
    
    AccessResult result;
    result.cycle = current_cycle;
    result.is_write = false;
    result.address = address;
    
    // Read from all 4 banks
    uint8_t byte0 = bank0[address];
    uint8_t byte1 = bank1[address];
    uint8_t byte2 = bank2[address];
    uint8_t byte3 = bank3[address];
    
    result.data = ((uint32_t)byte3 << 24) |
                  ((uint32_t)byte2 << 16) |
                  ((uint32_t)byte1 << 8)  |
                  ((uint32_t)byte0 << 0);
    
    result.hit = valid_bits[address];
    result.latency_ns = timing_spec.read_access_time_ns;
    
    read_count++;
    if (result.hit) {
        read_hits++;
    } else {
        read_misses++;
    }
    
    current_cycle++;
    return result;
}

AccessResult SRAMBehavioralModel::write(uint16_t address, uint32_t data) {
    assert(address < SRAM::DEPTH);  // ← Use constant from header
    
    AccessResult result;
    result.cycle = current_cycle;
    result.is_write = true;
    result.address = address;
    result.data = data;
    result.hit = true;  // Writes always succeed
    result.latency_ns = timing_spec.write_cycle_time_ns;
    
    // Write to all 4 banks
    bank0[address] = (uint8_t)(data >> 0);
    bank1[address] = (uint8_t)(data >> 8);
    bank2[address] = (uint8_t)(data >> 16);
    bank3[address] = (uint8_t)(data >> 24);
    valid_bits[address] = true;
    
    write_count++;
    current_cycle++;
    return result;
}

uint32_t SRAMBehavioralModel::read_data(uint16_t address) const {
    assert(address < SRAM::DEPTH);
    
    if (!valid_bits[address]) {
        return 0;  // Invalid data
    }
    
    uint32_t data = ((uint32_t)bank3[address] << 24) |
                    ((uint32_t)bank2[address] << 16) |
                    ((uint32_t)bank1[address] << 8)  |
                    ((uint32_t)bank0[address] << 0);
    return data;
}

bool SRAMBehavioralModel::is_valid(uint16_t address) const {
    assert(address < SRAM::DEPTH);
    return valid_bits[address];
}

// ============================================================================
// Latency Helpers
// ============================================================================

int SRAMBehavioralModel::get_read_latency_cycles() const {
    // Convert ns to cycles: latency_ns / clock_period_ns, rounded up
    return (int)std::ceil(timing_spec.read_access_time_ns / timing_spec.clock_period_ns);
}

int SRAMBehavioralModel::get_write_latency_cycles() const {
    return (int)std::ceil(timing_spec.write_cycle_time_ns / timing_spec.clock_period_ns);
}

// ============================================================================
// Area and Power Estimation
// ============================================================================

AreaEstimate SRAMBehavioralModel::estimate_area() const {
    AreaEstimate area;
    
    // 6T cell @ 0.57 μm² per cell (130nm SkyWater)
    int total_cells = SRAM::DEPTH * SRAM::NUM_BANKS;
    area.cell_area_um2 = total_cells * SRAM::CELL_SIZE_UM2;
    
    // Decoder area (logarithmic): ~20% overhead
    area.decoder_area_um2 = area.cell_area_um2 * 0.20;
    
    // Sense amps: one per bit line, ~20% overhead
    area.sense_amp_area_um2 = area.cell_area_um2 * 0.20;
    
    // Write drivers: ~15% overhead
    int bits_total = SRAM::DEPTH * SRAM::NUM_BANKS * SRAM::BITS_PER_BANK;
    area.routing_area_um2 = area.cell_area_um2 * 0.15;
    
    area.total_area_um2 = area.cell_area_um2 + 
                          area.decoder_area_um2 + 
                          area.sense_amp_area_um2 + 
                          area.routing_area_um2;
    
    area.array_area_um2 = area.cell_area_um2;
    
    return area;
}

PowerEstimate SRAMBehavioralModel::estimate_power(double activity_factor) const {
    PowerEstimate power;
    
    // Leakage: 6 transistors @ 0.02 μW per cell
    int total_cells = SRAM::DEPTH * SRAM::NUM_BANKS;
    power.leakage_power_uw = total_cells * SRAM::LEAKAGE_PER_CELL_UW;
    
    // Dynamic: bitline swing + sense amp energy per access
    double read_energy = SRAM::BITLINE_SWING_ENERGY_PJ + SRAM::SENSE_AMP_ENERGY_PJ;
    double write_energy = SRAM::BITLINE_SWING_ENERGY_PJ;
    
    power.read_energy_pj = read_energy;
    power.write_energy_pj = write_energy;
    
    // Average dynamic power @ 125MHz (8ns cycle)
    // Assume 50% reads, 50% writes
    double avg_energy = (read_energy + write_energy) / 2.0;
    double freq_ghz = 1000.0 / SRAM::CLOCK_PERIOD_NS / 1000.0;  // Convert to GHz
    power.avg_dynamic_power_mw = avg_energy * freq_ghz * activity_factor;
    
    return power;
}

// ============================================================================
// Statistics
// ============================================================================

double SRAMBehavioralModel::get_hit_rate() const {
    return read_count > 0 ? (double)read_hits / read_count : 0.0;
}

void SRAMBehavioralModel::reset_statistics() {
    read_count = 0;
    write_count = 0;
    read_hits = 0;
    read_misses = 0;
    current_cycle = 0;
}

void SRAMBehavioralModel::print_statistics() const {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "SRAM BEHAVIORAL MODEL STATISTICS\n";
    std::cout << std::string(70, '=') << "\n\n";
    
    std::cout << "Configuration:\n";
    std::cout << "  Capacity:        " << SRAM::DEPTH << " words x " 
              << SRAM::WIDTH << " bits\n";
    std::cout << "  Architecture:    " << SRAM::NUM_BANKS << " banks\n";
    std::cout << "  Cell Size:       " << SRAM::CELL_SIZE_UM2 << " um²\n\n";
    
    std::cout << "Access Statistics:\n";
    std::cout << "  Total Reads:     " << read_count << "\n";
    std::cout << "  Total Writes:    " << write_count << "\n";
    if (read_count > 0) {
        std::cout << "  Read Hits:       " << read_hits << " (" 
                  << std::fixed << std::setprecision(1) << (get_hit_rate() * 100) 
                  << "%)\n";
    }
    std::cout << "  Total Cycles:    " << current_cycle << "\n";
    
    std::cout << "\nTiming:\n";
    std::cout << "  Read Access:     " << std::fixed << std::setprecision(2)
              << timing_spec.read_access_time_ns << " ns (" 
              << get_read_latency_cycles() << " cycles)\n";
    std::cout << "  Write Cycle:     " << timing_spec.write_cycle_time_ns << " ns ("
              << get_write_latency_cycles() << " cycles)\n";
    
    std::cout << std::string(70, '=') << "\n\n";
}

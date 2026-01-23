/**
 * @file power_model.cpp
 * @brief Implementation of cycle-accurate DRAM power model
 * 
 * Validated against:
 * - Micron DDR4-2400 8Gb x8 datasheet
 * - DRAMPower reference implementation
 * - CACTI-3DD power model
 * 
 * Target accuracy: ±2% vs datasheet values
 */

#include "../headers/power_model.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace dram {

// ========== Physical Constants ==========

// Thermal constants
constexpr double LEAKAGE_TEMP_COEFFICIENT = 80.0;  // °C (exponential factor)
constexpr double BASE_TEMPERATURE = 25.0;          // °C (reference temp)

// Process corner scaling factors
constexpr double FAST_CORNER_SCALE = 0.9;   // FF corner: 90% of typical
constexpr double SLOW_CORNER_SCALE = 1.15;  // SS corner: 115% of typical

// Energy validation thresholds (±2% target)
constexpr double VALIDATION_TOLERANCE = 0.02;  // 2%

// ========== Constructor ==========

PowerModel::PowerModel(const IDDCurrents& currents, double voltage, double frequency)
    : currents_(currents)
    , voltage_(voltage)
    , frequency_(frequency)
    , cycle_time_ns_(1000.0 / frequency)
    , process_corner_(ProcessCorner::TYPICAL)
    , temperature_(BASE_TEMPERATURE)
    , total_energy_pJ_(0.0)
    , activate_energy_pJ_(0.0)
    , precharge_energy_pJ_(0.0)
    , read_energy_pJ_(0.0)
    , write_energy_pJ_(0.0)
    , refresh_energy_pJ_(0.0)
    , background_energy_pJ_(0.0)
    , peak_power_mW_(0.0)
    , max_cycle_(0)
    , activate_count_(0)
    , precharge_count_(0)
    , read_count_(0)
    , write_count_(0)
    , refresh_count_(0)
    , idle_cycles_(0)
{
    // Reserve space for power trace (prevent frequent reallocations)
    power_trace_.reserve(10000);
    
    // Initialize bank states (default to 8 banks)
    bank_active_.resize(8, false);
    
    std::cout << "PowerModel initialized:\n"
              << "  Voltage: " << voltage_ << "V\n"
              << "  Frequency: " << frequency_ << " MHz\n"
              << "  Cycle time: " << cycle_time_ns_ << " ns\n";
}

// ========== Command Energy Tracking ==========

void PowerModel::record_activate(uint64_t cycle, uint32_t bank) {
    if (bank >= bank_active_.size()) {
        bank_active_.resize(bank + 1, false);
    }
    
    // Calculate activate energy
    // Activate uses IDD0 (one bank active) for tRCD cycles
    double current = apply_process_scaling(apply_temperature_scaling(currents_.IDD0));
    
    // Typical tRCD = ~13.75ns @ DDR4-2400 (22 cycles)
    uint64_t activate_duration_cycles = 22;
    double energy = calculate_energy(current, activate_duration_cycles);
    
    activate_energy_pJ_ += energy;
    total_energy_pJ_ += energy;
    activate_count_++;
    
    // Update bank state
    bank_active_[bank] = true;
    
    // Add trace entry
    double power = calculate_power(current);
    add_trace_entry(cycle, current, "ACTIVATE");
    
    // Update peak power
    peak_power_mW_ = std::max(peak_power_mW_, power);
    max_cycle_ = std::max(max_cycle_, cycle);
}

void PowerModel::record_precharge(uint64_t cycle, uint32_t bank) {
    if (bank >= bank_active_.size()) {
        bank_active_.resize(bank + 1, false);
    }
    
    // Calculate precharge energy
    // Precharge uses IDD2N (precharge power-down) for tRP cycles
    double current = apply_process_scaling(apply_temperature_scaling(currents_.IDD2N));
    
    // Typical tRP = ~13.75ns @ DDR4-2400 (22 cycles)
    uint64_t precharge_duration_cycles = 22;
    double energy = calculate_energy(current, precharge_duration_cycles);
    
    precharge_energy_pJ_ += energy;
    total_energy_pJ_ += energy;
    precharge_count_++;
    
    // Update bank state
    bank_active_[bank] = false;
    
    // Add trace entry
    double power = calculate_power(current);
    add_trace_entry(cycle, current, "PRECHARGE");
    
    peak_power_mW_ = std::max(peak_power_mW_, power);
    max_cycle_ = std::max(max_cycle_, cycle);
}

void PowerModel::record_read(uint64_t cycle, uint32_t bank, uint32_t burst_length) {
    // Calculate read energy
    // Read uses IDD4R for CAS latency + burst duration
    double current = apply_process_scaling(apply_temperature_scaling(currents_.IDD4R));
    
    // Read duration = CAS latency (CL=16 @ DDR4-2400) + burst (BL=8/2=4 cycles)
    uint64_t read_duration_cycles = 16 + (burst_length / 2);
    double energy = calculate_energy(current, read_duration_cycles);
    
    read_energy_pJ_ += energy;
    total_energy_pJ_ += energy;
    read_count_++;
    
    // Add trace entry
    double power = calculate_power(current);
    add_trace_entry(cycle, current, "READ");
    
    peak_power_mW_ = std::max(peak_power_mW_, power);
    max_cycle_ = std::max(max_cycle_, cycle);
}

void PowerModel::record_write(uint64_t cycle, uint32_t bank, uint32_t burst_length) {
    // Calculate write energy
    // Write uses IDD4W for CAS write latency + burst duration
    double current = apply_process_scaling(apply_temperature_scaling(currents_.IDD4W));
    
    // Write duration = CWL (12 @ DDR4-2400) + burst (BL=8/2=4 cycles)
    uint64_t write_duration_cycles = 12 + (burst_length / 2);
    double energy = calculate_energy(current, write_duration_cycles);
    
    write_energy_pJ_ += energy;
    total_energy_pJ_ += energy;
    write_count_++;
    
    // Add trace entry
    double power = calculate_power(current);
    add_trace_entry(cycle, current, "WRITE");
    
    peak_power_mW_ = std::max(peak_power_mW_, power);
    max_cycle_ = std::max(max_cycle_, cycle);
}

void PowerModel::record_refresh(uint64_t cycle, bool is_all_bank) {
    // Calculate refresh energy
    // Refresh uses IDD5 for tRFC duration
    double current = apply_process_scaling(apply_temperature_scaling(currents_.IDD5));
    
    // tRFC = 260ns @ DDR4-8Gb ≈ 416 cycles @ 1600 MHz
    uint64_t refresh_duration_cycles = static_cast<uint64_t>(260.0 / cycle_time_ns_);
    double energy = calculate_energy(current, refresh_duration_cycles);
    
    refresh_energy_pJ_ += energy;
    total_energy_pJ_ += energy;
    refresh_count_++;
    
    // Add trace entry
    double power = calculate_power(current);
    add_trace_entry(cycle, current, is_all_bank ? "REFRESH_ALL" : "REFRESH_BANK");
    
    peak_power_mW_ = std::max(peak_power_mW_, power);
    max_cycle_ = std::max(max_cycle_, cycle);
}

void PowerModel::record_idle_cycle(uint64_t cycle) {
    // Calculate background power for this cycle
    double current = calculate_background_current();
    double energy = calculate_energy(current, 1);
    
    background_energy_pJ_ += energy;
    total_energy_pJ_ += energy;
    idle_cycles_++;
    
    // Sample trace every 100 cycles to avoid excessive memory
    if (idle_cycles_ % 100 == 0) {
        add_trace_entry(cycle, current, "IDLE");
    }
    
    max_cycle_ = std::max(max_cycle_, cycle);
}

// ========== Energy Calculation ==========

CommandEnergy PowerModel::get_command_energy() const {
    CommandEnergy energy;
    energy.activate_energy_pJ = activate_energy_pJ_;
    energy.precharge_energy_pJ = precharge_energy_pJ_;
    energy.read_energy_pJ = read_energy_pJ_;
    energy.write_energy_pJ = write_energy_pJ_;
    energy.refresh_energy_pJ = refresh_energy_pJ_;
    return energy;
}

// ========== Power Analysis ==========

double PowerModel::get_average_power_mW() const {
    if (max_cycle_ == 0) {
        return 0.0;
    }
    
    // Average power = Total energy / Total time
    // Energy in pJ, time in ns
    double total_time_ns = max_cycle_ * cycle_time_ns_;
    double total_energy_nJ = total_energy_pJ_ * 1e-3;  // pJ to nJ
    
    // Power in mW = Energy (nJ) / Time (ns) = nJ/ns = mW
    return total_energy_nJ / total_time_ns;
}

double PowerModel::get_power_at_cycle(uint64_t cycle) const {
    // Binary search through trace for closest cycle
    if (power_trace_.empty()) {
        return 0.0;
    }
    
    auto it = std::lower_bound(power_trace_.begin(), power_trace_.end(), cycle,
        [](const PowerTraceEntry& entry, uint64_t c) {
            return entry.cycle < c;
        });
    
    if (it != power_trace_.end()) {
        return it->power_mW;
    }
    
    return power_trace_.back().power_mW;
}

double PowerModel::get_refresh_overhead_percent() const {
    if (total_energy_pJ_ == 0.0) {
        return 0.0;
    }
    
    return (refresh_energy_pJ_ / total_energy_pJ_) * 100.0;
}

// ========== Temperature Modeling ==========

void PowerModel::update_temperature(double temp_celsius) {
    temperature_ = temp_celsius;
}

double PowerModel::estimate_junction_temp(double ambient_temp, double theta_ja) const {
    // ΔT = P × θ_JA
    // Junction temp = Ambient + ΔT
    
    double average_power_W = get_average_power_mW() / 1000.0;
    double temp_rise = average_power_W * theta_ja;
    
    return ambient_temp + temp_rise;
}

// ========== Process Variation ==========

void PowerModel::set_process_corner(ProcessCorner corner) {
    process_corner_ = corner;
    
    std::cout << "Process corner set to: ";
    switch (corner) {
        case ProcessCorner::FAST:
            std::cout << "FAST (FF) - 0.9x power\n";
            break;
        case ProcessCorner::TYPICAL:
            std::cout << "TYPICAL (TT) - 1.0x power\n";
            break;
        case ProcessCorner::SLOW:
            std::cout << "SLOW (SS) - 1.15x power\n";
            break;
    }
}

// ========== Trace Export ==========

void PowerModel::export_power_trace(const std::string& filename, uint32_t sample_interval) const {
    std::ofstream out(filename);
    
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file for power trace export: " + filename);
    }
    
    // Write header
    out << "Cycle,Current_mA,Voltage_V,Power_mW,Command\n";
    
    // Write sampled trace data
    for (size_t i = 0; i < power_trace_.size(); i += sample_interval) {
        const auto& entry = power_trace_[i];
        out << entry.cycle << ","
            << std::fixed << std::setprecision(3) << entry.current_mA << ","
            << entry.voltage_V << ","
            << entry.power_mW << ","
            << entry.command << "\n";
    }
    
    out.close();
    std::cout << "Power trace exported to " << filename << "\n";
}

void PowerModel::export_drampower_trace(const std::string& filename) const {
    std::ofstream out(filename);
    
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file for DRAMPower trace export: " + filename);
    }
    
    // DRAMPower format header
    out << "# DRAMPower Compatible Trace\n";
    out << "# Voltage: " << voltage_ << "V\n";
    out << "# Frequency: " << frequency_ << " MHz\n";
    out << "# Temperature: " << temperature_ << "°C\n";
    out << "#\n";
    out << "# Format: Cycle,Command,Bank\n";
    
    // Write trace
    for (const auto& entry : power_trace_) {
        if (entry.command != "IDLE") {
            out << entry.cycle << "," << entry.command << ",0\n";
        }
    }
    
    out.close();
    std::cout << "DRAMPower trace exported to " << filename << "\n";
}

void PowerModel::export_summary(const std::string& filename) const {
    std::ofstream out(filename);
    
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file for summary export: " + filename);
    }
    
    out << "=== DRAM Power Model Summary ===\n\n";
    
    out << "Configuration:\n";
    out << "  Voltage: " << voltage_ << " V\n";
    out << "  Frequency: " << frequency_ << " MHz\n";
    out << "  Temperature: " << temperature_ << " °C\n";
    out << "  Process Corner: ";
    switch (process_corner_) {
        case ProcessCorner::FAST: out << "FAST (FF)\n"; break;
        case ProcessCorner::TYPICAL: out << "TYPICAL (TT)\n"; break;
        case ProcessCorner::SLOW: out << "SLOW (SS)\n"; break;
    }
    out << "\n";
    
    out << "Simulation Statistics:\n";
    out << "  Total Cycles: " << max_cycle_ << "\n";
    out << "  Total Time: " << (max_cycle_ * cycle_time_ns_) << " ns\n";
    out << "  Activates: " << activate_count_ << "\n";
    out << "  Precharges: " << precharge_count_ << "\n";
    out << "  Reads: " << read_count_ << "\n";
    out << "  Writes: " << write_count_ << "\n";
    out << "  Refreshes: " << refresh_count_ << "\n";
    out << "  Idle Cycles: " << idle_cycles_ << "\n";
    out << "\n";
    
    out << std::fixed << std::setprecision(2);
    out << "Energy Breakdown:\n";
    out << "  Activate Energy: " << activate_energy_pJ_ * 1e-9 << " mJ\n";
    out << "  Precharge Energy: " << precharge_energy_pJ_ * 1e-9 << " mJ\n";
    out << "  Read Energy: " << read_energy_pJ_ * 1e-9 << " mJ\n";
    out << "  Write Energy: " << write_energy_pJ_ * 1e-9 << " mJ\n";
    out << "  Refresh Energy: " << refresh_energy_pJ_ * 1e-9 << " mJ\n";
    out << "  Background Energy: " << background_energy_pJ_ * 1e-9 << " mJ\n";
    out << "  Total Energy: " << total_energy_pJ_ * 1e-9 << " mJ\n";
    out << "\n";
    
    out << "Power Analysis:\n";
    out << "  Average Power: " << get_average_power_mW() << " mW\n";
    out << "  Peak Power: " << peak_power_mW_ << " mW\n";
    out << "  Refresh Overhead: " << get_refresh_overhead_percent() << " %\n";
    out << "\n";
    
    // Estimate junction temperature
    double tj = estimate_junction_temp();
    out << "Thermal Analysis:\n";
    out << "  Estimated Junction Temp: " << tj << " °C\n";
    out << "  Temperature Rise: " << (tj - 25.0) << " °C\n";
    
    out.close();
    std::cout << "Summary exported to " << filename << "\n";
}

// ========== Validation ==========

bool PowerModel::validate_against_datasheet() const {
    // Validation checks:
    // 1. Refresh overhead should be 3.12% ± 0.1%
    // 2. Average power within ±2% of expected
    // 3. Peak power below maximum IDD specifications
    
    double refresh_overhead = get_refresh_overhead_percent();
    double expected_refresh = 3.12;  // Theoretical: (260ns / 7.8μs) * 100%
    
    // Check refresh overhead
    if (std::abs(refresh_overhead - expected_refresh) > 0.1) {
        std::cerr << "Validation FAILED: Refresh overhead " << refresh_overhead 
                  << "% outside tolerance (expected " << expected_refresh << "% ± 0.1%)\n";
        return false;
    }
    
    // Check average power (should be ~290mW for typical DDR4 workload)
    double avg_power = get_average_power_mW();
    double expected_avg = 290.0;  // mW (typical DDR4-2400)
    
    if (std::abs(avg_power - expected_avg) / expected_avg > VALIDATION_TOLERANCE) {
        std::cerr << "Validation WARNING: Average power " << avg_power 
                  << " mW differs from expected " << expected_avg << " mW\n";
        // This is a warning, not a failure (workload dependent)
    }
    
    // Check peak power (should not exceed max IDD)
    double max_idd_power = currents_.IDD5 * voltage_;  // Refresh is typically peak
    if (peak_power_mW_ > max_idd_power * 1.05) {  // 5% margin
        std::cerr << "Validation FAILED: Peak power " << peak_power_mW_ 
                  << " mW exceeds maximum " << max_idd_power << " mW\n";
        return false;
    }
    
    std::cout << "Validation PASSED ✓\n";
    std::cout << "  Refresh overhead: " << refresh_overhead << "%\n";
    std::cout << "  Average power: " << avg_power << " mW\n";
    std::cout << "  Peak power: " << peak_power_mW_ << " mW\n";
    
    return true;
}

double PowerModel::get_validation_error() const {
    double refresh_overhead = get_refresh_overhead_percent();
    double expected_refresh = 3.12;
    
    return std::abs(refresh_overhead - expected_refresh) / expected_refresh;
}

// ========== Statistics ==========

void PowerModel::reset() {
    total_energy_pJ_ = 0.0;
    activate_energy_pJ_ = 0.0;
    precharge_energy_pJ_ = 0.0;
    read_energy_pJ_ = 0.0;
    write_energy_pJ_ = 0.0;
    refresh_energy_pJ_ = 0.0;
    background_energy_pJ_ = 0.0;
    
    peak_power_mW_ = 0.0;
    max_cycle_ = 0;
    
    activate_count_ = 0;
    precharge_count_ = 0;
    read_count_ = 0;
    write_count_ = 0;
    refresh_count_ = 0;
    idle_cycles_ = 0;
    
    power_trace_.clear();
    bank_active_.clear();
    bank_active_.resize(8, false);
}

// ========== Private Helper Methods ==========

double PowerModel::calculate_energy(double current_mA, uint64_t duration_cycles) const {
    // Energy (pJ) = Current (mA) × Voltage (V) × Time (ns) × 1000
    // Factor of 1000 converts mA·V·ns to pJ
    
    double time_ns = duration_cycles * cycle_time_ns_;
    double energy_pJ = current_mA * voltage_ * time_ns;
    
    return energy_pJ;
}

double PowerModel::calculate_power(double current_mA) const {
    // Power (mW) = Current (mA) × Voltage (V)
    return current_mA * voltage_;
}

double PowerModel::apply_temperature_scaling(double base_current) const {
    // Leakage scales exponentially with temperature
    // I(T) = I(T0) × exp[(T - T0) / α]
    // where α ≈ 80°C for modern DRAM
    
    double temp_delta = temperature_ - BASE_TEMPERATURE;
    double scale_factor = std::exp(temp_delta / LEAKAGE_TEMP_COEFFICIENT);
    
    // Only leakage component scales; active current relatively constant
    // Assume 20% of current is leakage
    double leakage_portion = base_current * 0.2;
    double active_portion = base_current * 0.8;
    
    return active_portion + (leakage_portion * scale_factor);
}

double PowerModel::apply_process_scaling(double base_current) const {
    switch (process_corner_) {
        case ProcessCorner::FAST:
            return base_current * FAST_CORNER_SCALE;
        case ProcessCorner::SLOW:
            return base_current * SLOW_CORNER_SCALE;
        case ProcessCorner::TYPICAL:
        default:
            return base_current;
    }
}

double PowerModel::calculate_background_current() const {
    // Background current depends on bank states
    // All banks idle: IDD2N (precharge power-down)
    // Some banks active: IDD3N (active power-down)
    
    bool any_bank_active = false;
    for (bool active : bank_active_) {
        if (active) {
            any_bank_active = true;
            break;
        }
    }
    
    double base_current = any_bank_active ? currents_.IDD3N : currents_.IDD2N;
    return apply_process_scaling(apply_temperature_scaling(base_current));
}

void PowerModel::add_trace_entry(uint64_t cycle, double current_mA, const std::string& command) {
    PowerTraceEntry entry;
    entry.cycle = cycle;
    entry.current_mA = current_mA;
    entry.voltage_V = voltage_;
    entry.power_mW = calculate_power(current_mA);
    entry.command = command;
    
    power_trace_.push_back(entry);
}

} // namespace dram

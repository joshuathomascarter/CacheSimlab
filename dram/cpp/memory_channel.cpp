#include "../headers/memory_channel.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace dram {

// Create standard DDR4 configuration
BankGroupConfig create_ddr4_config(uint32_t num_bank_groups, uint32_t banks_per_group) {
    BankGroupConfig config;
    config.num_bank_groups = num_bank_groups;
    config.num_banks_per_group = banks_per_group;
    config.total_banks = num_bank_groups * banks_per_group;
    
    // DDR4 bank group timing (typical values for DDR4-2400)
    config.tCCD_S = 4;  // Same group
    config.tCCD_L = 6;  // Different group
    config.tRRD_S = 4;
    config.tRRD_L = 6;
    config.tWTR_S = 2;
    config.tWTR_L = 6;
    
    return config;
}

// MemoryChannel implementation
MemoryChannel::MemoryChannel(const TimingParameters& timing,
                             const BankGroupConfig& config,
                             AddressMappingScheme scheme)
    : bank_config_(config),
      timing_params_(timing),
      timing_validator_(std::make_unique<TimingValidator>(timing)),
      mapping_scheme_(scheme),
      current_cycle_(0),
      total_commands_(0),
      bus_utilization_cycles_(0),
      is_3d_stacked_(false),
      num_dies_(1),
      die_bits_(0),
      current_die_(0),
      last_die_switch_cycle_(0) {
    
    // Initialize TSV timing (default values for HBM2/HBM3)
    tsv_timing_.tTSV = 2;           // 2 cycles for TSV transfer
    tsv_timing_.tDIE_SWITCH = 3;    // 3 cycles for die switching
    
    // Initialize thermal config (realistic values)
    thermal_config_.base_temp = 45.0;        // Base: 45°C
    thermal_config_.die_temp_delta = 12.0;   // +12°C per die
    thermal_config_.throttle_temp = 85.0;    // Throttle at 85°C
    thermal_config_.critical_temp = 95.0;    // Critical at 95°C
    
    // Create all banks
    banks_.reserve(config.total_banks);
    for (uint32_t i = 0; i < config.total_banks; ++i) {
        banks_.push_back(std::make_unique<DRAMBank>(timing, timing_validator_.get()));
    }
    
    // Initialize single die by default
    dies_.resize(1);
    dies_[0].die_id = 0;
    dies_[0].current_temp = thermal_config_.base_temp;
    dies_[0].last_access = 0;
    dies_[0].total_accesses = 0;
    dies_[0].throttled = false;
    for (uint32_t i = 0; i < config.total_banks; ++i) {
        dies_[0].bank_ids.push_back(i);
    }
    
    // Initialize bank group timing trackers
    last_cas_per_group_.resize(config.num_bank_groups, 0);
    last_activate_per_group_.resize(config.num_bank_groups, 0);
    last_write_per_group_.resize(config.num_bank_groups, 0);
    
    // Set address bit widths (typical DDR4 values)
    column_bits_ = 10;      // 1024 columns
    row_bits_ = 16;         // 65536 rows
    bank_bits_ = static_cast<uint32_t>(std::log2(config.num_banks_per_group));
    bank_group_bits_ = static_cast<uint32_t>(std::log2(config.num_bank_groups));
    rank_bits_ = 1;         // Single rank for now
    
    // Initialize BLP histogram
    blp_stats_.active_bank_histogram.resize(config.total_banks + 1, 0);
}

uint32_t MemoryChannel::get_bank_group(uint32_t bank_id) const {
    return bank_id / bank_config_.num_banks_per_group;
}

PhysicalAddress MemoryChannel::decode_address(uint64_t address) const {
    PhysicalAddress addr;
    addr.raw_address = address;
    
    uint32_t offset = 0;
    
    switch (mapping_scheme_) {
        case AddressMappingScheme::ROW_BANK_COLUMN: {
            // Sequential mapping: [Row][Bank][BankGroup][Column]
            // Poor BLP but good for debugging
            addr.column = (address >> offset) & ((1ULL << column_bits_) - 1);
            offset += column_bits_;
            
            addr.bank_group = (address >> offset) & ((1ULL << bank_group_bits_) - 1);
            offset += bank_group_bits_;
            
            addr.bank = (address >> offset) & ((1ULL << bank_bits_) - 1);
            offset += bank_bits_;
            
            addr.row = (address >> offset) & ((1ULL << row_bits_) - 1);
            break;
        }
        
        case AddressMappingScheme::BANK_ROW_COLUMN: {
            // Interleaved: [Row][Column][Bank][BankGroup]
            // Better BLP for random access
            addr.bank_group = (address >> offset) & ((1ULL << bank_group_bits_) - 1);
            offset += bank_group_bits_;
            
            addr.bank = (address >> offset) & ((1ULL << bank_bits_) - 1);
            offset += bank_bits_;
            
            addr.column = (address >> offset) & ((1ULL << column_bits_) - 1);
            offset += column_bits_;
            
            addr.row = (address >> offset) & ((1ULL << row_bits_) - 1);
            break;
        }
        
        case AddressMappingScheme::CACHE_LINE_INTERLEAVED: {
            // Cache line (64B) interleaving: Every 64 bytes goes to different bank
            // Optimal for streaming workloads
            // Assumes 64-byte cache lines (6 bits)
            uint32_t cache_line_bits = 6;
            
            offset = cache_line_bits;  // Skip cache line offset
            
            addr.bank_group = (address >> offset) & ((1ULL << bank_group_bits_) - 1);
            offset += bank_group_bits_;
            
            addr.bank = (address >> offset) & ((1ULL << bank_bits_) - 1);
            offset += bank_bits_;
            
            addr.column = (address >> offset) & ((1ULL << (column_bits_ - cache_line_bits)) - 1);
            offset += (column_bits_ - cache_line_bits);
            
            addr.row = (address >> offset) & ((1ULL << row_bits_) - 1);
            break;
        }
        
        case AddressMappingScheme::XOR_INTERLEAVED: {
            // XOR-based hashing for uniform distribution with random access
            addr.column = (address >> 0) & ((1ULL << column_bits_) - 1);
            
            // XOR high and low bits for better distribution
            uint32_t bank_hash = address_mapping::xor_hash_bank(address, 
                                                                 bank_config_.num_banks_per_group);
            addr.bank = bank_hash;
            
            uint32_t group_hash = address_mapping::xor_hash_bank_group(address,
                                                                        bank_config_.num_bank_groups);
            addr.bank_group = group_hash;
            
            addr.row = (address >> (column_bits_ + bank_bits_ + bank_group_bits_)) & 
                       ((1ULL << row_bits_) - 1);
            break;
        }
    }
    
    addr.rank = 0;  // Single rank
    addr.channel = 0;  // Single channel
    
    return addr;
}

uint64_t MemoryChannel::encode_address(const PhysicalAddress& addr) const {
    uint64_t address = 0;
    uint32_t offset = 0;
    
    switch (mapping_scheme_) {
        case AddressMappingScheme::ROW_BANK_COLUMN:
            address |= (static_cast<uint64_t>(addr.column) << offset);
            offset += column_bits_;
            address |= (static_cast<uint64_t>(addr.bank_group) << offset);
            offset += bank_group_bits_;
            address |= (static_cast<uint64_t>(addr.bank) << offset);
            offset += bank_bits_;
            address |= (static_cast<uint64_t>(addr.row) << offset);
            break;
            
        case AddressMappingScheme::BANK_ROW_COLUMN:
            address |= (static_cast<uint64_t>(addr.bank_group) << offset);
            offset += bank_group_bits_;
            address |= (static_cast<uint64_t>(addr.bank) << offset);
            offset += bank_bits_;
            address |= (static_cast<uint64_t>(addr.column) << offset);
            offset += column_bits_;
            address |= (static_cast<uint64_t>(addr.row) << offset);
            break;
            
        case AddressMappingScheme::CACHE_LINE_INTERLEAVED: {
            uint32_t cache_line_bits = 6;
            offset = cache_line_bits;
            address |= (static_cast<uint64_t>(addr.bank_group) << offset);
            offset += bank_group_bits_;
            address |= (static_cast<uint64_t>(addr.bank) << offset);
            offset += bank_bits_;
            address |= (static_cast<uint64_t>(addr.column) << offset);
            offset += (column_bits_ - cache_line_bits);
            address |= (static_cast<uint64_t>(addr.row) << offset);
            break;
        }
            
        case AddressMappingScheme::XOR_INTERLEAVED:
            // Simplified encoding (not perfect inverse of XOR decode)
            address |= (static_cast<uint64_t>(addr.column) << 0);
            address |= (static_cast<uint64_t>(addr.bank) << column_bits_);
            address |= (static_cast<uint64_t>(addr.bank_group) << (column_bits_ + bank_bits_));
            address |= (static_cast<uint64_t>(addr.row) << (column_bits_ + bank_bits_ + bank_group_bits_));
            break;
    }
    
    return address;
}

void MemoryChannel::set_mapping_scheme(AddressMappingScheme scheme) {
    mapping_scheme_ = scheme;
}

bool MemoryChannel::check_bank_group_timing(const BusCommand& cmd) {
    uint32_t bank_group = get_bank_group(cmd.bank_id);
    
    switch (cmd.type) {
        case CommandType::READ:
        case CommandType::WRITE: {
            // Check tCCD timing
            uint64_t last_cas = last_cas_per_group_[bank_group];
            uint32_t required_delay = bank_config_.tCCD_S;
            
            // Check if last command was to different bank group (relaxed timing)
            if (last_command_.type == CommandType::READ || 
                last_command_.type == CommandType::WRITE) {
                uint32_t last_group = get_bank_group(last_command_.bank_id);
                if (last_group != bank_group) {
                    required_delay = bank_config_.tCCD_L;
                }
            }
            
            if (current_cycle_ < last_cas + required_delay) {
                return false;
            }
            
            // Check tWTR timing (write to read)
            if (cmd.type == CommandType::READ) {
                uint64_t last_write = last_write_per_group_[bank_group];
                uint32_t wtr_delay = bank_config_.tWTR_S;
                
                if (last_command_.type == CommandType::WRITE) {
                    uint32_t last_group = get_bank_group(last_command_.bank_id);
                    if (last_group != bank_group) {
                        wtr_delay = bank_config_.tWTR_L;
                    }
                }
                
                if (current_cycle_ < last_write + wtr_delay) {
                    return false;
                }
            }
            break;
        }
        
        case CommandType::ACTIVATE: {
            // Check tRRD timing
            uint64_t last_act = last_activate_per_group_[bank_group];
            uint32_t required_delay = bank_config_.tRRD_S;
            
            if (last_command_.type == CommandType::ACTIVATE) {
                uint32_t last_group = get_bank_group(last_command_.bank_id);
                if (last_group != bank_group) {
                    required_delay = bank_config_.tRRD_L;
                }
            }
            
            if (current_cycle_ < last_act + required_delay) {
                return false;
            }
            break;
        }
        
        default:
            break;
    }
    
    return true;
}

void MemoryChannel::update_bank_group_timing(const BusCommand& cmd) {
    uint32_t bank_group = get_bank_group(cmd.bank_id);
    
    switch (cmd.type) {
        case CommandType::READ:
        case CommandType::WRITE:
            last_cas_per_group_[bank_group] = current_cycle_;
            if (cmd.type == CommandType::WRITE) {
                last_write_per_group_[bank_group] = current_cycle_;
            }
            break;
            
        case CommandType::ACTIVATE:
            last_activate_per_group_[bank_group] = current_cycle_;
            break;
            
        default:
            break;
    }
    
    last_command_ = cmd;
}

bool MemoryChannel::can_issue_command(CommandType type, uint32_t bank_id, uint32_t row) {
    if (bank_id >= banks_.size()) {
        return false;
    }
    
    // Check if bank can accept command
    bool can_accept = false;
    switch (type) {
        case CommandType::ACTIVATE:
            can_accept = banks_[bank_id]->can_activate(current_cycle_);
            break;
        case CommandType::READ:
            can_accept = banks_[bank_id]->can_read(current_cycle_);
            break;
        case CommandType::WRITE:
            can_accept = banks_[bank_id]->can_write(current_cycle_);
            break;
        case CommandType::PRECHARGE:
            can_accept = banks_[bank_id]->can_precharge(current_cycle_);
            break;
        case CommandType::REFRESH:
            can_accept = banks_[bank_id]->can_refresh(current_cycle_);
            break;
        default:
            can_accept = false;
    }
    if (!can_accept) {
        return false;
    }
    
    // Create temporary command to check timing
    BusCommand temp_cmd;
    temp_cmd.type = type;
    temp_cmd.bank_id = bank_id;
    temp_cmd.bank_group_id = get_bank_group(bank_id);
    temp_cmd.row = row;
    temp_cmd.cycle_issued = current_cycle_;
    
    // Check bank group timing constraints
    if (!check_bank_group_timing(temp_cmd)) {
        return false;
    }
    
    return true;
}

bool MemoryChannel::issue_command(CommandType type, uint32_t bank_id, uint32_t row, uint64_t request_id) {
    if (!can_issue_command(type, bank_id, row)) {
        return false;
    }
    
    // Issue command to bank
    bool issued = false;
    switch (type) {
        case CommandType::ACTIVATE:
            issued = banks_[bank_id]->activate(row, current_cycle_);
            break;
        case CommandType::READ:
            issued = banks_[bank_id]->read(0, current_cycle_);  // Column doesn't matter for timing
            break;
        case CommandType::WRITE:
            issued = banks_[bank_id]->write(0, current_cycle_);  // Column doesn't matter for timing
            break;
        case CommandType::PRECHARGE:
            issued = banks_[bank_id]->precharge(current_cycle_);
            break;
        case CommandType::REFRESH:
            issued = banks_[bank_id]->refresh(current_cycle_);
            break;
        default:
            issued = false;
    }
    if (!issued) {
        return false;
    }
    
    // Create bus command
    BusCommand cmd;
    cmd.type = type;
    cmd.bank_id = bank_id;
    cmd.bank_group_id = get_bank_group(bank_id);
    cmd.row = row;
    cmd.cycle_issued = current_cycle_;
    cmd.request_id = request_id;
    
    // Update timing trackers
    update_bank_group_timing(cmd);
    
    // Statistics
    total_commands_++;
    bus_utilization_cycles_++;
    
    return true;
}

void MemoryChannel::tick() {
    // Update all banks
    for (auto& bank : banks_) {
        bank->update(current_cycle_);
    }
    
    // Calculate BLP for this cycle
    uint32_t active_banks = 0;
    for (const auto& bank : banks_) {
        BankState state = bank->get_state();
        if (state == BankState::READING || 
            state == BankState::WRITING || 
            state == BankState::ACTIVATING ||
            state == BankState::PRECHARGING) {
            active_banks++;
        }
    }
    
    blp_stats_.update(active_banks);
}

void MemoryChannel::advance_cycle(uint64_t cycles) {
    for (uint64_t i = 0; i < cycles; ++i) {
        tick();
        current_cycle_++;
    }
}

BankState MemoryChannel::get_bank_state(uint32_t bank_id) const {
    if (bank_id >= banks_.size()) {
        return BankState::IDLE;
    }
    return banks_[bank_id]->get_state();
}

uint32_t MemoryChannel::get_open_row(uint32_t bank_id) const {
    if (bank_id >= banks_.size()) {
        return 0;
    }
    return banks_[bank_id]->get_open_row();
}

bool MemoryChannel::is_row_hit(uint32_t bank_id, uint32_t row) const {
    if (bank_id >= banks_.size()) {
        return false;
    }
    // Row buffer hit if bank is active and requested row is already open
    return banks_[bank_id]->get_state() == BankState::ACTIVE && 
           banks_[bank_id]->get_open_row() == row;
}

double MemoryChannel::get_bus_utilization() const {
    if (current_cycle_ == 0) return 0.0;
    return static_cast<double>(bus_utilization_cycles_) / current_cycle_;
}

void MemoryChannel::reset_statistics() {
    blp_stats_ = BLPStatistics();
    blp_stats_.active_bank_histogram.resize(bank_config_.total_banks + 1, 0);
    total_commands_ = 0;
    bus_utilization_cycles_ = 0;
}

void MemoryChannel::enable_3d_stacking(uint32_t num_dies) {
    if (num_dies < 2 || num_dies > 8) {
        throw std::runtime_error("Invalid num_dies for 3D stacking. Must be 2-8");
    }
    
    is_3d_stacked_ = true;
    num_dies_ = num_dies;
    die_bits_ = static_cast<uint32_t>(std::log2(num_dies));
    
    // Distribute banks evenly across dies
    uint32_t banks_per_die = bank_config_.total_banks / num_dies_;
    
    // Initialize die states
    dies_.clear();
    dies_.resize(num_dies_);
    
    for (uint32_t d = 0; d < num_dies_; ++d) {
        dies_[d].die_id = d;
        dies_[d].current_temp = thermal_config_.base_temp + (d * thermal_config_.die_temp_delta);
        dies_[d].last_access = 0;
        dies_[d].total_accesses = 0;
        dies_[d].throttled = false;
        
        // Assign banks to this die
        dies_[d].bank_ids.clear();
        for (uint32_t b = d * banks_per_die; b < (d + 1) * banks_per_die; ++b) {
            dies_[d].bank_ids.push_back(b);
        }
    }
    
    current_die_ = 0;
    last_die_switch_cycle_ = 0;
}

void MemoryChannel::print_bank_status() const {
    std::cout << "\n=== Memory Channel Status (Cycle " << current_cycle_ << ") ===\n";
    std::cout << "Mapping Scheme: ";
    switch (mapping_scheme_) {
        case AddressMappingScheme::ROW_BANK_COLUMN:
            std::cout << "ROW_BANK_COLUMN\n";
            break;
        case AddressMappingScheme::BANK_ROW_COLUMN:
            std::cout << "BANK_ROW_COLUMN\n";
            break;
        case AddressMappingScheme::CACHE_LINE_INTERLEAVED:
            std::cout << "CACHE_LINE_INTERLEAVED\n";
            break;
        case AddressMappingScheme::XOR_INTERLEAVED:
            std::cout << "XOR_INTERLEAVED\n";
            break;
    }
    
    std::cout << "\nBank States:\n";
    for (size_t i = 0; i < banks_.size(); ++i) {
        uint32_t group = get_bank_group(i);
        std::cout << "  Bank " << std::setw(2) << i 
                  << " (Group " << group << "): ";
        
        BankState state = banks_[i]->get_state();
        switch (state) {
            case BankState::IDLE: std::cout << "IDLE      "; break;
            case BankState::ACTIVATING: std::cout << "ACTIVATING"; break;
            case BankState::ACTIVE: std::cout << "ACTIVE    "; break;
            case BankState::READING: std::cout << "READING   "; break;
            case BankState::WRITING: std::cout << "WRITING   "; break;
            case BankState::PRECHARGING: std::cout << "PRECHARGE "; break;
            case BankState::REFRESHING: std::cout << "REFRESHING"; break;
        }
        
        if (state == BankState::ACTIVE || state == BankState::READING || state == BankState::WRITING) {
            std::cout << " Row: " << std::setw(5) << banks_[i]->get_open_row();
        }
        std::cout << "\n";
    }
    
    std::cout << "\nBLP Statistics:\n";
    std::cout << "  Average BLP: " << std::fixed << std::setprecision(2) 
              << blp_stats_.average_blp << "\n";
    std::cout << "  Peak BLP: " << blp_stats_.peak_blp << "\n";
    std::cout << "  Bus Utilization: " << (get_bus_utilization() * 100.0) << "%\n";
}

void MemoryChannel::export_trace(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open trace file: " << filename << "\n";
        return;
    }
    
    file << "cycle,active_banks,bus_utilization,average_blp\n";
    
    // Export summary statistics
    file << current_cycle_ << ","
         << blp_stats_.total_active_banks << ","
         << get_bus_utilization() << ","
         << blp_stats_.average_blp << "\n";
    
    file.close();
}

// Address mapping helper functions
namespace address_mapping {

BitLayout calculate_layout(AddressMappingScheme scheme,
                          uint32_t column_bits,
                          uint32_t row_bits,
                          uint32_t bank_bits,
                          uint32_t bank_group_bits,
                          uint32_t rank_bits) {
    BitLayout layout;
    uint32_t offset = 0;
    
    switch (scheme) {
        case AddressMappingScheme::ROW_BANK_COLUMN:
            layout.column_start = offset;
            layout.column_end = offset + column_bits - 1;
            offset += column_bits;
            
            layout.bank_group_start = offset;
            layout.bank_group_end = offset + bank_group_bits - 1;
            offset += bank_group_bits;
            
            layout.bank_start = offset;
            layout.bank_end = offset + bank_bits - 1;
            offset += bank_bits;
            
            layout.row_start = offset;
            layout.row_end = offset + row_bits - 1;
            offset += row_bits;
            
            layout.rank_start = offset;
            layout.rank_end = offset + rank_bits - 1;
            break;
            
        case AddressMappingScheme::CACHE_LINE_INTERLEAVED: {
            uint32_t cache_line_bits = 6;  // 64 bytes
            offset = cache_line_bits;
            
            layout.bank_group_start = offset;
            layout.bank_group_end = offset + bank_group_bits - 1;
            offset += bank_group_bits;
            
            layout.bank_start = offset;
            layout.bank_end = offset + bank_bits - 1;
            offset += bank_bits;
            
            layout.column_start = offset;
            layout.column_end = offset + (column_bits - cache_line_bits) - 1;
            offset += (column_bits - cache_line_bits);
            
            layout.row_start = offset;
            layout.row_end = offset + row_bits - 1;
            offset += row_bits;
            
            layout.rank_start = offset;
            layout.rank_end = offset + rank_bits - 1;
            break;
        }
            
        default:
            // Similar to BANK_ROW_COLUMN
            layout.bank_group_start = offset;
            layout.bank_group_end = offset + bank_group_bits - 1;
            offset += bank_group_bits;
            
            layout.bank_start = offset;
            layout.bank_end = offset + bank_bits - 1;
            offset += bank_bits;
            
            layout.column_start = offset;
            layout.column_end = offset + column_bits - 1;
            offset += column_bits;
            
            layout.row_start = offset;
            layout.row_end = offset + row_bits - 1;
            offset += row_bits;
            
            layout.rank_start = offset;
            layout.rank_end = offset + rank_bits - 1;
            break;
    }
    
    return layout;
}

uint32_t xor_hash_bank(uint64_t address, uint32_t num_banks) {
    uint32_t bits = static_cast<uint32_t>(std::log2(num_banks));
    uint32_t mask = (1U << bits) - 1;
    
    // Extract low bits from Bank field [9:8]
    uint32_t low = (address >> 8) & mask;
    

    // Extract LOW bits from Row field [21:20] - SMALL row changes
    // This changes the bank for small row increments
    uint32_t high = (address >> 20) & mask;
    
    // XOR them: Bank bits ^ Row bits
    // Small row changes flip the bank within same group
    return (low ^ high) & mask;
}

uint32_t xor_hash_bank_group(uint64_t address, uint32_t num_groups) {
    uint32_t bits = static_cast<uint32_t>(std::log2(num_groups));
    uint32_t mask = (1U << bits) - 1;
    
    // Extract low bits from BankGroup field [7:6]
    uint32_t low = (address >> 6) & mask;
    
    // Extract HIGH bits from Row field [29:28] - BIG row changes
    // This changes the bank group for large row increments
    uint32_t high = (address >> 28) & mask;
    
    // XOR them: BankGroup bits ^ Row bits
    // Big row changes flip the bank group
    return (low ^ high) & mask;
}

} // namespace address_mapping

} // namespace dram

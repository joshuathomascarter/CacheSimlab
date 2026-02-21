// Test 3D-stacked DRAM (HBM simulation)
#include "../headers/memory_channel.h"
#include <iostream>
#include <iomanip>

using namespace dram;

void test_3d_basic() {
    std::cout << "\n=== TEST 1: Basic 3D Stacking (4 Dies) ===\n";
    
    // Create channel
    auto timing = DRAMTiming::DDR4_2400_R();
    auto config = create_ddr4_config(4, 4);  // 4 groups, 4 banks/group = 16 banks total
    
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    
    // Enable 4-die stacking (HBM style)
    channel.enable_3d_stacking(4);
    
    std::cout << "Enabled 3D stacking with 4 dies\n";
    std::cout << "Each die has " << (config.total_banks / 4) << " banks\n";
    std::cout << "- Die 0: Banks 0-3\n";
    std::cout << "- Die 1: Banks 4-7\n";
    std::cout << "- Die 2: Banks 8-11\n";
    std::cout << "- Die 3: Banks 12-15\n\n";
}

void test_tsv_timing() {
    std::cout << "\n=== TEST 2: TSV Timing and Die Switching ===\n";
    
    auto timing = DRAMTiming::DDR4_2400_R();
    auto config = create_ddr4_config(4, 4);
    
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    channel.enable_3d_stacking(4);
    
    std::cout << "Issuing commands to different dies...\n";
    
    // Issue to bank 0 (Die 0)
    std::cout << "\nCycle 0: Accessing Bank 0 (Die 0)...\n";
    channel.issue_command(CommandType::ACTIVATE, 0, 100, 1);
    channel.advance_cycle();
    
    // Issue to bank 8 (Die 2) - requires die switch
    std::cout << "Cycle 1: Accessing Bank 8 (Die 2) - requires die switch delay\n";
    bool success = channel.issue_command(CommandType::ACTIVATE, 8, 200, 2);
    std::cout << "Command issued: " << (success ? "SUCCESS" : "FAILED (waiting for tDIE_SWITCH)") << "\n";
    
    // Wait for die switch delay (tDIE_SWITCH = 3 cycles)
    for (int i = 0; i < 3; ++i) {
        channel.advance_cycle();
        success = channel.issue_command(CommandType::ACTIVATE, 8, 200, 2);
        std::cout << "Cycle " << channel.get_current_cycle() 
                  << ": " << (success ? "SUCCESS" : "Still waiting...") << "\n";
        if (success) break;
    }
}

void test_thermal_model() {
    std::cout << "\n=== TEST 3: Thermal Modeling ===\n";
    
    auto timing = DRAMTiming::DDR4_2400_R();
    auto config = create_ddr4_config(4, 4);
    
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    channel.enable_3d_stacking(4);
    
    std::cout << "Each die has base temperature gradient:\n";
    std::cout << "- Die 0: 45°C (bottom, best cooling)\n";
    std::cout << "- Die 1: 57°C (45 + 12)\n";
    std::cout << "- Die 2: 69°C (45 + 24)\n";
    std::cout << "- Die 3: 81°C (45 + 36, top)\n\n";
    
    std::cout << "Thermal throttling kicks in at 85°C\n";
    std::cout << "Critical temperature: 95°C\n\n";
    
    std::cout << "This means Die 3 (top) will throttle faster than Die 0 (bottom)\n";
    std::cout << "Realistic HBM behavior: top dies get hotter!\n";
}

void test_address_decoding_3d() {
    std::cout << "\n=== TEST 4: Address Decoding with Die Field ===\n";
    
    auto timing = DRAMTiming::DDR4_2400_R();
    auto config = create_ddr4_config(4, 4);
    
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    channel.enable_3d_stacking(4);
    
    std::cout << "Address layout with 4 dies (2 die bits):\n";
    std::cout << "[Row bits] [Column bits] [Die bits] [Bank bits] [BG bits] [Cache line]\n";
    std::cout << "[   ...  ] [    ...    ] [  9:8   ] [  7:6   ] [ 5:4  ] [  3:0  ]\n\n";
    
    uint64_t addr1 = 0x0000;  // Die 0, Bank 0
    uint64_t addr2 = 0x0100;  // Die 1, Bank 0
    uint64_t addr3 = 0x0200;  // Die 2, Bank 0
    uint64_t addr4 = 0x0300;  // Die 3, Bank 0
    
    auto decoded1 = channel.decode_address(addr1);
    auto decoded2 = channel.decode_address(addr2);
    auto decoded3 = channel.decode_address(addr3);
    auto decoded4 = channel.decode_address(addr4);
    
    std::cout << std::hex;
    std::cout << "Address 0x" << addr1 << " -> Die " << decoded1.rank << ", Bank " << decoded1.bank << "\n";
    std::cout << "Address 0x" << addr2 << " -> Die " << decoded2.rank << ", Bank " << decoded2.bank << "\n";
    std::cout << "Address 0x" << addr3 << " -> Die " << decoded3.rank << ", Bank " << decoded3.bank << "\n";
    std::cout << "Address 0x" << addr4 << " -> Die " << decoded4.rank << ", Bank " << decoded4.bank << "\n";
    std::cout << std::dec;
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║    3D-STACKED DRAM (HBM) SIMULATION TESTS             ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    test_3d_basic();
    test_tsv_timing();
    test_thermal_model();
    test_address_decoding_3d();
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║    KEY CONCEPTS FOR APPLE INTERVIEWS                  ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    std::cout << "\n1. TSV (Through-Silicon Via):\n";
    std::cout << "   - Vertical connections between dies (1-2 cycle delay)\n";
    std::cout << "   - Much faster than wire-bonding\n";
    std::cout << "   - Key enabler for HBM bandwidth\n\n";
    
    std::cout << "2. Die Switching Overhead:\n";
    std::cout << "   - tDIE_SWITCH = 2-3 cycles when changing dies\n";
    std::cout << "   - Scheduler should minimize die thrashing\n";
    std::cout << "   - Batch requests to same die when possible\n\n";
    
    std::cout << "3. Thermal Gradient:\n";
    std::cout << "   - Bottom die: best cooling (closest to package)\n";
    std::cout << "   - Top die: worst cooling (heat rises + trapped)\n";
    std::cout << "   - Each die ~10-15°C hotter than below\n";
    std::cout << "   - Scheduler must be thermal-aware!\n\n";
    
    std::cout << "4. HBM vs GDDR vs DDR:\n";
    std::cout << "   - HBM: 4-8 dies, 1024-bit bus, 256+ GB/s\n";
    std::cout << "   - GDDR6: single die, 32-bit channels, ~50 GB/s\n";
    std::cout << "   - DDR5: single die, 64-bit bus, ~40 GB/s\n\n";
    
    std::cout << "5. Apple Silicon Uses:\n";
    std::cout << "   - M1/M2/M3: LPDDR (not HBM, unified memory)\n";
    std::cout << "   - M1 Ultra: Die-to-die interconnect (similar concept)\n";
    std::cout << "   - Future: HBM for high-end workstation?\n\n";
    
    return 0;
}

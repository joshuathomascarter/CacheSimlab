/**
 * @file generate_power_trace.cpp
 * @brief Generate power trace CSV for Python analyzer
 */

#include "../headers/refresh_controller.h"
#include "../headers/power_model.h"
#include "../headers/dram_timing.h"
#include <iostream>

using namespace dram;

int main() {
    std::cout << "Generating power trace for Python analyzer...\n";
    
    // Initialize components
    DRAMTiming timing;
    RefreshController refresh(timing, 8, 65536);
    PowerModel power;
    
    // Run simulation
    const uint64_t simulation_cycles = 100000;  // ~62.5 microseconds
    
    for (uint64_t cycle = 0; cycle < simulation_cycles; ++cycle) {
        // Background power every cycle
        power.record_idle_cycle(cycle);
        
        // Check for refresh
        if (refresh.is_refresh_needed(cycle)) {
            auto cmd = refresh.get_next_refresh(cycle);
            
            if (cmd) {
                uint64_t trfc = 416;  // tRFC cycles
                power.record_refresh(cycle, cmd->bank_id == 0xFF);
                refresh.complete_refresh(*cmd, cycle + trfc);
            }
        }
        
        // Simulate some read/write activity
        if (cycle % 100 == 10) {
            power.record_activate(cycle, 0);
        }
        if (cycle % 100 == 20) {
            power.record_read(cycle, 0);
        }
        if (cycle % 100 == 40) {
            power.record_write(cycle, 0);
        }
        if (cycle % 100 == 50) {
            power.record_precharge(cycle, 0);
        }
    }
    
    // Export trace for Python analyzer
    power.export_power_trace("power_trace_output.csv", 10);  // Sample every 10 cycles
    
    std::cout << "✅ Generated power_trace_output.csv\n";
    std::cout << "Run with: python3 python/power_analyzer.py power_trace_output.csv\n";
    
    return 0;
}

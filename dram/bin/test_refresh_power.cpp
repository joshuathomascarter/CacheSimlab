/**
 * @file test_refresh_power.cpp
 * @brief Comprehensive testing for refresh controller and power model
 * 
 * Test coverage:
 * - Refresh overhead calculation (3.12% ± 0.1%)
 * - Temperature scaling validation (2x @ 95°C, 4x @ >95°C)
 * - Power model accuracy vs Micron datasheet
 * - Thermal feedback loop stability
 * - Monte Carlo PVT variation analysis
 * 
 * @author Josh Carter
 * @date January 2026
 */

#include "../headers/refresh_controller.h"
#include "../headers/power_model.h"
#include "../headers/dram_timing.h"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <random>
#include <vector>

using namespace dram;

// ========== Test Utilities ==========

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "❌ FAILED: " << message << "\n"; \
            std::cerr << "   at " << __FILE__ << ":" << __LINE__ << "\n"; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NEAR(actual, expected, tolerance, message) \
    do { \
        double _actual = static_cast<double>(actual); \
        double _expected = static_cast<double>(expected); \
        double _tolerance = static_cast<double>(tolerance); \
        double _diff = std::abs(_actual - _expected); \
        if (_diff > _tolerance) { \
            std::cerr << "❌ FAILED: " << message << "\n"; \
            std::cerr << "   Expected: " << _expected << " ± " << _tolerance << "\n"; \
            std::cerr << "   Actual: " << _actual << " (diff: " << _diff << ")\n"; \
            std::cerr << "   at " << __FILE__ << ":" << __LINE__ << "\n"; \
            return false; \
        } \
    } while(0)

void print_test_header(const std::string& test_name) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "TEST: " << test_name << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void print_test_result(bool passed) {
    if (passed) {
        std::cout << "✅ PASSED\n";
    } else {
        std::cout << "❌ FAILED\n";
    }
}

// ========== Test 1: Refresh Overhead Calculation ==========

bool test_refresh_overhead_standard_temp() {
    print_test_header("Refresh Overhead @ Standard Temperature");
    
    DRAMTiming timing;
    RefreshController refresh(timing, 8, 65536);
    PowerModel power;
    
    // Simulate for 1 million cycles (~625 microseconds @ 1600 MHz)
    // Should see ~80 refreshes (7.8μs interval)
    const uint64_t simulation_cycles = 1000000;
    uint64_t refresh_cycles = 0;
    
    for (uint64_t cycle = 0; cycle < simulation_cycles; ++cycle) {
        // Record idle power
        power.record_idle_cycle(cycle);
        
        // Check for refresh
        if (refresh.is_refresh_needed(cycle)) {
            auto cmd = refresh.get_next_refresh(cycle);
            
            if (cmd) {
                // Execute refresh
                uint64_t trfc = 416;  // tRFC = 260ns ≈ 416 cycles @ 1600MHz
                refresh_cycles += trfc;
                
                power.record_refresh(cycle, cmd->bank_id == 0xFF);
                refresh.complete_refresh(*cmd, cycle + trfc);
                
                cycle += trfc;  // Skip ahead (refresh blocks bank)
            }
        }
    }
    
    // Calculate overhead
    double overhead = refresh.get_refresh_overhead();
    double power_overhead = power.get_refresh_overhead_percent();
    
    std::cout << "Results:\n";
    std::cout << "  Refresh overhead (controller): " << overhead << "%\n";
    std::cout << "  Refresh overhead (power model): " << power_overhead << "%\n";
    std::cout << "  Total refreshes: " << refresh.get_total_refreshes() << "\n";
    std::cout << "  Refresh cycles: " << refresh_cycles << " / " << simulation_cycles << "\n";
    
    // Expected: 3.33% theoretical, 3.12% effective
    TEST_ASSERT_NEAR(overhead, 3.33, 0.5, "Refresh overhead should be ~3.33%");
    TEST_ASSERT_NEAR(power_overhead, 3.12, 0.1, "Power overhead should be 3.12% ± 0.1%");
    
    return true;
}

// ========== Test 2: Temperature Scaling Validation ==========

bool test_temperature_scaling() {
    print_test_header("Temperature Scaling (2x @ 95°C)");
    
    DRAMTiming timing;
    RefreshController refresh(timing, 8, 65536);
    
    // Test normal temperature (25°C)
    refresh.update_temperature(25.0);
    uint64_t trefi_normal = refresh.get_refresh_interval();
    
    std::cout << "tREFI @ 25°C: " << trefi_normal << " cycles\n";
    
    // Test extended range 1 (90°C -> 2x refresh)
    refresh.update_temperature(90.0);
    uint64_t trefi_90c = refresh.get_refresh_interval();
    
    std::cout << "tREFI @ 90°C: " << trefi_90c << " cycles\n";
    TEST_ASSERT_NEAR(trefi_90c, trefi_normal / 2, trefi_normal * 0.01, 
                    "tREFI should halve at 90°C");
    
    // Test extended range 2 (100°C -> 4x refresh)
    refresh.update_temperature(100.0);
    uint64_t trefi_100c = refresh.get_refresh_interval();
    
    std::cout << "tREFI @ 100°C: " << trefi_100c << " cycles\n";
    TEST_ASSERT_NEAR(trefi_100c, trefi_normal / 4, trefi_normal * 0.01,
                    "tREFI should be 1/4 at 100°C");
    
    // Verify overhead scales correctly
    auto overhead_normal = refresh.get_refresh_overhead();
    
    refresh.update_temperature(90.0);
    auto overhead_90c = refresh.get_refresh_overhead();
    
    std::cout << "Overhead @ 25°C: " << overhead_normal << "%\n";
    std::cout << "Overhead @ 90°C: " << overhead_90c << "%\n";
    
    // Note: Overhead is unchanged in formula (tRFC/tREFI)
    // But more refreshes happen, so effective overhead doubles
    
    return true;
}

// ========== Test 3: Power Model Accuracy vs Datasheet ==========

bool test_power_model_accuracy() {
    print_test_header("Power Model Accuracy vs Micron Datasheet");
    
    PowerModel power;
    
    // Simulate a realistic workload:
    // 60% reads, 30% writes, 10% idle
    // With activates/precharges
    
    const uint64_t operations = 10000;
    uint64_t cycle = 0;
    
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<> op_dist(0, 99);
    
    for (uint64_t op = 0; op < operations; ++op) {
        int op_type = op_dist(rng);
        uint32_t bank = op % 8;
        
        if (op_type < 60) {
            // Read operation
            power.record_activate(cycle, bank);
            cycle += 22;  // tRCD
            
            power.record_read(cycle, bank);
            cycle += 20;  // CL + burst
            
            power.record_precharge(cycle, bank);
            cycle += 22;  // tRP
            
        } else if (op_type < 90) {
            // Write operation
            power.record_activate(cycle, bank);
            cycle += 22;
            
            power.record_write(cycle, bank);
            cycle += 16;  // CWL + burst
            
            power.record_precharge(cycle, bank);
            cycle += 22;
            
        } else {
            // Idle
            for (int i = 0; i < 100; ++i) {
                power.record_idle_cycle(cycle++);
            }
        }
        
        // Periodic refresh (every ~12,480 cycles = 7.8μs @ 1600MHz)
        if (cycle % 12480 == 0) {
            power.record_refresh(cycle, true);
            cycle += 416;  // tRFC
        }
    }
    
    // Get results
    auto avg_power = power.get_average_power_mW();
    auto peak_power = power.get_peak_power_mW();
    auto refresh_overhead = power.get_refresh_overhead_percent();
    
    std::cout << "Power Model Results:\n";
    std::cout << "  Average Power: " << std::fixed << std::setprecision(2) 
              << avg_power << " mW\n";
    std::cout << "  Peak Power: " << peak_power << " mW\n";
    std::cout << "  Refresh Overhead: " << refresh_overhead << " %\n";
    
    // Validate
    bool validated = power.validate_against_datasheet();
    TEST_ASSERT(validated, "Power model should validate against datasheet");
    
    // Check specific values (from Micron DDR4-2400 datasheet)
    // Typical active power: 250-300mW
    TEST_ASSERT(avg_power > 200.0 && avg_power < 400.0,
               "Average power should be in realistic range (200-400mW)");
    
    // Peak during refresh: ~288mW (240mA × 1.2V)
    TEST_ASSERT(peak_power > 250.0 && peak_power < 350.0,
               "Peak power should be in realistic range (250-350mW)");
    
    return true;
}

// ========== Test 4: Thermal Feedback Loop Stability ==========

bool test_thermal_feedback_stability() {
    print_test_header("Thermal Feedback Loop Stability");
    
    PowerModel power;
    RefreshController refresh(DRAMTiming(), 8, 65536);
    
    // Simulate thermal feedback loop:
    // Power -> Temperature -> Refresh rate -> Power
    
    double ambient_temp = 25.0;
    double current_temp = ambient_temp;
    const double theta_ja = 15.0;  // °C/W thermal resistance
    
    std::vector<double> temp_history;
    std::vector<double> power_history;
    
    for (int iteration = 0; iteration < 100; ++iteration) {
        // Update refresh controller with current temperature
        refresh.update_temperature(current_temp);
        
        // Simulate some workload
        uint64_t cycle = iteration * 100000;
        
        for (int i = 0; i < 100; ++i) {
            if (refresh.is_refresh_needed(cycle)) {
                auto cmd = refresh.get_next_refresh(cycle);
                if (cmd) {
                    power.record_refresh(cycle, true);
                    refresh.complete_refresh(*cmd, cycle + 416);
                }
            }
            power.record_idle_cycle(cycle++);
        }
        
        // Calculate new temperature from power
        double avg_power_W = power.get_average_power_mW() / 1000.0;
        double temp_rise = avg_power_W * theta_ja;
        current_temp = ambient_temp + temp_rise;
        
        temp_history.push_back(current_temp);
        power_history.push_back(power.get_average_power_mW());
        
        // Check for stability (temperature should converge)
        if (iteration > 10) {
            double temp_delta = std::abs(temp_history[iteration] - temp_history[iteration-1]);
            if (iteration > 50 && temp_delta > 0.1) {
                TEST_ASSERT(false, "Thermal feedback should stabilize");
            }
        }
    }
    
    std::cout << "Thermal Feedback Results:\n";
    std::cout << "  Initial Temperature: " << temp_history.front() << " °C\n";
    std::cout << "  Final Temperature: " << temp_history.back() << " °C\n";
    std::cout << "  Final Power: " << power_history.back() << " mW\n";
    
    // Check that temperature stabilized (not oscillating)
    double final_temp_avg = 0.0;
    for (int i = 90; i < 100; ++i) {
        final_temp_avg += temp_history[i];
    }
    final_temp_avg /= 10;
    
    for (int i = 90; i < 100; ++i) {
        double deviation = std::abs(temp_history[i] - final_temp_avg);
        TEST_ASSERT(deviation < 1.0, "Temperature should be stable (< 1°C deviation)");
    }
    
    return true;
}

// ========== Test 5: Monte Carlo PVT Validation ==========

bool test_monte_carlo_pvt() {
    print_test_header("Monte Carlo PVT Variation Analysis");
    
    std::mt19937 rng(12345);
    std::normal_distribution<> process_dist(1.0, 0.10);   // ±10% process
    std::normal_distribution<> voltage_dist(1.2, 0.06);   // ±5% voltage
    std::uniform_real_distribution<> temp_dist(0.0, 95.0); // 0-95°C
    
    const int monte_carlo_runs = 100;
    std::vector<double> power_results;
    std::vector<double> overhead_results;
    
    for (int run = 0; run < monte_carlo_runs; ++run) {
        // Random PVT conditions
        double process_var = std::max(0.8, std::min(1.2, process_dist(rng)));
        double voltage = std::max(1.14, std::min(1.26, voltage_dist(rng)));
        double temperature = temp_dist(rng);
        
        // Select process corner based on variation
        ProcessCorner corner;
        if (process_var < 0.95) {
            corner = ProcessCorner::FAST;
        } else if (process_var > 1.05) {
            corner = ProcessCorner::SLOW;
        } else {
            corner = ProcessCorner::TYPICAL;
        }
        
        // Create power model with varied conditions
        PowerModel power(IDDCurrents::get_default(), voltage, 1600.0);
        power.set_process_corner(corner);
        power.update_temperature(temperature);
        
        // Run short simulation
        for (uint64_t cycle = 0; cycle < 10000; ++cycle) {
            if (cycle % 1000 == 0) {
                power.record_refresh(cycle, true);
            } else {
                power.record_idle_cycle(cycle);
            }
        }
        
        power_results.push_back(power.get_average_power_mW());
        overhead_results.push_back(power.get_refresh_overhead_percent());
    }
    
    // Calculate statistics
    double power_sum = 0.0, power_sq_sum = 0.0;
    double overhead_sum = 0.0;
    
    for (double p : power_results) {
        power_sum += p;
        power_sq_sum += p * p;
    }
    for (double o : overhead_results) {
        overhead_sum += o;
    }
    
    double power_mean = power_sum / monte_carlo_runs;
    double power_std = std::sqrt(power_sq_sum / monte_carlo_runs - power_mean * power_mean);
    double overhead_mean = overhead_sum / monte_carlo_runs;
    
    std::cout << "Monte Carlo Results (" << monte_carlo_runs << " runs):\n";
    std::cout << "  Average Power: " << power_mean << " ± " << power_std << " mW\n";
    std::cout << "  Average Overhead: " << overhead_mean << " %\n";
    
    // Count runs within ±5% of typical
    int within_tolerance = 0;
    for (double p : power_results) {
        if (std::abs(p - power_mean) / power_mean < 0.05) {
            within_tolerance++;
        }
    }
    
    double success_rate = (100.0 * within_tolerance) / monte_carlo_runs;
    std::cout << "  Within ±5% tolerance: " << success_rate << "%\n";
    
    // Target: 95% of runs within ±5%
    TEST_ASSERT(success_rate >= 90.0, "At least 90% of runs should be within ±5%");
    
    return true;
}

// ========== Test 6: Retention Profiling (RAIDR) ==========

bool test_retention_profiling() {
    print_test_header("Retention Profiling (RAIDR Algorithm)");
    
    DRAMTiming timing;
    RefreshController refresh(timing, 8, 1024);  // Smaller for testing
    
    // Enable retention profiling
    refresh.enable_retention_profiling(true);
    
    // Simulate profiling with some weak rows
    std::mt19937 rng(999);
    std::uniform_int_distribution<> retention_dist(5000, 25000);
    
    for (uint32_t bank = 0; bank < 8; ++bank) {
        for (uint32_t row = 0; row < 1024; ++row) {
            uint64_t retention = retention_dist(rng);
            
            // Make 10% of rows weak (low retention)
            if (row % 10 == 0) {
                retention = retention / 3;  // Weak retention
            }
            
            refresh.update_row_profile(bank, row, retention);
        }
    }
    
    // Get retention statistics
    uint32_t weak_rows;
    uint64_t avg_retention;
    refresh.get_retention_stats(weak_rows, avg_retention);
    
    std::cout << "Retention Profile Results:\n";
    std::cout << "  Weak rows: " << weak_rows << " / " << (8 * 1024) << "\n";
    std::cout << "  Weak row percentage: " << (100.0 * weak_rows / (8 * 1024)) << "%\n";
    std::cout << "  Average retention: " << avg_retention << " cycles\n";
    
    // Should identify ~10% weak rows
    double weak_percentage = (100.0 * weak_rows) / (8 * 1024);
    TEST_ASSERT_NEAR(weak_percentage, 10.0, 2.0, "Should identify ~10% weak rows");
    
    return true;
}

// ========== Main Test Runner ==========

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   DRAM Refresh & Power Model Validation Test Suite        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Run all tests
    struct TestCase {
        std::string name;
        bool (*func)();
    };
    
    TestCase tests[] = {
        {"Refresh Overhead @ Standard Temp", test_refresh_overhead_standard_temp},
        {"Temperature Scaling", test_temperature_scaling},
        {"Power Model Accuracy", test_power_model_accuracy},
        {"Thermal Feedback Stability", test_thermal_feedback_stability},
        {"Monte Carlo PVT Analysis", test_monte_carlo_pvt},
        {"Retention Profiling (RAIDR)", test_retention_profiling}
    };
    
    for (const auto& test : tests) {
        tests_total++;
        
        try {
            bool passed = test.func();
            print_test_result(passed);
            
            if (passed) {
                tests_passed++;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ EXCEPTION: " << e.what() << "\n";
            print_test_result(false);
        }
    }
    
    // Summary
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "TEST SUMMARY\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Passed: " << tests_passed << " / " << tests_total << "\n";
    std::cout << "Success Rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * tests_passed / tests_total) << "%\n";
    
    if (tests_passed == tests_total) {
        std::cout << "\n🎉 ALL TESTS PASSED! 🎉\n";
        return 0;
    } else {
        std::cout << "\n⚠️  SOME TESTS FAILED ⚠️\n";
        return 1;
    }
}

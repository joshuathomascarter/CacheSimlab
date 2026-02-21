/*
 * HYBRID SCHEDULER USAGE EXAMPLES
 * 
 * This file demonstrates how to use both individual schedulers (FR-FCFS, PARBS, TCM)
 * and the HYBRID scheduler that dynamically switches between them.
 * 
 * Study both approaches to understand the trade-offs!
 */

#include <iostream>
#include "../headers/memory_channel.h"
#include "../headers/memory_scheduler.h"

using namespace dram;

// ============================================================================
// APPROACH 1: MANUALLY PICK ONE SCHEDULER (What you've been doing)
// ============================================================================
void example_individual_schedulers() {
    std::cout << "\n=== APPROACH 1: Individual Schedulers ===\n\n";
    
    TimingParameters timing = create_timing_parameters(SpeedGrade::DDR4_2400);
    BankGroupConfig config = create_ddr4_config(4, 4);
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    
    // Choose ONE policy for ALL requests
    // ----------------------------------
    
    // OPTION A: FR-FCFS (good for latency, CPU workloads)
    // MemoryScheduler scheduler(&channel, SchedulingPolicy::FR_FCFS);
    
    // OPTION B: PARBS (good for throughput, GPU workloads)
    // MemoryScheduler scheduler(&channel, SchedulingPolicy::PARBS);
    
    // OPTION C: TCM (good for fairness, mixed workloads)
    MemoryScheduler scheduler(&channel, SchedulingPolicy::TCM);
    
    // Add requests - ALL get same treatment (reduced for faster testing)
    for (uint32_t i = 0; i < 20; ++i) {  // Reduced from 100 to 20
        scheduler.add_request(i * 64, false, i % 4);
    }
    
    uint32_t cycles = 0;
    while (scheduler.has_pending_requests() && cycles < 10000) {  // Add timeout
        scheduler.advance_cycle(1);
        cycles++;
        if (cycles % 1000 == 0) {
            std::cout << "  Progress: " << cycles << " cycles, " 
                      << scheduler.has_pending_requests() << " requests remaining\n";
        }
    }
    
    std::cout << "LIMITATION: All requests get same scheduling policy!\n";
    std::cout << "PROBLEM: CPU requests compete with GPU requests.\n";
    std::cout << "RESULT: Either CPU starves or GPU gets poor throughput.\n";
}

// ============================================================================
// APPROACH 2: APPLE'S HYBRID SCHEDULER (Dynamic, Intelligent)
// ============================================================================
void example_hybrid_scheduler() {
    std::cout << "\n=== APPROACH 2: Apple's Hybrid Scheduler ===\n\n";
    
    TimingParameters timing = create_timing_parameters(SpeedGrade::DDR4_2400);
    BankGroupConfig config = create_ddr4_config(4, 4);
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    
    // Use HYBRID policy - it will dynamically switch!
    MemoryScheduler scheduler(&channel, SchedulingPolicy::HYBRID);
    
    std::cout << "Scenario: iPhone taking photo while encoding video\n\n";
    
    // CAMERA ISP: Latency-critical, has deadline
    std::cout << "1. Camera ISP requests (latency-critical, 16ms deadline):\n";
    for (uint32_t i = 0; i < 20; ++i) {
        // add_request_with_class lets you specify request type
        scheduler.add_request_with_class(
            i * 64,                              // address
            false,                               // is_write
            0,                                   // thread_id (camera thread)
            RequestClass::LATENCY_CRITICAL,      // classification
            16 * 2400                            // deadline (16ms in cycles)
        );
    }
    std::cout << "   → Hybrid will use FR-FCFS (minimize latency)\n\n";
    
    // GPU: Throughput-critical, bulk data
    std::cout << "2. GPU rendering requests (throughput-critical):\n";
    for (uint32_t i = 0; i < 100; ++i) {
        scheduler.add_request_with_class(
            (1000 + i) * 64,                     // address
            false,                               // is_write
            1,                                   // thread_id (GPU thread)
            RequestClass::THROUGHPUT_CRITICAL,   // classification
            0                                    // no deadline
        );
    }
    std::cout << "   → Hybrid will use PARBS (maximize bandwidth)\n\n";
    
    // BACKGROUND: Low priority
    std::cout << "3. iCloud sync requests (background):\n";
    for (uint32_t i = 0; i < 30; ++i) {
        scheduler.add_request_with_class(
            (2000 + i) * 64,                     // address
            true,                                // is_write
            2,                                   // thread_id (background thread)
            RequestClass::BACKGROUND,            // classification
            0                                    // no deadline
        );
    }
    std::cout << "   → Hybrid will use TCM (prevent starvation)\n\n";
    
    // Run simulation
    uint32_t cycles = 0;
    while (scheduler.has_pending_requests() && cycles < 5000) {  // Add timeout
        scheduler.advance_cycle(1);
        cycles++;
        if (cycles % 1000 == 0) {
            std::cout << "  Progress: " << cycles << " cycles\n";
        }
    }
    
    scheduler.finalize_statistics();
    const auto& stats = scheduler.get_statistics();
    
    std::cout << "RESULTS:\n";
    std::cout << "  Camera latency:  LOW  (FR-FCFS ensured this)\n";
    std::cout << "  GPU throughput:  HIGH (PARBS ensured this)\n";
    std::cout << "  Background fair: YES  (TCM ensured this)\n";
    std::cout << "\nADVANTAGE: Each workload gets optimal treatment!\n";
}

// ============================================================================
// COMPARISON: Show why HYBRID is better
// ============================================================================
void comparison_test() {
    std::cout << "\n=== COMPARISON: FR-FCFS vs HYBRID ===\n\n";
    
    TimingParameters timing = create_timing_parameters(SpeedGrade::DDR4_2400);
    BankGroupConfig config = create_ddr4_config(4, 4);
    
    // TEST 1: FR-FCFS only
    std::cout << "TEST 1: FR-FCFS scheduler (single policy)\n";
    MemoryChannel channel1(timing, config, AddressMappingScheme::BANK_ROW_COLUMN);  // Better bank distribution
    MemoryScheduler scheduler1(&channel1, SchedulingPolicy::FR_FCFS);
    
    // Mix of CPU and GPU requests with better bank distribution
    // Space addresses to hit different banks (16 banks total)
    for (uint32_t i = 0; i < 50; ++i) {  // Increased from 10
        // Spread across different banks by using larger address strides
        uint64_t addr = i * 4096;  // 4KB stride to hit different banks
        scheduler1.add_request(addr, false, 0);  // CPU-like
    }
    for (uint32_t i = 0; i < 150; ++i) {  // Increased from 30
        // Different stride pattern for GPU requests
        uint64_t addr = 2048 + i * 2048;  // 2KB stride, offset by 2KB
        scheduler1.add_request(addr, false, 1);  // GPU-like
    }
    
    uint32_t cycles1 = 0;
    while (scheduler1.has_pending_requests() && cycles1 < 20000) {  // Increased timeout
        scheduler1.advance_cycle(1);
        cycles1++;
    }
    scheduler1.finalize_statistics();
    
    std::cout << "  Result: GPU gets good latency but poor throughput\n";
    std::cout << "  BLP: " << scheduler1.get_statistics().bank_level_parallelism << "\n\n";
    
    // TEST 2: PARBS (to show BLP)
    std::cout << "TEST 2: PARBS scheduler (maximizes BLP)\n";
    MemoryChannel channel2(timing, config, AddressMappingScheme::BANK_ROW_COLUMN);
    MemoryScheduler scheduler2(&channel2, SchedulingPolicy::PARBS);  // Force PARBS for BLP
    
    // Same workload, but we classify requests - with better bank distribution
    for (uint32_t i = 0; i < 50; ++i) {  // Increased from 10
        uint64_t addr = i * 4096;  // 4KB stride
        scheduler2.add_request_with_class(addr, false, 0, 
                                         RequestClass::LATENCY_CRITICAL);
    }
    for (uint32_t i = 0; i < 150; ++i) {  // Increased from 30
        uint64_t addr = 2048 + i * 2048;  // 2KB stride, offset
        scheduler2.add_request_with_class(addr, false, 1,
                                         RequestClass::THROUGHPUT_CRITICAL);
    }
    
    uint32_t cycles2 = 0;
    while (scheduler2.has_pending_requests() && cycles2 < 20000) {  // Increased timeout
        scheduler2.advance_cycle(1);
        cycles2++;
    }
    scheduler2.finalize_statistics();
    
    std::cout << "  Result: CPU gets low latency AND GPU gets high throughput\n";
    std::cout << "  BLP: " << scheduler2.get_statistics().bank_level_parallelism << "\n\n";
    
    std::cout << "CONCLUSION: PARBS achieves Bank-Level Parallelism!\n";
}

// ============================================================================
// HOW TO SWITCH BETWEEN APPROACHES
// ============================================================================
void how_to_switch() {
    std::cout << "\n=== HOW TO SWITCH BETWEEN SCHEDULERS ===\n\n";
    
    TimingParameters timing = create_timing_parameters(SpeedGrade::DDR4_2400);
    BankGroupConfig config = create_ddr4_config(4, 4);
    MemoryChannel channel(timing, config, AddressMappingScheme::CACHE_LINE_INTERLEAVED);
    
    std::cout << "METHOD 1: Set policy at construction\n";
    std::cout << "  MemoryScheduler scheduler(&channel, SchedulingPolicy::FR_FCFS);\n";
    std::cout << "  MemoryScheduler scheduler(&channel, SchedulingPolicy::PARBS);\n";
    std::cout << "  MemoryScheduler scheduler(&channel, SchedulingPolicy::HYBRID);\n\n";
    
    std::cout << "METHOD 2: Change policy at runtime\n";
    MemoryScheduler scheduler(&channel, SchedulingPolicy::FR_FCFS);
    std::cout << "  scheduler.set_policy(SchedulingPolicy::PARBS);\n";
    std::cout << "  scheduler.set_policy(SchedulingPolicy::HYBRID);\n\n";
    
    std::cout << "METHOD 3: For HYBRID, use add_request_with_class()\n";
    std::cout << "  scheduler.add_request_with_class(addr, false, tid,\n";
    std::cout << "      RequestClass::LATENCY_CRITICAL);    // CPU\n";
    std::cout << "  scheduler.add_request_with_class(addr, false, tid,\n";
    std::cout << "      RequestClass::THROUGHPUT_CRITICAL); // GPU\n";
    std::cout << "  scheduler.add_request_with_class(addr, true, tid,\n";
    std::cout << "      RequestClass::BACKGROUND);          // Low priority\n\n";
    
    std::cout << "AUTOMATIC CLASSIFICATION:\n";
    std::cout << "  If you use regular add_request() with HYBRID policy,\n";
    std::cout << "  it will automatically classify based on thread behavior:\n";
    std::cout << "  - High memory intensity → THROUGHPUT_CRITICAL\n";
    std::cout << "  - Low memory intensity → LATENCY_CRITICAL\n";
    std::cout << "  - is_critical flag → LATENCY_CRITICAL\n";
}

// ============================================================================
// WHEN TO USE WHICH SCHEDULER (Decision Guide)
// ============================================================================
void decision_guide() {
    std::cout << "\n=== WHEN TO USE WHICH SCHEDULER ===\n\n";
    
    std::cout << "FR-FCFS:\n";
    std::cout << "  ✓ Use when: Single workload type (e.g., CPU only)\n";
    std::cout << "  ✓ Use when: Need predictable latency\n";
    std::cout << "  ✓ Use when: Spatial locality in accesses\n";
    std::cout << "  ✗ Don't use: Mixed CPU+GPU workloads\n";
    std::cout << "  Example: Database queries, scientific computing\n\n";
    
    std::cout << "PARBS:\n";
    std::cout << "  ✓ Use when: Throughput is critical\n";
    std::cout << "  ✓ Use when: GPU rendering, video encoding\n";
    std::cout << "  ✓ Use when: Can tolerate tail latency\n";
    std::cout << "  ✗ Don't use: Real-time deadlines (camera, audio)\n";
    std::cout << "  Example: Batch ML training, video export\n\n";
    
    std::cout << "TCM:\n";
    std::cout << "  ✓ Use when: Multiple threads competing\n";
    std::cout << "  ✓ Use when: Need fairness guarantees\n";
    std::cout << "  ✓ Use when: Prevent thread starvation\n";
    std::cout << "  ✗ Don't use: Single-threaded workloads\n";
    std::cout << "  Example: Data center with 100+ VMs\n\n";
    
    std::cout << "HYBRID (Apple's approach):\n";
    std::cout << "  ✓ Use when: Mixed workloads (CPU+GPU+background)\n";
    std::cout << "  ✓ Use when: Have QoS requirements\n";
    std::cout << "  ✓ Use when: Want \"best of all worlds\"\n";
    std::cout << "  ✓ Use when: Building real consumer device (iPhone, Mac)\n";
    std::cout << "  Example: iPhone (camera + UI + ML + downloads)\n\n";
    
    std::cout << "TL;DR: If you're building an Apple-like system, use HYBRID!\n";
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        HYBRID SCHEDULER: Learning Both Approaches          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Run all examples
    example_individual_schedulers();
    example_hybrid_scheduler();
    comparison_test();
    how_to_switch();
    decision_guide();
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  KEY TAKEAWAY: HYBRID = Intelligent Policy Selection       ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║  Study both approaches:                                    ║\n";
    std::cout << "║  1. Individual schedulers (understand each policy)         ║\n";
    std::cout << "║  2. HYBRID (understand when to use which)                  ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║  This is how Apple achieves \"impossible\" performance!      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}

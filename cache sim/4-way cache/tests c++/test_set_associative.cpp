#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../include/set_associative_cache.h"

// ============================================================================
// Test Utilities
// ============================================================================

#define TEST(name) void name(); \
    struct name##_registrar { name##_registrar() { tests.push_back({#name, name}); } } name##_instance; \
    void name()

struct TestCase {
    std::string name;
    void (*func)();
};

std::vector<TestCase> tests;

void run_all_tests() {
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : tests) {
        std::cout << "Running: " << test.name << "... ";
        try {
            test.func();
            std::cout << "PASSED" << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "FAILED: " << e.what() << std::endl;
            failed++;
        }
    }
    
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Passed: " << passed << "/" << (passed + failed) << std::endl;
    if (failed > 0) {
        std::cout << "Failed: " << failed << std::endl;
    }
}

// ============================================================================
// Test Cases
// ============================================================================

/**
 * Test 1: Basic hit and miss behavior
 */
TEST(test_basic_hit_miss) {
    // Small cache: 256 bytes, 64-byte blocks, 4-way
    // = 4 lines total = 1 set
    SetAssociativeCache cache(256, 64, 4);
    
    // First access should be a miss
    auto r1 = cache.access(0x1000, AccessType::READ);
    assert(!r1.hit && "First access should miss");
    
    // Same block should hit
    auto r2 = cache.access(0x1004, AccessType::READ);
    assert(r2.hit && "Same block should hit");
    
    // Different block should miss
    auto r3 = cache.access(0x2000, AccessType::READ);
    assert(!r3.hit && "Different block should miss");
    
    auto stats = cache.get_stats();
    assert(stats.hits == 1);
    assert(stats.misses == 2);
}

/**
 * Test 2: Conflict misses in set-associative cache
 */
TEST(test_conflict_misses) {
    // 1KB cache, 64-byte blocks, 4-way = 4 sets
    // Addresses that map to same set: separated by 256 bytes (4 sets * 64 bytes)
    SetAssociativeCache cache(1024, 64, 4);
    
    // These all map to set 0 (assuming 64-byte blocks, 4 sets)
    uint64_t addrs[] = {0x000, 0x100, 0x200, 0x300, 0x400};
    
    // Fill all 4 ways of set 0
    for (int i = 0; i < 4; i++) {
        auto r = cache.access(addrs[i], AccessType::READ);
        assert(!r.hit && "Should all be misses initially");
    }
    
    // Access 5th address - should evict one
    auto r = cache.access(addrs[4], AccessType::READ);
    assert(!r.hit && "5th address should miss");
    assert(r.evicted && "Should evict a line");
    
    // The evicted address should now miss
    auto r2 = cache.access(addrs[0], AccessType::READ);
    assert(!r2.hit && "Evicted address should miss");
    
    cache.print_set_contents(0);
}

/**
 * Test 3: LRU replacement verification
 */
TEST(test_lru_ordering) {
    // 1KB cache, 64-byte blocks, 4-way = 4 sets
    SetAssociativeCache cache(1024, 64, 4);
    
    // Addresses mapping to set 0
    uint64_t a0 = 0x000;
    uint64_t a1 = 0x100;
    uint64_t a2 = 0x200;
    uint64_t a3 = 0x300;
    uint64_t a4 = 0x400;
    
    // Fill set 0: access order a0, a1, a2, a3
    // LRU order after: [a0, a1, a2, a3] (a0 is LRU)
    cache.access(a0, AccessType::READ);
    cache.access(a1, AccessType::READ);
    cache.access(a2, AccessType::READ);
    cache.access(a3, AccessType::READ);
    
    // Access a0 again - now a0 is MRU
    // LRU order: [a1, a2, a3, a0] (a1 is LRU)
    cache.access(a0, AccessType::READ);
    assert(cache.access(a0, AccessType::READ).hit && "a0 should still be present");
    
    // Access a4 - should evict a1 (the LRU)
    cache.access(a4, AccessType::READ);
    
    // a1 should be evicted
    assert(!cache.access(a1, AccessType::READ).hit && "a1 should be evicted");
    
    // a0, a2, a3 should still hit (after a1 miss loaded a1 again)
    cache.reset();
    cache.access(a0, AccessType::READ);
    cache.access(a1, AccessType::READ);
    cache.access(a2, AccessType::READ);
    cache.access(a3, AccessType::READ);
    cache.access(a0, AccessType::READ);  // Make a0 MRU
    cache.access(a4, AccessType::READ);  // Evict a1
    
    assert(cache.access(a0, AccessType::READ).hit && "a0 should hit");
    assert(cache.access(a2, AccessType::READ).hit && "a2 should hit");
    assert(cache.access(a3, AccessType::READ).hit && "a3 should hit");
}

/**
 * Test 4: Dirty eviction (write-back)
 */
TEST(test_dirty_eviction) {
    // 256 bytes, 64-byte blocks, 4-way = 1 set
    SetAssociativeCache cache(256, 64, 4);
    
    // Write to first 4 blocks (fills the set)
    cache.access(0x000, AccessType::WRITE);
    cache.access(0x100, AccessType::WRITE);
    cache.access(0x200, AccessType::WRITE);
    cache.access(0x300, AccessType::WRITE);
    
    // Access 5th block - should evict first (dirty) block
    auto r = cache.access(0x400, AccessType::READ);
    assert(r.evicted && "Should evict");
    assert(r.evicted_dirty && "Evicted line should be dirty");
    
    auto stats = cache.get_stats();
    assert(stats.dirty_evictions == 1);
}

/**
 * Test 5: Compare hit rates across associativities
 */
TEST(test_associativity_comparison) {
    // Trace that causes conflict misses in direct-mapped
    // Pattern: A, B, A, B, A, B (where A and B map to same set)
    
    // For 1KB cache with 64-byte blocks:
    // Direct-mapped: 16 sets (1024 / 64 = 16 lines)
    //   Set index = (addr >> 6) & 0xF
    //   0x0000 >> 6 = 0,  & 0xF = 0 (Set 0)
    //   0x0400 >> 6 = 16, & 0xF = 0 (Set 0) ← SAME SET! (conflict)
    //
    // 2-way: 8 sets (1024 / 64 / 2 = 8 sets)
    //   Set index = (addr >> 6) & 0x7
    //   0x0000 >> 6 = 0,  & 0x7 = 0 (Set 0)
    //   0x0400 >> 6 = 16, & 0x7 = 0 (Set 0) ← SAME SET! (conflict)
    //   But 2-way can hold 2 blocks, so no eviction
    
    std::vector<uint64_t> trace;
    uint64_t a = 0x0000;  // Address A
    uint64_t b = 0x0400;  // Address B (1024 bytes apart = conflicts in direct-mapped)
    
    // Create alternating access pattern
    for (int i = 0; i < 10; i++) {
        trace.push_back(a);
        trace.push_back(b);
    }
    
    // Test direct-mapped
    SetAssociativeCache direct(1024, 64, 1);
    for (auto addr : trace) {
        direct.access(addr, AccessType::READ);
    }
    auto direct_stats = direct.get_stats();
    
    // Test 2-way
    SetAssociativeCache two_way(1024, 64, 2);
    for (auto addr : trace) {
        two_way.access(addr, AccessType::READ);
    }
    auto two_stats = two_way.get_stats();
    
    std::cout << "\n  Direct-mapped: " << direct_stats.hits << " hits, " 
              << direct_stats.misses << " misses (hit rate: " 
              << (direct_stats.hit_rate() * 100) << "%)" << std::endl;
    std::cout << "  2-way: " << two_stats.hits << " hits, " 
              << two_stats.misses << " misses (hit rate: " 
              << (two_stats.hit_rate() * 100) << "%)" << std::endl;
    
    // 2-way should have better hit rate for this pattern
    assert(two_stats.hit_rate() > direct_stats.hit_rate() && 
           "2-way should beat direct-mapped for alternating pattern");
}

/**
 * Test 6: Address decoding correctness
 */
TEST(test_address_decoding) {
    // 8KB cache, 64-byte blocks, 4-way = 32 sets
    SetAssociativeCache cache(8192, 64, 4);
    
    // Verify different addresses go to different sets
    // With 32 sets and 64-byte blocks:
    // Set index = (addr >> 6) & 0x1F
    
    cache.access(0x0000, AccessType::READ);  // Set 0
    cache.access(0x0040, AccessType::READ);  // Set 1 (64 bytes later)
    cache.access(0x0080, AccessType::READ);  // Set 2 (128 bytes later)
    cache.access(0x07C0, AccessType::READ);  // Set 31 (31 * 64 = 1984)
    
    // Verify via print
    std::cout << "\n";
    cache.print_set_contents(0);
    cache.print_set_contents(1);
    cache.print_set_contents(31);
}

/**
 * Test 7: Large trace simulation
 */
TEST(test_sequential_access) {
    // 4KB cache, 64-byte blocks, 4-way
    SetAssociativeCache cache(4096, 64, 4);
    
    // Sequential access pattern - should have high hit rate due to spatial locality
    for (uint64_t addr = 0; addr < 1024; addr += 4) {
        cache.access(addr, AccessType::READ);
    }
    
    auto stats = cache.get_stats();
    std::cout << "\n  Sequential access: " << stats.hits << " hits, "
              << stats.misses << " misses (hit rate: "
              << (stats.hit_rate() * 100) << "%)" << std::endl;
    
    // With 64-byte blocks and 4-byte stride, we should hit 15/16 = 93.75%
    assert(stats.hit_rate() > 0.9 && "Sequential access should have high hit rate");
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Set-Associative Cache Tests ===" << std::endl << std::endl;
    run_all_tests();
    return 0;
}

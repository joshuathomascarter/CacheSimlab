#include "eviction_policies.hpp"
#include <random>
#include <algorithm>
#include <stdexcept>
#include <cassert>

// ============================================================================
// LRU Implementation
// ============================================================================

LRU::LRU(int num_ways) : EvictionPolicy(num_ways), clock(0) {
    counters.resize(num_ways, 0);
}

void LRU::access(int way) {
    counters[way] = clock;
    clock ++;
}

int LRU::get_victim() {
    int victim = 0;
    for (int i = 1; i < num_ways; ++i) {
        if (counters[i] < counters[victim]) {
            victim = i;
        }
        }
        return victim;
    }

void LRU::reset() {
    for (int i = 0; i < num_ways; ++i) {
        counters[i] = 0;
    }

    clock = 0;
}

// ============================================================================
// FIFO Implementation
// ============================================================================

FIFO::FIFO(int num_ways) : EvictionPolicy(num_ways), insertion_time(1) {
    // Start insertion_time at 1, so 0 means "never inserted"
    insertion_order.resize(num_ways, 0);
}

void FIFO::access(int way) {
    // Only mark insertion time on FIRST access
    if (insertion_order[way] != 0) {
        // Already inserted (has non-zero time), skip re-access
        return;
    }
    
    // First time accessing this way, mark it with current insertion time
    insertion_order[way] = insertion_time;
    insertion_time++;
}

int FIFO::get_victim() {
    int victim = 0;
    for (int i = 1; i < num_ways; ++i) {
        if (insertion_order[i] < insertion_order[victim]) {
            victim = i;
        }
    }
    return victim;
}

void FIFO::reset() {
    for (int i = 0; i < num_ways; ++i) {
        insertion_order[i] = 0;  // Reset to 0 (never inserted)
    }
    insertion_time = 1;  // Start at 1 again

}

// ============================================================================
// Random Implementation
// ============================================================================

Random::Random(int num_ways) : EvictionPolicy(num_ways) {
    rng.seed(std::random_device{}());
}

void Random::access(int way) {
    // Random replacement doesn't track accesses
    // Do nothing
}

int Random::get_victim() {
   std::uniform_int_distribution<int> dist(0, num_ways-1);
   return dist(rng);  
}

void Random::reset() {
    // Random replacement doesn't track accesses
    // Do nothing
}

// ============================================================================
// Pseudo-LRU Implementation
// ============================================================================

PseudoLRU::PseudoLRU(int num_ways) : EvictionPolicy(num_ways) {
    assert(num_ways == 4 || num_ways == 8 || num_ways == 16);
    
    num_bits = num_ways - 1;
    bits.resize(num_bits, 0);
    
    // Calculate max_depth once
    max_depth = 0;
    int temp = num_ways;
    while (temp > 1) {
        temp /= 2;
        max_depth++;
    }
}

void PseudoLRU::access(int way) {
    // Flip bits along path from leaf to root
    int bit_idx = 0;
    int pos = way;
    
    for (int level = 0; level < max_depth; level++) {
        int subtree_size = 1 << (max_depth - level - 1);
        bool is_right = (pos >= subtree_size);
        
        bits[bit_idx] = is_right ? 0 : 1;
        
        pos = is_right ? (pos - subtree_size) : pos;
        bit_idx = 2 * bit_idx + (is_right ? 2 : 1);
        
        if (bit_idx >= num_bits) {
            break;
        }
    }
}

int PseudoLRU::get_victim() {
    // Traverse tree using bits to find victim
    int bit_idx = 0;
    int victim = 0;
    
    for (int level = 0; level < max_depth; level++) {
        int go_right = bits[bit_idx];
        int subtree_size = 1 << (max_depth - level - 1);
        
        if (go_right) {
            victim += subtree_size;
        }
        
        bit_idx = 2 * bit_idx + (go_right ? 2 : 1);
        if (bit_idx >= num_bits) {
            break;
        }
    }
    
    return victim;
}

void PseudoLRU::reset() {
    for (int i = 0; i < num_bits; i++) {
        bits[i] = 0;
    }
}

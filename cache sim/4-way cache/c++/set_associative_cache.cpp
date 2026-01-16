#include "set_associative_cache.h"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Calculate log base 2 of n (n must be power of 2)
 */
size_t SetAssociativeCache::log2(size_t n) {
    size_t result = 0;
    while (n > 1) {
        n >>= 1;
        result++;
    }
    return result;
}

/**
 * Extract the block offset from an address
 * These are the low-order bits that identify a byte within a block
 */
uint64_t SetAssociativeCache::get_offset(uint64_t address) const {
    return address & ((1ULL << offset_bits) - 1);
}

/**
 * Extract the set index from an address
 * These bits identify which set the address maps to
 */
uint64_t SetAssociativeCache::get_set_index(uint64_t address) const {
    return (address >> offset_bits) & ((1ULL << index_bits) - 1);
}

/**
 * Extract the tag from an address
 * The remaining high-order bits after offset and index
 */
uint64_t SetAssociativeCache::get_tag(uint64_t address) const {
    return address >> (offset_bits + index_bits);
}

/**
 * Reconstruct an address from tag and set index (offset = 0)
 * Useful for write-back when evicting dirty lines
 */
uint64_t SetAssociativeCache::reconstruct_address(uint64_t tag, uint64_t set_index) const {
    return (tag << (offset_bits + index_bits)) | (set_index << offset_bits);
}

// ============================================================================
// Constructor
// ============================================================================

SetAssociativeCache::SetAssociativeCache(size_t size, size_t block, size_t assoc, size_t addr_bits)
    : cache_size(size), block_size(block), associativity(assoc) {
    
    // Validate parameters
    assert(size > 0 && "Cache size must be positive");
    assert(block > 0 && "Block size must be positive");
    assert(assoc > 0 && "Associativity must be positive");
    assert((block & (block - 1)) == 0 && "Block size must be power of 2");
    assert((assoc & (assoc - 1)) == 0 && "Associativity must be power of 2");
    
    // Calculate cache geometry
    num_lines = cache_size / block_size;
    num_sets = num_lines / associativity;
    
    assert(num_sets > 0 && "Must have at least one set");
    assert((num_sets & (num_sets - 1)) == 0 && "Number of sets must be power of 2");
    
    // Calculate address bit fields
    offset_bits = log2(block_size);
    index_bits = log2(num_sets);
    tag_bits = addr_bits - offset_bits - index_bits;
    
    // Initialize sets
    sets.reserve(num_sets);
    for (size_t i = 0; i < num_sets; i++) {
        sets.emplace_back(associativity);
    }
    
    // Print cache configuration
    std::cout << "=== Cache Configuration ===" << std::endl;
    std::cout << "Size: " << cache_size << " bytes" << std::endl;
    std::cout << "Block size: " << block_size << " bytes" << std::endl;
    std::cout << "Associativity: " << associativity << "-way" << std::endl;
    std::cout << "Number of lines: " << num_lines << std::endl;
    std::cout << "Number of sets: " << num_sets << std::endl;
    std::cout << "Address bits: " << addr_bits << std::endl;
    std::cout << "  Offset bits: " << offset_bits << std::endl;
    std::cout << "  Index bits: " << index_bits << std::endl;
    std::cout << "  Tag bits: " << tag_bits << std::endl;
    std::cout << "===========================" << std::endl << std::endl;
}

// ============================================================================
// Core Access Logic
// ============================================================================

AccessResult SetAssociativeCache::access(uint64_t address, AccessType type) {
    AccessResult result;
    
    // Update access type statistics
    if (type == AccessType::READ) {
        stats.reads++;
    } else {
        stats.writes++;
    }
    
    // Decode address
    uint64_t set_index = get_set_index(address);
    uint64_t tag = get_tag(address);
    result.set_index = set_index;
    
    CacheSet& set = sets[set_index];
    
    // Search for tag in the set
    int way = set.find_line(tag);
    
    if (way >= 0) {
        // ===== CACHE HIT =====
        result.hit = true;
        result.way = way;
        stats.hits++;
        
        // Update LRU (this line is now most recently used)
        set.update_lru(way);
        
        // If it's a write, mark the line as dirty
        if (type == AccessType::WRITE) {
            set.lines[way].dirty = true;
        }
    } else {
        // ===== CACHE MISS =====
        result.hit = false;
        stats.misses++;
        
        // Find a victim (empty line or LRU line)
        way = set.find_victim();
        result.way = way;
        
        CacheLine& victim = set.lines[way];
        
        // Check if we're evicting a valid line
        if (victim.valid) {
            result.evicted = true;
            result.evicted_tag = victim.tag;
            stats.evictions++;
            
            // Check if victim is dirty (needs write-back)
            if (victim.dirty) {
                result.evicted_dirty = true;
                stats.dirty_evictions++;
                // In a real system, we'd write back to memory here
                // uint64_t evicted_addr = reconstruct_address(victim.tag, set_index);
            }
        }
        
        // Load new block into the victim line
        victim.valid = true;
        victim.tag = tag;
        victim.dirty = (type == AccessType::WRITE);  // Dirty if write-allocate
        
        // Update LRU (this line is now most recently used)
        set.update_lru(way);
    }
    
    return result;
}

// ============================================================================
// Debug and Utility Functions
// ============================================================================

void SetAssociativeCache::print_set_contents(size_t set_idx) const {
    if (set_idx >= num_sets) {
        std::cout << "Invalid set index: " << set_idx << std::endl;
        return;
    }
    
    const CacheSet& set = sets[set_idx];
    std::cout << "Set " << set_idx << ":" << std::endl;
    
    for (size_t way = 0; way < associativity; way++) {
        const CacheLine& line = set.lines[way];
        std::cout << "  Way " << way << ": ";
        
        if (line.valid) {
            std::cout << "V";
        } else {
            std::cout << "-";
        }
        
        if (line.dirty) {
            std::cout << "D";
        } else {
            std::cout << "-";
        }
        
        std::cout << " Tag=0x" << std::hex << std::setw(8) << std::setfill('0') 
                  << line.tag << std::dec;
        
        if (line.valid) {
            uint64_t addr = reconstruct_address(line.tag, set_idx);
            std::cout << " (Addr=0x" << std::hex << addr << std::dec << ")";
        }
        
        std::cout << std::endl;
    }
    
    // Print LRU order
    std::vector<size_t> lru_order = set.get_lru_order();
    std::cout << "  LRU order: [";
    for (size_t i = 0; i < lru_order.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << lru_order[i];
    }
    std::cout << "] (left=LRU, right=MRU)" << std::endl;
}

void SetAssociativeCache::print_all_contents() const {
    std::cout << "=== Cache Contents ===" << std::endl;
    
    for (size_t set_idx = 0; set_idx < num_sets; set_idx++) {
        // Only print non-empty sets
        bool has_valid = false;
        for (size_t way = 0; way < associativity; way++) {
            if (sets[set_idx].lines[way].valid) {
                has_valid = true;
                break;
            }
        }
        
        if (has_valid) {
            print_set_contents(set_idx);
            std::cout << std::endl;
        }
    }
    
    std::cout << "======================" << std::endl;
}

CacheStats SetAssociativeCache::get_stats() const {
    return stats;
}

void SetAssociativeCache::reset() {
    // Reset all sets
    sets.clear();
    sets.reserve(num_sets);
    for (size_t i = 0; i < num_sets; i++) {
        sets.emplace_back(associativity);
    }
    
    // Reset statistics
    stats = CacheStats();
}

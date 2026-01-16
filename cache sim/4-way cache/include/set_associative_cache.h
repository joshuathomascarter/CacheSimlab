#ifndef SET_ASSOCIATIVE_CACHE_H
#define SET_ASSOCIATIVE_CACHE_H

#include <vector>
#include <cstdint>
#include <cstddef>
#include "cache_set.h"

/**
 * AccessType - Type of memory access
 */
enum class AccessType {
    READ,
    WRITE
};

/**
 * AccessResult - Result of a cache access operation
 */
struct AccessResult {
    bool hit;               // Was it a cache hit?
    bool evicted;           // Did we evict a line?
    bool evicted_dirty;     // Was evicted line dirty (needs write-back)?
    uint64_t evicted_tag;   // Tag of evicted line (for reconstructing address)
    size_t set_index;       // Which set was accessed
    int way;                // Which way was accessed/allocated
    
    AccessResult() 
        : hit(false), evicted(false), evicted_dirty(false), 
          evicted_tag(0), set_index(0), way(-1) {}
};

/**
 * CacheStats - Statistics for cache performance
 */
struct CacheStats {
    uint64_t hits;
    uint64_t misses;
    uint64_t reads;
    uint64_t writes;
    uint64_t evictions;
    uint64_t dirty_evictions;   // Write-backs required
    
    CacheStats() 
        : hits(0), misses(0), reads(0), writes(0), 
          evictions(0), dirty_evictions(0) {}
    
    double hit_rate() const {
        uint64_t total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / total : 0.0;
    }
};

/**
 * SetAssociativeCache - N-way set-associative cache simulator
 * 
 * Address breakdown:
 * ┌──────────────────┬─────────────┬──────────────────┐
 * │       TAG        │    INDEX    │     OFFSET       │
 * │   (remaining)    │ (set bits)  │ (block bits)     │
 * └──────────────────┴─────────────┴──────────────────┘
 */
class SetAssociativeCache {
private:
    size_t cache_size;          // Total cache size in bytes
    size_t block_size;          // Block (line) size in bytes
    size_t associativity;       // Number of ways per set
    size_t num_sets;            // Number of sets
    size_t num_lines;           // Total number of cache lines
    
    size_t offset_bits;         // Number of bits for block offset
    size_t index_bits;          // Number of bits for set index
    size_t tag_bits;            // Number of bits for tag
    
    std::vector<CacheSet> sets; // The cache sets
    CacheStats stats;           // Performance statistics

    // Helper functions
    uint64_t get_offset(uint64_t address) const;
    uint64_t get_set_index(uint64_t address) const;
    uint64_t get_tag(uint64_t address) const;
    uint64_t reconstruct_address(uint64_t tag, uint64_t set_index) const;
    static size_t log2(size_t n);

public:
    /**
     * Construct a set-associative cache
     * @param size Total cache size in bytes
     * @param block Block size in bytes (must be power of 2)
     * @param assoc Associativity (1 = direct-mapped, N = N-way)
     * @param addr_bits Address size in bits (default 32)
     */
    SetAssociativeCache(size_t size, size_t block, size_t assoc, size_t addr_bits = 32);
    
    /**
     * Access the cache (read or write)
     * @param address Memory address to access
     * @param type Read or write access
     * @return AccessResult with hit/miss info and eviction details
     */
    AccessResult access(uint64_t address, AccessType type);
    
    /**
     * Print contents of a specific set (for debugging)
     * @param set_idx Set index to display
     */
    void print_set_contents(size_t set_idx) const;
    
    /**
     * Print all non-empty cache contents
     */
    void print_all_contents() const;
    
    /**
     * Get cache statistics
     */
    CacheStats get_stats() const;
    
    /**
     * Reset cache to initial state
     */
    void reset();
    
    // Getters for cache parameters
    size_t get_cache_size() const { return cache_size; }
    size_t get_block_size() const { return block_size; }
    size_t get_associativity() const { return associativity; }
    size_t get_num_sets() const { return num_sets; }
    size_t get_num_lines() const { return num_lines; }
    size_t get_offset_bits() const { return offset_bits; }
    size_t get_index_bits() const { return index_bits; }
    size_t get_tag_bits() const { return tag_bits; }
};

#endif // SET_ASSOCIATIVE_CACHE_H

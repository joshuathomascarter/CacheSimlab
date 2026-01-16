#ifndef CACHE_SET_H
#define CACHE_SET_H

#include <vector>
#include <cstdint>
#include "lru_tracker.h"

/**
 * CacheLine - Represents a single cache line
 */
struct CacheLine {
    bool valid;         // Is this line holding valid data?
    bool dirty;         // Has this line been written to? (for write-back)
    uint64_t tag;       // Tag portion of the address
    
    CacheLine() : valid(false), dirty(false), tag(0) {}
};

/**
 * CacheSet - Represents one set in a set-associative cache
 * 
 * A set contains multiple "ways" (cache lines). For a 4-way cache,
 * each set has 4 lines that can hold different blocks mapping to the same set.
 */
class CacheSet {
private:
    size_t num_ways;
    LRUTracker lru;

public:
    std::vector<CacheLine> lines;  // The cache lines (ways) in this set

    /**
     * Initialize a cache set with given associativity
     * @param ways Number of ways (lines) in this set
     */
    explicit CacheSet(size_t ways) 
        : num_ways(ways), lru(ways), lines(ways) {}

    /**
     * Search all ways for a matching tag
     * @param tag The tag to search for
     * @return Way index if found, -1 if not found
     */
    int find_line(uint64_t tag) const {
        for (size_t i = 0; i < num_ways; i++) {
            if (lines[i].valid && lines[i].tag == tag) {
                return static_cast<int>(i);
            }
        }
        return -1;  // Not found
    }

    /**
     * Find a victim way for eviction
     * Prefers invalid (empty) lines over LRU eviction
     * @return Way index to evict/use
     */
    int find_victim() {
        // First, look for an invalid (empty) line
        for (size_t i = 0; i < num_ways; i++) {
            if (!lines[i].valid) {
                return static_cast<int>(i);
            }
        }
        // All lines valid, use LRU
        return static_cast<int>(lru.get_victim());
    }

    /**
     * Update LRU state - mark way as most recently used
     * @param way The way that was just accessed
     */
    void update_lru(int way) {
        if (way >= 0 && static_cast<size_t>(way) < num_ways) {
            lru.access(static_cast<size_t>(way));
        }
    }

    /**
     * Get number of ways in this set
     */
    size_t get_num_ways() const {
        return num_ways;
    }

    /**
     * Get LRU order for debugging
     */
    std::vector<size_t> get_lru_order() const {
        return lru.get_order();
    }
};

#endif // CACHE_SET_H

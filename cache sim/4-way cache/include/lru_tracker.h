#ifndef LRU_TRACKER_H
#define LRU_TRACKER_H

#include <vector>
#include <cstdint>
#include <algorithm>

/**
 * LRUTracker - Tracks Least Recently Used ordering for cache ways
 * 
 * Uses a counter-based approach:
 * - Each way has a "last access time" (counter value when accessed)
 * - The way with the smallest counter is the LRU (least recently used)
 * - The way with the largest counter is the MRU (most recently used)
 */
class LRUTracker {
private:
    size_t num_ways;
    uint64_t access_counter;              // Global counter, increments on each access
    std::vector<uint64_t> last_access;    // Last access time for each way

public:
    /**
     * Initialize LRU tracker for given number of ways
     * @param ways Number of ways in the cache set (e.g., 4 for 4-way)
     */
    explicit LRUTracker(size_t ways) 
        : num_ways(ways), access_counter(0), last_access(ways, 0) {
        // Initialize with staggered values so way 0 is default victim
        for (size_t i = 0; i < ways; i++) {
            last_access[i] = i;
        }
        access_counter = ways;
    }

    /**
     * Mark a way as Most Recently Used
     * @param way The way index that was just accessed
     */
    void access(size_t way) {
        if (way < num_ways) {
            last_access[way] = access_counter++;
        }
    }

    /**
     * Get the index of the Least Recently Used way
     * @return The way index to evict
     */
    size_t get_victim() const {
        size_t victim = 0;
        uint64_t min_time = last_access[0];
        
        for (size_t i = 1; i < num_ways; i++) {
            if (last_access[i] < min_time) {
                min_time = last_access[i];
                victim = i;
            }
        }
        return victim;
    }

    /**
     * Get the LRU ordering for debugging
     * @return Vector of way indices ordered from LRU to MRU
     */
    std::vector<size_t> get_order() const {
        std::vector<std::pair<uint64_t, size_t>> times;
        for (size_t i = 0; i < num_ways; i++) {
            times.push_back({last_access[i], i});
        }
        std::sort(times.begin(), times.end());
        
        std::vector<size_t> order;
        for (const auto& p : times) {
            order.push_back(p.second);
        }
        return order;
    }

    /**
     * Reset all ways to initial state
     */
    void reset() {
        access_counter = num_ways;
        for (size_t i = 0; i < num_ways; i++) {
            last_access[i] = i;
        }
    }
};

#endif // LRU_TRACKER_H

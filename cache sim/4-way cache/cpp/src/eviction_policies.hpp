#ifndef EVICTION_POLICIES_HPP
#define EVICTION_POLICIES_HPP

#include <vector>
#include <cstdint>
#include <random>

// ============================================================================
// Abstract Base Class for Eviction Policies
// ============================================================================

class EvictionPolicy {
public:
    virtual ~EvictionPolicy() = default;
    
    /**
     * Mark a way as recently accessed.
     * Updates internal state to reflect that this way was just used.
     */
    virtual void access(int way) = 0;
    
    /**
     * Get the way to evict (victim selection).
     * Returns the index of the way that should be evicted.
     */
    virtual int get_victim() = 0;
    
    /**
     * Reset the policy state.
     * Clears all tracking information for a new cache line.
     */
    virtual void reset() = 0;
    
protected:
    int num_ways;
    EvictionPolicy(int num_ways) : num_ways(num_ways) {}
};

// ============================================================================
// LRU - Least Recently Used (Counter-based)
// ============================================================================

class LRU : public EvictionPolicy {
public:
    explicit LRU(int num_ways);
    ~LRU() = default;
    
    void access(int way) override;
    int get_victim() override;
    void reset() override;
    
private:
    std::vector<uint32_t> counters;
    uint32_t clock;
};

// ============================================================================
// FIFO - First In, First Out
// ============================================================================

class FIFO : public EvictionPolicy {
public:
    explicit FIFO(int num_ways);
    ~FIFO() = default;
    
    void access(int way) override;
    int get_victim() override;
    void reset() override;
    
private:
    std::vector<uint32_t> insertion_order;
    uint32_t insertion_time;
};

// ============================================================================
// Random - Random Replacement
// ============================================================================

class Random : public EvictionPolicy {
public:
    explicit Random(int num_ways);
    ~Random() = default;
    
    void access(int way) override;
    int get_victim() override;
    void reset() override;
    
private:
    std::mt19937 rng;  // Mersenne Twister random number generator
};

// ============================================================================
// Pseudo-LRU (pLRU) - Bit Tree Approximation
// ============================================================================

class PseudoLRU : public EvictionPolicy {
public:
    /**
     * Pseudo-LRU using a binary tree of bits.
     * For 4-way cache, uses 3 bits to approximate LRU behavior.
     * 
     *     bit0
     *     /  \
     *   bit1  bit2
     *   / \    / \
     *  W0 W1  W2 W3
     */
    explicit PseudoLRU(int num_ways);
    ~PseudoLRU() = default;
    
    void access(int way) override;
    int get_victim() override;
    void reset() override;
    
private:
    std::vector<int> bits;
    int num_bits;
    int max_depth;  // Calculate once, reuse
};

#endif // EVICTION_POLICIES_HPP

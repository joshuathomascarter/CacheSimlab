#pragma once

#include "../../memory system simulator/c++/config.h"
#include "../../memory system simulator/c++/statistics.h"
#include "../../memory system simulator/c++/types.h"
#include "cache_line.h"
#include <cstdint>
#include <vector>

namespace memsim {

/**
 * AccessResult - Result of a cache access operation
 */
struct AccessResult {
  bool hit;      // Was it a cache hit?
  Cycle latency; // Latency in cycles for this access

  AccessResult(bool hit, Cycle latency) : hit(hit), latency(latency) {}
};

/**
 * DirectMappedCache - Simulates a direct-mapped cache
 *
 * In a direct-mapped cache, each memory address maps to exactly ONE cache line.
 * Formula: cache_index = (address / block_size) % num_cache_lines
 *
 * This is like a hash table with no collision handling - if two addresses
 * map to the same index, they evict each other (conflict miss).
 */
class DirectMappedCache {
public:
  /**
   * Constructor
   * @param config Cache configuration (size, block size, etc.)
   * @param cache_latency Latency in cycles for cache hit
   * @param memory_latency Latency in cycles for memory access (miss)
   */
  DirectMappedCache(const CacheConfig &config, Cycle cache_latency = 1,
                    Cycle memory_latency = 100);

  /**
   * Access the cache (read or write)
   * @param addr Memory address to access
   * @param type Access type (READ or WRITE)
   * @return AccessResult with hit/miss status and latency
   */
  AccessResult access(Address addr, AccessType type);

  /**
   * Get cache statistics
   * @return Reference to statistics object
   */
  const Statistics &get_stats() const { return stats_; }

  /**
   * Print cache configuration
   */
  void print_config(std::ostream &out) const;

private:
  // Configuration
  CacheConfig config_;
  Cycle cache_latency_;  // Hit latency
  Cycle memory_latency_; // Miss latency

  // Cache storage
  std::vector<CacheLine> lines_;

  // Derived parameters (computed from config)
  uint32_t num_lines_;   // Number of cache lines
  uint32_t offset_bits_; // Number of bits for offset
  uint32_t index_bits_;  // Number of bits for index

  // Statistics
  Statistics stats_;

  // Current simulation cycle
  Cycle current_cycle_;

  /**
   * Extract tag bits from address
   * Tag identifies which memory block this address belongs to
   * @param addr Memory address
   * @return Tag bits
   */
  uint64_t extract_tag(Address addr) const;

  /**
   * Extract index bits from address
   * Index determines which cache line to use
   * @param addr Memory address
   * @return Cache line index
   */
  uint64_t extract_index(Address addr) const;

  /**
   * Extract offset bits from address
   * Offset determines byte position within the cache line
   * @param addr Memory address
   * @return Byte offset
   */
  uint64_t extract_offset(Address addr) const;

  /**
   * Evict a cache line (write back if dirty)
   * @param index Cache line index to evict
   */
  void evict(uint64_t index);

  /**
   * Compute log2 of a number (assumes power of 2)
   * @param n Number (must be power of 2)
   * @return log2(n)
   */
  static uint32_t log2(uint32_t n);
};

} // namespace memsim

#pragma once

#include <cstdint>
#include <vector>

namespace memsim {

/**
 * CacheLine - Represents a single cache line (slot) in the cache
 *
 * A cache line is the fundamental storage unit in a cache. It holds:
 * - Valid bit: Is this line currently holding valid data?
 * - Dirty bit: Has the data been modified (needs write-back)?
 * - Tag: Which memory block does this line represent?
 * - Data: The actual cached bytes
 * - Last access cycle: For LRU replacement policy
 */
struct CacheLine {
  // State flags
  bool valid; // Is this cache line valid?
  bool dirty; // Has this line been written to (needs write-back)?

  // Address information
  uint64_t tag; // Tag bits from the address (identifies which block)

  // Data storage
  std::vector<uint8_t> data; // The actual cached data bytes

  // Replacement policy tracking
  uint64_t last_access_cycle; // Last time this line was accessed (for LRU)

  /**
   * Default constructor - initializes to empty/invalid state
   */
  CacheLine() : valid(false), dirty(false), tag(0), last_access_cycle(0) {}

  /**
   * Constructor with block size - allocates data array
   * @param block_size Number of bytes in this cache line
   */
  explicit CacheLine(uint32_t block_size)
      : valid(false), dirty(false), tag(0), data(block_size, 0),
        last_access_cycle(0) {}

  /**
   * Reset this cache line to empty state
   * Used when evicting or invalidating a line
   */
  void reset() {
    valid = false;
    dirty = false;
    tag = 0;
    last_access_cycle = 0;
    // Don't clear data array - just mark as invalid
  }

  /**
   * Check if this line matches a given tag
   * @param query_tag Tag to check against
   * @return true if valid and tag matches
   */
  bool matches(uint64_t query_tag) const { return valid && (tag == query_tag); }
};

} // namespace memsim

#include "direct_mapped_cache.h"
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>

namespace memsim {

DirectMappedCache::DirectMappedCache(const CacheConfig &config,
                                     Cycle cache_latency, Cycle memory_latency)
    : config_(config), cache_latency_(cache_latency),
      memory_latency_(memory_latency), current_cycle_(0) {

  // Calculate number of cache lines
  // num_lines = (size_kb * 1024) / block_size
  num_lines_ = (config_.size_kb * 1024) / config_.block_size;

  // Calculate bit field sizes
  offset_bits_ = log2(config_.block_size);
  index_bits_ = log2(num_lines_);

  // Initialize cache lines
  lines_.reserve(num_lines_);
  for (uint32_t i = 0; i < num_lines_; ++i) {
    lines_.emplace_back(config_.block_size);
  }

  // Verify configuration
  assert(config_.block_size > 0 && "Block size must be positive");
  assert((config_.block_size & (config_.block_size - 1)) == 0 &&
         "Block size must be power of 2");
  assert(num_lines_ > 0 && "Must have at least one cache line");
}

uint64_t DirectMappedCache::extract_tag(Address addr) const {
  // Tag is the upper bits after removing index and offset
  // Shift right by (index_bits + offset_bits)
  return addr >> (index_bits_ + offset_bits_);
}

uint64_t DirectMappedCache::extract_index(Address addr) const {
  // Index is the middle bits
  // Shift right by offset_bits to remove offset
  // Then mask with (num_lines - 1) to keep only index bits
  return (addr >> offset_bits_) & (num_lines_ - 1);
}

uint64_t DirectMappedCache::extract_offset(Address addr) const {
  // Offset is the lowest bits
  // Mask with (block_size - 1) to keep only offset bits
  return addr & (config_.block_size - 1);
}

AccessResult DirectMappedCache::access(Address addr, AccessType type) {
  // Step 1: Decode the address
  uint64_t index = extract_index(addr);
  uint64_t tag = extract_tag(addr);
  uint64_t offset = extract_offset(addr);

  // Step 2: Look up the cache line
  CacheLine &line = lines_[index];

  // Step 3: Check for hit or miss
  bool is_hit = line.matches(tag);

  if (is_hit) {
    // CACHE HIT!
    // Update last access time (for LRU tracking)
    line.last_access_cycle = current_cycle_;

    // If it's a write, mark the line as dirty
    if (type == AccessType::WRITE) {
      line.dirty = true;
    }

    // Record statistics
    stats_.record_access(true, cache_latency_);

    // Advance simulation time
    current_cycle_ += cache_latency_;

    return AccessResult(true, cache_latency_);

  } else {
    // CACHE MISS!
    // Need to evict current line (if valid) and load new data

    // Step 4: Evict old line if necessary
    if (line.valid) {
      evict(index);
    }

    // Step 5: Load new data from memory (simulated)
    line.valid = true;
    line.tag = tag;
    line.last_access_cycle = current_cycle_;

    // If it's a write, mark as dirty
    // Otherwise, it's clean (just loaded from memory)
    line.dirty = (type == AccessType::WRITE);

    // Record statistics
    stats_.record_access(false, memory_latency_);

    // Advance simulation time
    current_cycle_ += memory_latency_;

    return AccessResult(false, memory_latency_);
  }
}

void DirectMappedCache::evict(uint64_t index) {
  CacheLine &line = lines_[index];

  // Only evict if line is valid
  if (!line.valid) {
    return;
  }

  // If line is dirty, we need to write it back to memory
  if (line.dirty) {
    // Simulate write-back to memory
    // In a real simulator, this would update memory state
    // Here we just account for the latency
    current_cycle_ += memory_latency_;

    // Note: We don't record this as a separate "access" in stats
    // It's part of the eviction overhead
  }

  // Invalidate the line
  line.reset();
}

void DirectMappedCache::print_config(std::ostream &out) const {
  out << "Direct-Mapped Cache Configuration:\n";
  out << "  Size: " << config_.size_kb << " KB\n";
  out << "  Block size: " << config_.block_size << " bytes\n";
  out << "  Number of lines: " << num_lines_ << "\n";
  out << "  Index bits: " << index_bits_ << "\n";
  out << "  Offset bits: " << offset_bits_ << "\n";
  out << "  Cache hit latency: " << cache_latency_ << " cycles\n";
  out << "  Memory miss latency: " << memory_latency_ << " cycles\n";
}

uint32_t DirectMappedCache::log2(uint32_t n) {
  // Calculate log2 of n (assumes n is power of 2)
  uint32_t result = 0;
  while (n > 1) {
    n >>= 1;
    result++;
  }
  return result;
}

} // namespace memsim

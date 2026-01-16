#pragma once

#include "types.h"
#include <cstdint>

namespace memsim {

// Cache configuration parameters
struct CacheConfig {
  uint32_t size_kb;
  uint32_t block_size;
  uint32_t associativity;

  CacheConfig() = default; // Allow empty creation
  CacheConfig(uint32_t size_kb, uint32_t block_size, uint32_t associativity)
      : size_kb(size_kb), block_size(block_size), associativity(associativity) {
  }
};

// DRAM timing and organization parameters
struct DRAMConfig {
  uint32_t banks;
  uint32_t tRCD;
  uint32_t tCAS;
  uint32_t tRP;
  uint32_t tRAS;

  DRAMConfig() = default; // Allow empty creation
  DRAMConfig(uint32_t banks, uint32_t tRCD, uint32_t tCAS, uint32_t tRP,
             uint32_t tRAS)
      : banks(banks), tRCD(tRCD), tCAS(tCAS), tRP(tRP), tRAS(tRAS) {}
};

// Top-level simulation configuration
struct SimConfig {
  CacheConfig l1_cache;
  DRAMConfig dram;

  SimConfig() = default; // Allow empty creation
  SimConfig(CacheConfig l1_cache, DRAMConfig dram)
      : l1_cache(l1_cache), dram(dram) {}
};

} // namespace memsim

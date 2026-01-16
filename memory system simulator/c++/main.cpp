#include "config.h"
#include "statistics.h"
#include "types.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
  std::cout << "Memory Simulator Starting..." << std::endl;

  // 1. Configuration - either from command line args or defaults
  uint32_t l1_size_kb = 32;
  uint32_t l1_block_size = 64;
  uint32_t l1_associativity = 8;
  uint32_t dram_banks = 16;
  uint32_t dram_tRCD = 14;
  uint32_t dram_tCAS = 14;
  uint32_t dram_tRP = 14;
  uint32_t dram_tRAS = 38;

  if (argc >= 9) {
    // Parse command line arguments
    l1_size_kb = std::atoi(argv[1]);
    l1_block_size = std::atoi(argv[2]);
    l1_associativity = std::atoi(argv[3]);
    dram_banks = std::atoi(argv[4]);
    dram_tRCD = std::atoi(argv[5]);
    dram_tCAS = std::atoi(argv[6]);
    dram_tRP = std::atoi(argv[7]);
    dram_tRAS = std::atoi(argv[8]);
  }

  memsim::CacheConfig l1_config(l1_size_kb, l1_block_size, l1_associativity);
  memsim::DRAMConfig dram_config(dram_banks, dram_tRCD, dram_tCAS, dram_tRP, dram_tRAS);

  memsim::SimConfig config(l1_config, dram_config);
  memsim::Statistics stats;

  // 2. Simulation Loop (Reading from stdin for now)
  // Format expectation: [Rw] [Address in Hex]
  // Example: R 0x12345678

  char access_char;
  memsim::Address addr;
  size_t line_count = 0;

  std::cout << "Reading trace from standard input (Ctrl+D to end)..."
            << std::endl;

  while (std::cin >> access_char >> std::hex >> addr) {
    memsim::AccessType type;
    if (access_char == 'R' || access_char == 'r') {
      type = memsim::AccessType::READ;
    } else if (access_char == 'W' || access_char == 'w') {
      type = memsim::AccessType::WRITE;
    } else {
      // Skip malformed lines or comments
      continue;
    }

    // TODO: Pass to MemorySystem here
    // For now, we simulate a "fake" result:
    // 90% Hit Rate (Random), 4 cycle hit, 100 cycle miss

    bool is_hit =
        (addr % 10 != 0); // Fake logic: addresses ending in 0 are misses
    memsim::Cycle latency = is_hit ? 4 : 100;

    stats.record_access(is_hit, latency);
    line_count++;
  }

  std::cout << "Processed " << std::dec << line_count << " requests."
            << std::endl;
  std::cout << "Simulation complete." << std::endl;

  stats.print_summary(std::cout);

  return 0;
}

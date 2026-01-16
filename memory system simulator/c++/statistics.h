#pragma once

#include "types.h"
#include <cstdint>
#include <iostream>

namespace memsim {

class Statistics {
public:
  Statistics() = default;

  void record_access(bool hit, Cycle latency);

  void print_summary(std::ostream &out) const;

private:
  uint64_t total_accesses_ = 0;
  uint64_t total_hits_ = 0;
  Cycle total_latency_ = 0;
};

} // namespace memsim

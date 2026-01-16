#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace memsim {

using Address = uint64_t;

using Cycle = uint64_t;

enum class AccessType { READ, WRITE };

struct MemoryRequest {
  Address addr;
  Cycle arrival_cycle;
  AccessType type;
  uint32_t size_bytes;

  MemoryRequest(Address addr, Cycle arrival_cycle, AccessType type,
                uint32_t size_bytes)
      : addr(addr), arrival_cycle(arrival_cycle), type(type),
        size_bytes(size_bytes) {}
};

inline std::ostream &operator<<(std::ostream &os, const AccessType &type) {
  switch (type) {
  case AccessType::READ:
    os << "READ";
    break;
  case AccessType::WRITE:
    os << "WRITE";
    break;
    // Easier to add new types later
  }
  return os;
}
// namespace memsim
} // namespace memsim
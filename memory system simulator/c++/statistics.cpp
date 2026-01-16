#include "statistics.h"
#include <iomanip>

namespace memsim {

void Statistics::record_access(bool hit, Cycle latency) {
  total_accesses_++;
  if (hit) {
    total_hits_++;
  }
  total_latency_ += latency;
}

void Statistics::print_summary(std::ostream &out) const {
  out << "=== Simulation Statistics ===" << std::endl;
  out << "Total Accesses: " << total_accesses_ << std::endl;
  out << "Total Hits:     " << total_hits_ << std::endl;
  out << "Total Latency:  " << total_latency_ << " cycles" << std::endl;

  double hit_rate = 0.0;
  double avg_latency = 0.0;

  if (total_accesses_ > 0) {
    hit_rate = static_cast<double>(total_hits_) / total_accesses_;
    avg_latency = static_cast<double>(total_latency_) / total_accesses_;

    out << "Hit Rate:       " << std::fixed << std::setprecision(2)
        << (hit_rate * 100.0) << "%" << std::endl;
    out << "Avg Latency:    " << avg_latency << " cycles" << std::endl;

  } else {
    out << "No accesses recorded." << std::endl;
  }
}

// namespace memsim
} // namespace memsim
#ifndef MEMORY_SCHEDULER_H
#define MEMORY_SCHEDULER_H

#include <vector>
#include <queue>
#include <deque>
#include <memory>
#include <functional>
#include <unordered_map>
#include "memory_channel.h"
#include "dram_timing.h"

namespace dram {

// Scheduling policies
enum class SchedulingPolicy {
    FCFS,           // First Come First Serve (baseline)
    FR_FCFS,        // First Ready, First Come First Serve (row-hit priority)
    PARBS,          // Parallelism-Aware Batch Scheduling
    TCM,            // Thread Cluster Memory Scheduling
    ATLAS,          // Adaptive per-Thread Least-Attained-Service
    HYBRID          // Dynamic policy switching (Apple's approach)
};

// Request classification for Hybrid scheduler (Apple-style QoS)
enum class RequestClass {
    LATENCY_CRITICAL,    // CPU, UI, camera, audio - needs low latency
    THROUGHPUT_CRITICAL, // GPU, video encode - needs high bandwidth
    BACKGROUND,          // Downloads, indexing - low priority
    UNCLASSIFIED         // Default
};

// Memory request
struct MemoryRequest {
    uint64_t id;
    uint64_t address;
    bool is_write;
    uint32_t thread_id;
    uint64_t arrival_cycle;
    uint64_t issue_cycle;
    uint64_t completion_cycle;
    
    // Decoded address components
    uint32_t bank_id;
    uint32_t bank_group;
    uint32_t row;
    uint32_t column;
    
    // Priority/scheduling metadata
    uint32_t priority;
    uint32_t batch_id;
    bool is_critical;  // QoS flag
    
    // Hybrid scheduler extensions (Apple-style)
    RequestClass request_class;  // Latency vs throughput critical
    uint32_t deadline_cycle;     // For real-time deadlines (camera, audio)
    
    MemoryRequest() : id(0), address(0), is_write(false), thread_id(0),
                      arrival_cycle(0), issue_cycle(0), completion_cycle(0),
                      bank_id(0), bank_group(0), row(0), column(0),
                      priority(0), batch_id(0), is_critical(false),
                      request_class(RequestClass::UNCLASSIFIED), deadline_cycle(0) {}
};

// Thread characteristics for TCM
struct ThreadCharacteristics {
    uint32_t thread_id;
    uint64_t total_requests;
    uint64_t memory_intensity;  // Requests per 1000 instructions
    double row_buffer_hit_rate;
    uint64_t total_service_cycles;
    bool is_low_intensity;
    
    ThreadCharacteristics() : thread_id(0), total_requests(0), memory_intensity(0),
                             row_buffer_hit_rate(0.0), total_service_cycles(0),
                             is_low_intensity(true) {}
};

// Batch for PARBS
struct Batch {
    uint32_t batch_id;
    std::vector<MemoryRequest*> requests;
    uint64_t creation_cycle;
    bool is_marked;  // For marking/draining mechanism
    
    Batch() : batch_id(0), creation_cycle(0), is_marked(false) {}
};

// Scheduler statistics
struct SchedulerStatistics {
    // Latency metrics
    std::vector<uint64_t> request_latencies;
    double average_latency;
    double median_latency;
    double percentile_95;
    double percentile_99;
    double percentile_999;
    
    // Throughput metrics
    uint64_t total_requests_served;
    uint64_t total_cycles;
    double throughput_gbps;
    
    // Fairness metrics
    std::unordered_map<uint32_t, uint64_t> per_thread_service_cycles;
    double fairness_index;  // Jain's fairness index
    uint64_t max_starvation_cycles;
    
    // Efficiency metrics
    double row_buffer_hit_rate;
    double bank_level_parallelism;
    double write_buffer_utilization;
    uint64_t bank_conflicts;
    
    SchedulerStatistics() : average_latency(0), median_latency(0),
                           percentile_95(0), percentile_99(0), percentile_999(0),
                           total_requests_served(0), total_cycles(0),
                           throughput_gbps(0), fairness_index(0),
                           max_starvation_cycles(0), row_buffer_hit_rate(0),
                           bank_level_parallelism(0), write_buffer_utilization(0),
                           bank_conflicts(0) {}
    
    void calculate_percentiles();
    void calculate_fairness();
};

// Memory scheduler managing request queues and policies
class MemoryScheduler {
private:
    // Memory channel reference
    MemoryChannel* channel_;
    
    // Scheduling policy
    SchedulingPolicy policy_;
    
    // Request queues
    std::deque<MemoryRequest> read_queue_;
    std::deque<MemoryRequest> write_queue_;
    std::deque<MemoryRequest> pending_queue_;  // Waiting for bank/timing
    
    // Write buffer management
    uint32_t write_queue_max_size_;
    uint32_t write_high_watermark_;  // Force write drain threshold
    uint32_t write_low_watermark_;   // Resume normal scheduling
    bool write_drain_mode_;
    
    // FR-FCFS state
    std::unordered_map<uint32_t, uint32_t> bank_open_rows_;  // Fast row-hit check
    
    // PARBS state
    std::vector<Batch> batches_;
    uint32_t current_batch_id_;
    uint32_t batch_size_threshold_;
    bool batch_formation_mode_;
    
    // TCM state
    std::unordered_map<uint32_t, ThreadCharacteristics> thread_stats_;
    std::vector<uint32_t> low_intensity_threads_;
    std::vector<uint32_t> high_intensity_threads_;
    uint32_t cluster_threshold_;  // Intensity threshold for clustering
    
    // ATLAS state
    std::unordered_map<uint32_t, uint64_t> attained_service_;
    uint32_t quantum_size_;
    
    // HYBRID scheduler state (Apple-style dynamic switching)
    // ---------------------------------------------------------
    // Separate queues per request class for intelligent routing
    std::deque<MemoryRequest> latency_critical_queue_;    // CPU, UI, camera
    std::deque<MemoryRequest> throughput_critical_queue_; // GPU, video encode
    std::deque<MemoryRequest> background_queue_;          // Low priority
    
    // Policy selection thresholds
    uint32_t latency_queue_threshold_;     // Switch to FR-FCFS if queue > this
    uint32_t throughput_queue_threshold_;  // Switch to PARBS if queue > this
    uint64_t last_policy_switch_cycle_;    // Prevent thrashing
    uint32_t policy_switch_cooldown_;      // Minimum cycles between switches
    
    // Active sub-policy for hybrid mode
    SchedulingPolicy active_hybrid_policy_;
    
    // Deadline tracking for real-time requests (camera ISP, audio)
    std::vector<MemoryRequest*> deadline_requests_;
    
    // Statistics
    SchedulerStatistics stats_;
    uint64_t current_cycle_;
    uint64_t next_request_id_;
    
    // Helper functions for each policy
    MemoryRequest* select_request_fcfs();
    MemoryRequest* select_request_fr_fcfs();
    MemoryRequest* select_request_parbs();
    MemoryRequest* select_request_tcm();
    MemoryRequest* select_request_atlas();
    MemoryRequest* select_request_hybrid();  // NEW: Apple-style hybrid
    
    // Helper functions
    bool is_row_hit(const MemoryRequest& req) const;
    bool is_ready(const MemoryRequest& req) const;
    void update_thread_characteristics(const MemoryRequest& req);
    void remove_completed_request(const MemoryRequest& req);
    void form_batch();
    void classify_threads();
    double calculate_priority_fr_fcfs(const MemoryRequest& req) const;
    
    // Hybrid scheduler helpers
    void classify_request(MemoryRequest& req);           // Classify by workload type
    void route_to_queue(MemoryRequest& req);             // Route to appropriate queue
    SchedulingPolicy select_active_policy();             // Dynamic policy selection
    MemoryRequest* get_highest_priority_request();      // Cross-queue selection
    bool check_deadline_violation(const MemoryRequest& req) const;
    
    // Queue management
    void move_pending_to_ready();
    bool check_write_drain();
    
public:
    MemoryScheduler(MemoryChannel* channel, SchedulingPolicy policy);
    ~MemoryScheduler() = default;
    
    // Request management
    void add_request(uint64_t address, bool is_write, uint32_t thread_id, bool is_critical = false);
    
    // HYBRID scheduler extensions
    void add_request_with_class(uint64_t address, bool is_write, uint32_t thread_id, 
                                RequestClass req_class, uint32_t deadline = 0);
    bool has_pending_requests() const;
    
    // Scheduling
    void tick();
    void advance_cycle(uint64_t cycles = 1);
    
    // Policy management
    void set_policy(SchedulingPolicy policy);
    SchedulingPolicy get_policy() const { return policy_; }
    
    // Configuration
    void set_write_watermarks(uint32_t high, uint32_t low);
    void set_batch_threshold(uint32_t threshold);
    void set_cluster_threshold(uint32_t intensity);
    
    // Statistics
    const SchedulerStatistics& get_statistics() const { return stats_; }
    void reset_statistics();
    void finalize_statistics();
    
    // Queue status
    size_t get_read_queue_size() const { return read_queue_.size(); }
    size_t get_write_queue_size() const { return write_queue_.size(); }
    size_t get_pending_queue_size() const { return pending_queue_.size(); }
    
    // Debug
    void print_queue_status() const;
    void export_latency_distribution(const std::string& filename) const;
};

// Helper functions for workload generation
namespace workload {
    // Generate streaming access pattern (video encode, sequential scan)
    std::vector<uint64_t> generate_streaming_pattern(uint64_t start_address,
                                                      uint64_t num_accesses,
                                                      uint32_t stride = 64);
    
    // Generate random access pattern (hash table, pointer chasing)
    std::vector<uint64_t> generate_random_pattern(uint64_t num_accesses,
                                                   uint64_t address_range);
    
    // Generate strided access pattern (matrix transpose)
    std::vector<uint64_t> generate_strided_pattern(uint64_t start_address,
                                                    uint64_t num_accesses,
                                                    uint32_t stride);
    
    // Mixed workload (realistic scenario)
    struct WorkloadMix {
        double streaming_ratio;
        double random_ratio;
        double strided_ratio;
    };
    
    std::vector<uint64_t> generate_mixed_pattern(uint64_t num_accesses,
                                                  const WorkloadMix& mix);
}

} // namespace dram

#endif // MEMORY_SCHEDULER_H

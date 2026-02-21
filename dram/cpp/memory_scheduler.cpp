#include "../headers/memory_scheduler.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <random>

namespace dram {

// SchedulerStatistics implementation
void SchedulerStatistics::calculate_percentiles() {
    if (request_latencies.empty()) {
        return;
    }
    
    std::vector<uint64_t> sorted_latencies = request_latencies;
    std::sort(sorted_latencies.begin(), sorted_latencies.end());
    
    size_t n = sorted_latencies.size();
    average_latency = 0;
    for (uint64_t lat : sorted_latencies) {
        average_latency += lat;
    }
    average_latency /= n;
    
    median_latency = sorted_latencies[n / 2];
    percentile_95 = sorted_latencies[static_cast<size_t>(n * 0.95)];
    percentile_99 = sorted_latencies[static_cast<size_t>(n * 0.99)];
    percentile_999 = sorted_latencies[static_cast<size_t>(n * 0.999)];
}

void SchedulerStatistics::calculate_fairness() {
    if (per_thread_service_cycles.empty()) {
        fairness_index = 0.0;
        return;
    }
    
    // Jain's Fairness Index: (sum(x_i))^2 / (n * sum(x_i^2))
    double sum = 0.0;
    double sum_squared = 0.0;
    
    for (const auto& [thread_id, cycles] : per_thread_service_cycles) {
        sum += cycles;
        sum_squared += cycles * cycles;
    }
    
    size_t n = per_thread_service_cycles.size();
    fairness_index = (sum * sum) / (n * sum_squared);
}

// MemoryScheduler implementation
MemoryScheduler::MemoryScheduler(MemoryChannel* channel, SchedulingPolicy policy)
    : channel_(channel),
      policy_(policy),
      write_queue_max_size_(64),
      write_high_watermark_(48),
      write_low_watermark_(16),
      write_drain_mode_(false),
      current_batch_id_(0),
      batch_size_threshold_(16),
      batch_formation_mode_(true),
      cluster_threshold_(100),
      quantum_size_(1000),
      current_cycle_(0),
      next_request_id_(0),
      // Hybrid scheduler initialization
      latency_queue_threshold_(8),
      throughput_queue_threshold_(32),
      last_policy_switch_cycle_(0),
      policy_switch_cooldown_(1000),
      active_hybrid_policy_(SchedulingPolicy::FR_FCFS) {
}

void MemoryScheduler::add_request(uint64_t address, bool is_write, uint32_t thread_id, bool is_critical) {
    MemoryRequest req;
    req.id = next_request_id_++;
    req.address = address;
    req.is_write = is_write;
    req.thread_id = thread_id;
    req.arrival_cycle = current_cycle_;
    req.is_critical = is_critical;
    
    // Decode address
    PhysicalAddress phys_addr = channel_->decode_address(address);
    req.bank_id = phys_addr.bank * 4 + phys_addr.bank_group;  // Flat bank ID
    req.bank_group = phys_addr.bank_group;
    req.row = phys_addr.row;
    req.column = phys_addr.column;
    
    // Initialize thread statistics if needed
    if (thread_stats_.find(thread_id) == thread_stats_.end()) {
        ThreadCharacteristics tc;
        tc.thread_id = thread_id;
        thread_stats_[thread_id] = tc;
    }
    
    // For HYBRID mode, classify and route to appropriate queue
    if (policy_ == SchedulingPolicy::HYBRID) {
        classify_request(req);
        route_to_queue(req);
    } else {
        // For non-hybrid modes, use traditional queues
        if (is_write) {
            write_queue_.push_back(req);
        } else {
            read_queue_.push_back(req);
        }
    }
}

bool MemoryScheduler::is_row_hit(const MemoryRequest& req) const {
    return channel_->is_row_hit(req.bank_id, req.row);
}

bool MemoryScheduler::is_ready(const MemoryRequest& req) const {
    // Check if command can be issued based on bank state
    if (req.is_write) {
        // For writes, need to activate row first if not a hit
        if (is_row_hit(req)) {
            return channel_->can_issue_command(CommandType::WRITE, req.bank_id, req.row);
        } else {
            BankState state = channel_->get_bank_state(req.bank_id);
            return (state == BankState::IDLE || state == BankState::ACTIVE);
        }
    } else {
        // For reads
        if (is_row_hit(req)) {
            return channel_->can_issue_command(CommandType::READ, req.bank_id, req.row);
        } else {
            BankState state = channel_->get_bank_state(req.bank_id);
            return (state == BankState::IDLE || state == BankState::ACTIVE);
        }
    }
}

double MemoryScheduler::calculate_priority_fr_fcfs(const MemoryRequest& req) const {
    double priority = 0.0;
    
    // Priority 1: Row hits (highest priority)
    if (is_row_hit(req)) {
        priority += 1000000.0;
    }
    
    // Priority 2: Age (older requests get priority)
    uint64_t age = current_cycle_ - req.arrival_cycle;
    priority += age;
    
    // Priority 3: Critical requests
    if (req.is_critical) {
        priority += 500000.0;
    }
    
    return priority;
}

MemoryRequest* MemoryScheduler::select_request_fcfs() {
    // Simple FCFS: oldest request first
    MemoryRequest* oldest = nullptr;
    std::deque<MemoryRequest>* selected_queue = nullptr;
    
    // Check write drain mode
    if (write_drain_mode_ && !write_queue_.empty()) {
        oldest = &write_queue_.front();
        selected_queue = &write_queue_;
    } else {
        // Prioritize reads
        if (!read_queue_.empty()) {
            oldest = &read_queue_.front();
            selected_queue = &read_queue_;
        } else if (!write_queue_.empty()) {
            oldest = &write_queue_.front();
            selected_queue = &write_queue_;
        }
    }
    
    if (oldest && is_ready(*oldest)) {
        return oldest;
    }
    
    return nullptr;
}

MemoryRequest* MemoryScheduler::select_request_fr_fcfs() {
    MemoryRequest* best = nullptr;
    double best_priority = -1.0;
    std::deque<MemoryRequest>* selected_queue = nullptr;
    size_t selected_index = 0;
    
    // Check write drain mode
    if (write_drain_mode_ && !write_queue_.empty()) {
        // Only consider writes during drain
        for (size_t i = 0; i < write_queue_.size(); ++i) {
            if (is_ready(write_queue_[i])) {
                double priority = calculate_priority_fr_fcfs(write_queue_[i]);
                if (priority > best_priority) {
                    best_priority = priority;
                    best = &write_queue_[i];
                    selected_queue = &write_queue_;
                    selected_index = i;
                }
            }
        }
    } else {
        // Normal mode: prioritize reads, then writes
        for (size_t i = 0; i < read_queue_.size(); ++i) {
            if (is_ready(read_queue_[i])) {
                double priority = calculate_priority_fr_fcfs(read_queue_[i]);
                if (priority > best_priority) {
                    best_priority = priority;
                    best = &read_queue_[i];
                    selected_queue = &read_queue_;
                    selected_index = i;
                }
            }
        }
        
        // Consider writes if no ready reads
        if (!best) {
            for (size_t i = 0; i < write_queue_.size(); ++i) {
                if (is_ready(write_queue_[i])) {
                    double priority = calculate_priority_fr_fcfs(write_queue_[i]);
                    if (priority > best_priority) {
                        best_priority = priority;
                        best = &write_queue_[i];
                        selected_queue = &write_queue_;
                        selected_index = i;
                    }
                }
            }
        }
    }
    
    return best;
}

void MemoryScheduler::form_batch() {
    if (read_queue_.empty() && write_queue_.empty()) {
        return;
    }
    
    Batch new_batch;
    new_batch.batch_id = current_batch_id_++;
    new_batch.creation_cycle = current_cycle_;
    
    // Add all current requests to batch
    for (auto& req : read_queue_) {
        req.batch_id = new_batch.batch_id;
        new_batch.requests.push_back(&req);
    }
    
    for (auto& req : write_queue_) {
        req.batch_id = new_batch.batch_id;
        new_batch.requests.push_back(&req);
    }
    
    if (!new_batch.requests.empty()) {
        batches_.push_back(std::move(new_batch));
        batch_formation_mode_ = false;
    }
}

MemoryRequest* MemoryScheduler::select_request_parbs() {
    // Form new batch if needed
    if (batch_formation_mode_ && 
        (read_queue_.size() + write_queue_.size()) >= batch_size_threshold_) {
        form_batch();
    }
    
    if (batches_.empty()) {
        return nullptr;
    }
    
    // Serve oldest batch
    Batch& current_batch = batches_.front();
    
    // Group requests by bank and prioritize row hits
    std::unordered_map<uint32_t, std::vector<MemoryRequest*>> bank_requests;
    
    for (auto* req : current_batch.requests) {
        if (req->completion_cycle == 0) {  // Not yet completed
            bank_requests[req->bank_id].push_back(req);
        }
    }
    
    // Find best request (maximize row hits within each bank)
    MemoryRequest* best = nullptr;
    double best_priority = -1.0;
    
    for (auto& [bank_id, requests] : bank_requests) {
        for (auto* req : requests) {
            if (is_ready(*req)) {
                double priority = 0.0;
                
                // Prioritize row hits strongly in PARBS
                if (is_row_hit(*req)) {
                    priority += 1000000.0;
                }
                
                // Then by bank ID (process banks in order for max parallelism)
                priority += (100000.0 / (bank_id + 1));
                
                // Then by age within batch
                priority += (current_cycle_ - req->arrival_cycle);
                
                if (priority > best_priority) {
                    best_priority = priority;
                    best = req;
                }
            }
        }
    }
    
    // Check if batch is complete
    bool batch_complete = true;
    for (auto* req : current_batch.requests) {
        if (req->completion_cycle == 0) {
            batch_complete = false;
            break;
        }
    }
    
    if (batch_complete) {
        batches_.erase(batches_.begin());
        batch_formation_mode_ = true;
    }
    
    return best;
}


void MemoryScheduler::classify_threads() {
    low_intensity_threads_.clear();
    high_intensity_threads_.clear();
    
    for (auto& [thread_id, stats] : thread_stats_) {
        if (stats.memory_intensity < cluster_threshold_) {
            stats.is_low_intensity = true;
            low_intensity_threads_.push_back(thread_id);
        } else {
            stats.is_low_intensity = false;
            high_intensity_threads_.push_back(thread_id);
        }
    }
}

MemoryRequest* MemoryScheduler::select_request_tcm() {
    // Classify threads periodically
    if (current_cycle_ % 10000 == 0) {
        classify_threads();
    }
    
    MemoryRequest* best = nullptr;
    double best_priority = -1.0;
    
    // Priority order: Low intensity cluster > High intensity cluster
    auto evaluate_request = [&](MemoryRequest& req) {
        if (!is_ready(req)) return;
        
        double priority = 0.0;
        
        // Highest priority: low intensity threads
        auto& stats = thread_stats_[req.thread_id];
        if (stats.is_low_intensity) {
            priority += 10000000.0;
        }
        
        // Row hits
        if (is_row_hit(req)) {
            priority += 1000000.0;
        }
        
        // Age
        priority += (current_cycle_ - req.arrival_cycle);
        
        // Critical requests
        if (req.is_critical) {
            priority += 5000000.0;
        }
        
        if (priority > best_priority) {
            best_priority = priority;
            best = &req;
        }
    };
    
    // Check reads first
    for (auto& req : read_queue_) {
        evaluate_request(req);
    }
    
    // Then writes (unless in drain mode)
    if (write_drain_mode_ || !best) {
        for (auto& req : write_queue_) {
            evaluate_request(req);
        }
    }
    
    return best;
}

MemoryRequest* MemoryScheduler::select_request_atlas() {
    // ATLAS: Give quantum to thread with least attained service
    MemoryRequest* best = nullptr;
    uint64_t min_service = UINT64_MAX;
    
    auto evaluate_request = [&](MemoryRequest& req) {
        if (!is_ready(req)) return;
        
        uint64_t service = attained_service_[req.thread_id];
        
        // Prefer row hits among least-attained threads
        if (service < min_service || 
            (service == min_service && is_row_hit(req))) {
            min_service = service;
            best = &req;
        }
    };
    
    for (auto& req : read_queue_) {
        evaluate_request(req);
    }
    
    if (!best || write_drain_mode_) {
        for (auto& req : write_queue_) {
            evaluate_request(req);
        }
    }
    
    return best;
}

// ============================================================================
// HYBRID SCHEDULER IMPLEMENTATION (Apple-style Dynamic Policy Switching)
// ============================================================================
//
// CONCEPT: Instead of using ONE policy for ALL requests, the hybrid scheduler:
// 1. Classifies incoming requests by workload type (CPU, GPU, background)
// 2. Routes them to separate queues
// 3. Dynamically picks the best scheduling policy based on queue states
// 4. Ensures latency-critical requests (camera, UI) never starve
//
// EXAMPLE: iPhone taking a photo while exporting video in background:
// - Camera ISP requests → latency_critical_queue → FR-FCFS (low latency)
// - Video export → throughput_critical_queue → PARBS (high bandwidth)
// - iCloud sync → background_queue → TCM (fairness, don't starve others)
//
// KEY INSIGHT: Different workloads need different optimization strategies!
// ============================================================================

void MemoryScheduler::classify_request(MemoryRequest& req) {
    // Automatic classification based on thread characteristics
    // In real system, OS would tag requests with QoS class
    
    if (req.request_class != RequestClass::UNCLASSIFIED) {
        return;  // Already classified by caller
    }
    
    // Heuristic classification based on thread behavior
    auto& tc = thread_stats_[req.thread_id];
    
    if (req.is_critical || req.deadline_cycle > 0) {
        // Explicit critical flag or deadline → latency-critical
        req.request_class = RequestClass::LATENCY_CRITICAL;
    }
    else if (tc.memory_intensity > 200) {
        // High memory intensity (>200 requests/1000 inst) → GPU-like
        req.request_class = RequestClass::THROUGHPUT_CRITICAL;
    }
    else if (tc.memory_intensity < 50) {
        // Low memory intensity → likely CPU or background
        if (tc.total_requests < 100) {
            req.request_class = RequestClass::LATENCY_CRITICAL;  // CPU
        } else {
            req.request_class = RequestClass::BACKGROUND;  // Background task
        }
    }
    else {
        // Medium intensity → default to latency-critical
        req.request_class = RequestClass::LATENCY_CRITICAL;
    }
}

void MemoryScheduler::route_to_queue(MemoryRequest& req) {
    // Route request to appropriate queue based on classification
    
    switch (req.request_class) {
        case RequestClass::LATENCY_CRITICAL:
            latency_critical_queue_.push_back(req);
            break;
            
        case RequestClass::THROUGHPUT_CRITICAL:
            throughput_critical_queue_.push_back(req);
            break;
            
        case RequestClass::BACKGROUND:
            background_queue_.push_back(req);
            break;
            
        case RequestClass::UNCLASSIFIED:
            // Fallback to default queue (reads/writes)
            if (req.is_write) {
                write_queue_.push_back(req);
            } else {
                read_queue_.push_back(req);
            }
            break;
    }
    
    // Track deadline requests separately for urgent handling
    if (req.deadline_cycle > 0) {
        deadline_requests_.push_back(&req);
    }
}

SchedulingPolicy MemoryScheduler::select_active_policy() {
    // DYNAMIC POLICY SELECTION: Choose best policy based on current state
    // This is the "intelligence" of Apple's memory controller!
    
    // Prevent policy thrashing - don't switch too frequently
    if (current_cycle_ - last_policy_switch_cycle_ < policy_switch_cooldown_) {
        return active_hybrid_policy_;  // Keep current policy
    }
    
    // Helper lambda to finalize policy selection
    auto finalize_policy = [&](SchedulingPolicy new_policy) {
        if (new_policy != active_hybrid_policy_) {
            last_policy_switch_cycle_ = current_cycle_;
        }
        return new_policy;
    };
    
    SchedulingPolicy new_policy = active_hybrid_policy_;
    
    // PRIORITY 1: Check for deadline violations (camera, audio, real-time)
    // If any request is close to deadline, use FR-FCFS for minimum latency
    for (auto* req : deadline_requests_) {
        if (req->completion_cycle == 0) {  // Not yet completed
            if (check_deadline_violation(*req)) {
                return finalize_policy(SchedulingPolicy::FR_FCFS);  // Urgent, return immediately
            }
        }
    }
    
    // PRIORITY 2: Latency-critical queue (CPU, UI, camera)
    // These need low latency, so use FR-FCFS (row-hit priority)
    if (latency_critical_queue_.size() >= latency_queue_threshold_) {
        return finalize_policy(SchedulingPolicy::FR_FCFS);
    }
    
    // PRIORITY 3: Throughput-critical queue (GPU, video encode)
    // These need high bandwidth, so use PARBS (maximize parallelism)
    if (throughput_critical_queue_.size() >= throughput_queue_threshold_) {
        return finalize_policy(SchedulingPolicy::PARBS);
    }
    
    // PRIORITY 4: Mixed workload with background tasks
    // Use TCM to prevent background from starving foreground
    if (background_queue_.size() > 0 && 
        (latency_critical_queue_.size() > 0 || throughput_critical_queue_.size() > 0)) {
        return finalize_policy(SchedulingPolicy::TCM);
    }
    
    // DEFAULT: Use FR-FCFS as general-purpose policy
    new_policy = SchedulingPolicy::FR_FCFS;
    
    return finalize_policy(new_policy);
}

bool MemoryScheduler::check_deadline_violation(const MemoryRequest& req) const {
    // Check if request is approaching its deadline
    if (req.deadline_cycle == 0) {
        return false;  // No deadline
    }
    
    // Conservative threshold: 80% of time to deadline
    uint64_t time_remaining = (req.deadline_cycle > current_cycle_) ? 
                              (req.deadline_cycle - current_cycle_) : 0;
    
    uint64_t time_budget = req.deadline_cycle - req.arrival_cycle;
    
    return (time_remaining < time_budget * 0.2);  // <20% time left = urgent!
}

MemoryRequest* MemoryScheduler::get_highest_priority_request() {
    // Select next request across all queues based on active policy
    
    MemoryRequest* selected = nullptr;
    double best_priority = -1.0;
    std::deque<MemoryRequest>* source_queue = nullptr;
    
    // Lambda to evaluate request priority
    auto evaluate = [&](MemoryRequest& req, std::deque<MemoryRequest>* queue, double base_priority) {
        if (!is_ready(req)) return;
        
        double priority = base_priority;
        
        // Apply active policy's priority rules
        switch (active_hybrid_policy_) {
            case SchedulingPolicy::FR_FCFS:
                priority += calculate_priority_fr_fcfs(req);
                break;
                
            case SchedulingPolicy::PARBS:
                if (is_row_hit(req)) {
                    priority += 1000000.0;
                }
                priority += (100000.0 / (req.bank_id + 1));
                break;
                
            case SchedulingPolicy::TCM: {
                auto& stats = thread_stats_[req.thread_id];
                if (stats.is_low_intensity) {
                    priority += 10000000.0;
                }
                if (is_row_hit(req)) {
                    priority += 1000000.0;
                }
                break;
            }
                
            default:
                priority += (current_cycle_ - req.arrival_cycle);
                break;
        }
        
        if (priority > best_priority) {
            best_priority = priority;
            selected = &req;
            source_queue = queue;
        }
    };
    
    // PRIORITY ORDER: Latency > Throughput > Background
    // This ensures UI/camera never starve behind GPU work
    
    // 1. Deadline requests (highest priority)
    for (auto* req : deadline_requests_) {
        if (req->completion_cycle == 0) {
            evaluate(*req, nullptr, 100000000.0);  // Massive priority boost
        }
    }
    
    // 2. Latency-critical queue (CPU, UI, camera)
    for (auto& req : latency_critical_queue_) {
        evaluate(req, &latency_critical_queue_, 10000000.0);
    }
    
    // 3. Throughput-critical queue (GPU, video encode)
    // Only if no urgent latency-critical requests
    if (!selected || best_priority < 10000000.0) {
        for (auto& req : throughput_critical_queue_) {
            evaluate(req, &throughput_critical_queue_, 1000000.0);
        }
    }
    
    // 4. Background queue (lowest priority)
    // Only if nothing more important
    if (!selected || best_priority < 1000000.0) {
        for (auto& req : background_queue_) {
            evaluate(req, &background_queue_, 100000.0);
        }
    }
    
    // 5. Fallback to default queues (for unclassified requests)
    if (!selected) {
        for (auto& req : read_queue_) {
            evaluate(req, &read_queue_, 10000.0);
        }
        for (auto& req : write_queue_) {
            evaluate(req, &write_queue_, 1000.0);
        }
    }
    
    return selected;
}

MemoryRequest* MemoryScheduler::select_request_hybrid() {
    // MAIN HYBRID SCHEDULER LOGIC
    // This implements Apple's multi-tier memory scheduling strategy
    
    // Step 1: Update active policy based on current queue states
    active_hybrid_policy_ = select_active_policy();
    
    // Step 2: Select highest priority request across all queues
    MemoryRequest* selected = get_highest_priority_request();
    
    return selected;
}

void MemoryScheduler::add_request_with_class(uint64_t address, bool is_write, 
                                             uint32_t thread_id, RequestClass req_class,
                                             uint32_t deadline) {
    // HYBRID SCHEDULER REQUEST INTERFACE
    // Allows caller to explicitly specify request class and deadline
    
    MemoryRequest req;
    req.id = next_request_id_++;
    req.address = address;
    req.is_write = is_write;
    req.thread_id = thread_id;
    req.arrival_cycle = current_cycle_;
    req.request_class = req_class;
    req.deadline_cycle = (deadline > 0) ? (current_cycle_ + deadline) : 0;
    
    // Set critical flag for latency-critical requests
    req.is_critical = (req_class == RequestClass::LATENCY_CRITICAL) || (deadline > 0);
    
    // Decode address
    PhysicalAddress phys_addr = channel_->decode_address(address);
    req.bank_id = phys_addr.bank * 4 + phys_addr.bank_group;
    req.bank_group = phys_addr.bank_group;
    req.row = phys_addr.row;
    req.column = phys_addr.column;
    
    // Initialize thread statistics if needed
    if (thread_stats_.find(thread_id) == thread_stats_.end()) {
        ThreadCharacteristics tc;
        tc.thread_id = thread_id;
        thread_stats_[thread_id] = tc;
    }
    
    // Route to appropriate queue
    route_to_queue(req);
}

// ============================================================================
// END OF HYBRID SCHEDULER
// ============================================================================

void MemoryScheduler::remove_completed_request(const MemoryRequest& req) {
    // Remove from appropriate queue
    if (req.is_write) {
        auto it = std::find_if(write_queue_.begin(), write_queue_.end(),
                              [&](const MemoryRequest& r) { return r.id == req.id; });
        if (it != write_queue_.end()) {
            write_queue_.erase(it);
        }
    } else {
        auto it = std::find_if(read_queue_.begin(), read_queue_.end(),
                              [&](const MemoryRequest& r) { return r.id == req.id; });
        if (it != read_queue_.end()) {
            read_queue_.erase(it);
        }
    }
    
    // Also check hybrid queues if in HYBRID mode
    if (policy_ == SchedulingPolicy::HYBRID) {
        auto remove_from_queue = [&](std::deque<MemoryRequest>& queue) {
            auto it = std::find_if(queue.begin(), queue.end(),
                                  [&](const MemoryRequest& r) { return r.id == req.id; });
            if (it != queue.end()) {
                queue.erase(it);
            }
        };
        
        remove_from_queue(latency_critical_queue_);
        remove_from_queue(throughput_critical_queue_);
        remove_from_queue(background_queue_);
        
        // Remove from deadline tracking
        deadline_requests_.erase(
            std::remove_if(deadline_requests_.begin(), deadline_requests_.end(),
                          [&](MemoryRequest* r) { return r->id == req.id; }),
            deadline_requests_.end()
        );
    }
}

bool MemoryScheduler::check_write_drain() {
    size_t write_size = write_queue_.size();
    
    if (!write_drain_mode_) {
        // Enter drain mode if above high watermark
        if (write_size >= write_high_watermark_) {
            write_drain_mode_ = true;
            return true;
        }
    } else {
        // Exit drain mode if below low watermark
        if (write_size <= write_low_watermark_) {
            write_drain_mode_ = false;
            return false;
        }
    }
    
    return write_drain_mode_;
}

void MemoryScheduler::update_thread_characteristics(const MemoryRequest& req) {
    auto& stats = thread_stats_[req.thread_id];
    stats.total_requests++;
    
    uint64_t service_cycles = req.completion_cycle - req.arrival_cycle;
    stats.total_service_cycles += service_cycles;
    
    // Update row buffer hit rate
    if (is_row_hit(req)) {
        stats.row_buffer_hit_rate = 
            (stats.row_buffer_hit_rate * (stats.total_requests - 1) + 1.0) / 
            stats.total_requests;
    } else {
        stats.row_buffer_hit_rate = 
            (stats.row_buffer_hit_rate * (stats.total_requests - 1)) / 
            stats.total_requests;
    }
    
    // Estimate memory intensity (simplified)
    // In real system, would get from performance counters
    if (stats.total_requests > 10) {
        stats.memory_intensity = stats.total_requests / 
                                 ((current_cycle_ - req.arrival_cycle + stats.total_service_cycles) / 1000);
    }
}

void MemoryScheduler::tick() {
    // Check write drain status
    check_write_drain();
    
    // Select request based on policy
    MemoryRequest* selected = nullptr;
    
    switch (policy_) {
        case SchedulingPolicy::FCFS:
            selected = select_request_fcfs();
            break;
        case SchedulingPolicy::FR_FCFS:
            selected = select_request_fr_fcfs();
            break;
        case SchedulingPolicy::PARBS:
            selected = select_request_parbs();
            break;
        case SchedulingPolicy::TCM:
            selected = select_request_tcm();
            break;
        case SchedulingPolicy::ATLAS:
            selected = select_request_atlas();
            break;
        case SchedulingPolicy::HYBRID:
            // *** APPLE-STYLE HYBRID SCHEDULER ***
            // Dynamically switches between FR-FCFS, PARBS, and TCM
            // based on workload classification and queue states
            selected = select_request_hybrid();
            break;
        default:
            selected = select_request_fr_fcfs();
            break;
    }
    
    if (selected) {
        // Try to issue command
        bool issued = false;
        
        // Need to activate row first?
        if (!is_row_hit(*selected)) {
            BankState state = channel_->get_bank_state(selected->bank_id);
            if (state == BankState::IDLE) {
                issued = channel_->issue_command(CommandType::ACTIVATE, 
                                                selected->bank_id, 
                                                selected->row, 
                                                selected->id);
            } else if (state == BankState::ACTIVE) {
                // Need to precharge first
                issued = channel_->issue_command(CommandType::PRECHARGE,
                                                selected->bank_id,
                                                channel_->get_open_row(selected->bank_id),
                                                selected->id);
            }
            // If issued is true here, we issued ACTIVATE/PRECHARGE but not the actual command
            // The request stays in queue for next cycle when bank will be ready
        } else {
            // Row hit - issue read/write directly
            CommandType cmd = selected->is_write ? CommandType::WRITE : CommandType::READ;
            issued = channel_->issue_command(cmd, selected->bank_id, selected->row, selected->id);
            
            if (issued) {
                // Mark request as issued and complete
                selected->issue_cycle = current_cycle_;
                selected->completion_cycle = current_cycle_;
                
                // Update statistics
                uint64_t latency = selected->completion_cycle - selected->arrival_cycle;
                stats_.request_latencies.push_back(latency);
                stats_.total_requests_served++;
                stats_.per_thread_service_cycles[selected->thread_id] += latency;
                
                // Check for starvation
                if (latency > stats_.max_starvation_cycles) {
                    stats_.max_starvation_cycles = latency;
                }
                
                // Update thread characteristics
                update_thread_characteristics(*selected);
                
                // Update attained service for ATLAS
                attained_service_[selected->thread_id]++;
                
                // Remove from appropriate queue
                remove_completed_request(*selected);
            }
        }
    }
    
    // Advance channel
    channel_->tick();
}

void MemoryScheduler::advance_cycle(uint64_t cycles) {
    for (uint64_t i = 0; i < cycles; ++i) {
        tick();
        current_cycle_++;
    }
}

bool MemoryScheduler::has_pending_requests() const {
    bool has_requests = !read_queue_.empty() || !write_queue_.empty() || !pending_queue_.empty();
    
    // Also check hybrid queues if in HYBRID mode
    if (policy_ == SchedulingPolicy::HYBRID) {
        has_requests = has_requests || 
                       !latency_critical_queue_.empty() || 
                       !throughput_critical_queue_.empty() || 
                       !background_queue_.empty();
    }
    
    return has_requests;
}

void MemoryScheduler::set_policy(SchedulingPolicy policy) {
    policy_ = policy;
}

void MemoryScheduler::set_write_watermarks(uint32_t high, uint32_t low) {
    write_high_watermark_ = high;
    write_low_watermark_ = low;
}

void MemoryScheduler::set_batch_threshold(uint32_t threshold) {
    batch_size_threshold_ = threshold;
}

void MemoryScheduler::set_cluster_threshold(uint32_t intensity) {
    cluster_threshold_ = intensity;
}

void MemoryScheduler::reset_statistics() {
    stats_ = SchedulerStatistics();
}

void MemoryScheduler::finalize_statistics() {
    stats_.total_cycles = current_cycle_;
    stats_.calculate_percentiles();
    stats_.calculate_fairness();
    
    // Calculate derived metrics
    const auto& blp_stats = channel_->get_blp_statistics();
    stats_.bank_level_parallelism = blp_stats.average_blp;
    stats_.bank_conflicts = blp_stats.bank_conflicts;
    
    // Calculate row buffer hit rate
    uint64_t total_hits = 0;
    for (const auto& [tid, tc] : thread_stats_) {
        total_hits += static_cast<uint64_t>(tc.row_buffer_hit_rate * tc.total_requests);
    }
    stats_.row_buffer_hit_rate = static_cast<double>(total_hits) / stats_.total_requests_served;
    
    // Calculate throughput (assuming 64-byte cache lines, 2400 MHz DRAM)
    double bytes_transferred = stats_.total_requests_served * 64.0;
    double seconds = stats_.total_cycles / 2400000000.0;
    stats_.throughput_gbps = (bytes_transferred / seconds) / 1e9;
}

void MemoryScheduler::print_queue_status() const {
    std::cout << "\n=== Scheduler Status (Cycle " << current_cycle_ << ") ===\n";
    std::cout << "Policy: ";
    switch (policy_) {
        case SchedulingPolicy::FCFS: std::cout << "FCFS\n"; break;
        case SchedulingPolicy::FR_FCFS: std::cout << "FR-FCFS\n"; break;
        case SchedulingPolicy::PARBS: std::cout << "PARBS\n"; break;
        case SchedulingPolicy::TCM: std::cout << "TCM\n"; break;
        case SchedulingPolicy::ATLAS: std::cout << "ATLAS\n"; break;
        case SchedulingPolicy::HYBRID:
            std::cout << "HYBRID (Active: ";
            switch (active_hybrid_policy_) {
                case SchedulingPolicy::FR_FCFS: std::cout << "FR-FCFS"; break;
                case SchedulingPolicy::PARBS: std::cout << "PARBS"; break;
                case SchedulingPolicy::TCM: std::cout << "TCM"; break;
                default: std::cout << "UNKNOWN"; break;
            }
            std::cout << ")\n";
            break;
        default: std::cout << "UNKNOWN\n"; break;
    }
    
    if (policy_ == SchedulingPolicy::HYBRID) {
        std::cout << "Latency-Critical Queue:   " << latency_critical_queue_.size() << " requests\n";
        std::cout << "Throughput-Critical Queue: " << throughput_critical_queue_.size() << " requests\n";
        std::cout << "Background Queue:         " << background_queue_.size() << " requests\n";
        std::cout << "Deadline Requests:        " << deadline_requests_.size() << " active\n";
    }
    
    std::cout << "Read Queue: " << read_queue_.size() << " requests\n";
    std::cout << "Write Queue: " << write_queue_.size() << " requests";
    if (write_drain_mode_) {
        std::cout << " [DRAIN MODE]";
    }
    std::cout << "\n";
    std::cout << "Pending Queue: " << pending_queue_.size() << " requests\n";
    
    if (policy_ == SchedulingPolicy::PARBS) {
        std::cout << "Active Batches: " << batches_.size() << "\n";
    }
    
    std::cout << "Requests Served: " << stats_.total_requests_served << "\n";
}

void MemoryScheduler::export_latency_distribution(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }
    
    file << "request_id,latency_cycles\n";
    for (size_t i = 0; i < stats_.request_latencies.size(); ++i) {
        file << i << "," << stats_.request_latencies[i] << "\n";
    }
    
    file.close();
}

// Workload generation helpers
namespace workload {

std::vector<uint64_t> generate_streaming_pattern(uint64_t start_address,
                                                  uint64_t num_accesses,
                                                  uint32_t stride) {
    std::vector<uint64_t> pattern;
    pattern.reserve(num_accesses);
    
    uint64_t addr = start_address;
    for (uint64_t i = 0; i < num_accesses; ++i) {
        pattern.push_back(addr);
        addr += stride;
    }
    
    return pattern;
}

std::vector<uint64_t> generate_random_pattern(uint64_t num_accesses,
                                               uint64_t address_range) {
    std::vector<uint64_t> pattern;
    pattern.reserve(num_accesses);
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, address_range);
    
    for (uint64_t i = 0; i < num_accesses; ++i) {
        pattern.push_back(dist(gen));
    }
    
    return pattern;
}

std::vector<uint64_t> generate_strided_pattern(uint64_t start_address,
                                                uint64_t num_accesses,
                                                uint32_t stride) {
    return generate_streaming_pattern(start_address, num_accesses, stride);
}

std::vector<uint64_t> generate_mixed_pattern(uint64_t num_accesses,
                                              const WorkloadMix& mix) {
    std::vector<uint64_t> pattern;
    pattern.reserve(num_accesses);
    
    uint64_t streaming_count = static_cast<uint64_t>(num_accesses * mix.streaming_ratio);
    uint64_t random_count = static_cast<uint64_t>(num_accesses * mix.random_ratio);
    uint64_t strided_count = num_accesses - streaming_count - random_count;
    
    auto streaming = generate_streaming_pattern(0, streaming_count, 64);
    auto random = generate_random_pattern(random_count, 1ULL << 30);
    auto strided = generate_strided_pattern(0, strided_count, 256);
    
    pattern.insert(pattern.end(), streaming.begin(), streaming.end());
    pattern.insert(pattern.end(), random.begin(), random.end());
    pattern.insert(pattern.end(), strided.begin(), strided.end());
    
    // Shuffle to mix patterns
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::shuffle(pattern.begin(), pattern.end(), gen);
    
    return pattern;
}

} // namespace workload

} // namespace dram

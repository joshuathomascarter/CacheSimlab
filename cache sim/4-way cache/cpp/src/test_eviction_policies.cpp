#include "eviction_policies.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>

// ============================================================================
// Trace Reader
// ============================================================================

/**
 * Read a trace file containing way access sequence.
 * Expected format: one way number per line (0-3 for 4-way)
 */
std::vector<int> read_trace(const std::string& filename) {
    std::vector<int> trace;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Could not open trace file: " + filename);
    }
    
    int way;
    while (file >> way) {
        if (way < 0 || way > 15) {  // Support up to 16-way
            std::cerr << "Warning: Invalid way number " << way << std::endl;
            continue;
        }
        trace.push_back(way);
    }
    
    file.close();
    return trace;
}

// ============================================================================
// Policy Tester
// ============================================================================

struct TestResult {
    std::string policy_name;
    int total_accesses;
    int evictions;
    std::vector<int> evicted_ways;
};

TestResult test_policy(std::unique_ptr<EvictionPolicy>& policy, 
                       const std::vector<int>& trace,
                       const std::string& policy_name) {
    TestResult result;
    result.policy_name = policy_name;
    result.total_accesses = trace.size();
    result.evictions = 0;
    
    for (int way : trace) {
        policy -> access(way);
        int victim = policy ->get_victim();
        result.evictions++;
        result.evicted_ways.push_back(victim);
    }
    
    return result;
}

// ============================================================================
// Output Formatting
// ============================================================================

void print_header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void print_results(const std::vector<TestResult>& results) {
    std::cout << std::left << std::setw(15) << "Policy"
              << std::setw(15) << "Accesses"
              << std::setw(15) << "Evictions"
              << "\n";
    std::cout << std::string(45, '-') << "\n";
    
    for (const auto& result : results) {
        std::cout << std::left << std::setw(15) << result.policy_name
                  << std::setw(15) << result.total_accesses
                  << std::setw(15) << result.evictions
                  << "\n";
    }
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <trace_file>\n";
        std::cerr << "Example: " << argv[0] << " traces/random_access.txt\n";
        return 1;
    }
    
    std::string trace_file = argv[1];
    
    try {
        // Read trace
        print_header("Reading trace from: " + trace_file);
        std::vector<int> trace = read_trace(trace_file);
        std::cout << "Loaded " << trace.size() << " accesses\n";
        
        // Create policies
        std::vector<std::pair<std::unique_ptr<EvictionPolicy>, std::string>> policies;
        policies.push_back({std::make_unique<LRU>(4), "LRU"});
        policies.push_back({std::make_unique<FIFO>(4), "FIFO"});
        policies.push_back({std::make_unique<Random>(4), "Random"});
        policies.push_back({std::make_unique<PseudoLRU>(4), "Pseudo-LRU"});
        
        // Test each policy
        print_header("Testing eviction policies");
        std::vector<TestResult> results;
        
        for (auto& [policy, name] : policies) {
            TestResult result = test_policy(policy, trace, name);
            results.push_back(result);
        }
        
        // Print results
        print_results(results);
        
        // Write results to output file for validation
        std::string output_file = "../results/cpp_results.txt";
        std::ofstream outfile(output_file);
        
        if (!outfile.is_open()) {
            throw std::runtime_error("Could not open output file: " + output_file);
        }
        
        // Write header
        outfile << std::left << std::setw(15) << "Policy"
                << std::setw(15) << "Accesses"
                << std::setw(15) << "Evictions"
                << "\n";
        outfile << std::string(45, '-') << "\n";
        
        // Write results
        for (const auto& result : results) {
            outfile << std::left << std::setw(15) << result.policy_name
                    << std::setw(15) << result.total_accesses
                    << std::setw(15) << result.evictions
                    << "\n";
        }
        
        // Write evicted ways detail (optional)
        outfile << "\n" << std::string(60, '=') << "\n";
        outfile << "Evicted Ways (in order)\n";
        outfile << std::string(60, '=') << "\n";
        
        for (const auto& result : results) {
            outfile << result.policy_name << ": ";
            for (int way : result.evicted_ways) {
                outfile << way << " ";
            }
            outfile << "\n";
        }
        
        outfile.close();
        
        std::cout << "\n✅ Results written to: " << output_file << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

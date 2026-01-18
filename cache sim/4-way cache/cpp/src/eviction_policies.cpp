#include "eviction_policies.hpp"
#include <cassert>

// ============================================================================
// LRU Implementation (Linked List)
// ============================================================================

LRU::LRU(int num_ways) : EvictionPolicy(num_ways) {
    // Create dummy nodes
    head = new Node(-1);
    tail = new Node(-1);
    head->next = tail;
    tail->prev = head;
    
    // Create a node for each way
    way_nodes.resize(num_ways);
    for (int i = 0; i < num_ways; i++) {
        way_nodes[i] = nullptr;  // Not inserted yet
    }
}

LRU::~LRU() {
    // Delete all nodes
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

void LRU::access(int way) {
    if (way_nodes[way] == nullptr) {
        // First access: create node and add to front
        Node* new_node = new Node(way);
        way_nodes[way] = new_node;
        add_to_front(new_node);
    } else {
        // Re-access: move to front
        remove_node(way_nodes[way]);
        add_to_front(way_nodes[way]);
    }
}

int LRU::get_victim() {
    // Victim is the node just before dummy_tail (least recently used)
    Node* victim_node = tail->prev;
    return victim_node->way;
}

void LRU::reset() {
    // Delete all real nodes
    Node* current = head->next;
    while (current != tail) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    
    // Reset list to just dummy nodes
    head->next = tail;
    tail->prev = head;
    
    // Reset way_nodes
    for (int i = 0; i < num_ways; i++) {
        way_nodes[i] = nullptr;
    }
}

void LRU::remove_node(Node* node) {
    // Unlink node from list
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void LRU::add_to_front(Node* node) {
    // Insert node right after dummy_head
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}

// ============================================================================
// FIFO Implementation
// ============================================================================

FIFO::FIFO(int num_ways) : EvictionPolicy(num_ways), insertion_time(1) {
    // Start insertion_time at 1, so 0 means "never inserted"
    insertion_order.resize(num_ways, 0);
}

void FIFO::access(int way) {
    // Only mark insertion time on FIRST access
    if (insertion_order[way] != 0) {
        // Already inserted (has non-zero time), skip re-access
        return;
    }
    
    // First time accessing this way, mark it with current insertion time
    insertion_order[way] = insertion_time;
    insertion_time++;
}

int FIFO::get_victim() {
    int victim = 0;
    for (int i = 1; i < num_ways; ++i) {
        if (insertion_order[i] < insertion_order[victim]) {
            victim = i;
        }
    }
    return victim;
}

void FIFO::reset() {
    for (int i = 0; i < num_ways; ++i) {
        insertion_order[i] = 0;  // Reset to 0 (never inserted)
    }
    insertion_time = 1;  // Start at 1 again

}

// ============================================================================
// Random Implementation
// ============================================================================

Random::Random(int num_ways) : EvictionPolicy(num_ways) {
    rng.seed(std::random_device{}());
}

void Random::access(int way) {
    // Random replacement doesn't track accesses
    // Do nothing
}

int Random::get_victim() {
   std::uniform_int_distribution<int> dist(0, num_ways-1);
   return dist(rng);  
}

void Random::reset() {
    // Random replacement doesn't track accesses
    // Do nothing
}

// ============================================================================
// Pseudo-LRU Implementation
// ============================================================================

PseudoLRU::PseudoLRU(int num_ways) : EvictionPolicy(num_ways) {
    assert(num_ways == 4 || num_ways == 8 || num_ways == 16);
    
    num_bits = num_ways - 1;
    bits.resize(num_bits, 0);
    
    // Calculate max_depth once
    max_depth = 0;
    int temp = num_ways;
    while (temp > 1) {
        temp /= 2;
        max_depth++;
    }
}

void PseudoLRU::access(int way) {
    // Flip bits along path from leaf to root
    int bit_idx = 0;
    int pos = way;
    
    for (int level = 0; level < max_depth; level++) {
        int subtree_size = 1 << (max_depth - level - 1);
        bool is_right = (pos >= subtree_size);
        
        bits[bit_idx] = is_right ? 0 : 1;
        
        pos = is_right ? (pos - subtree_size) : pos;
        bit_idx = 2 * bit_idx + (is_right ? 2 : 1);
        
        if (bit_idx >= num_bits) {
            break;
        }
    }
}

int PseudoLRU::get_victim() {
    // Traverse tree using bits to find victim
    int bit_idx = 0;
    int victim = 0;
    
    for (int level = 0; level < max_depth; level++) {
        int go_right = bits[bit_idx];
        int subtree_size = 1 << (max_depth - level - 1);
        
        if (go_right) {
            victim += subtree_size;
        }
        
        bit_idx = 2 * bit_idx + (go_right ? 2 : 1);
        if (bit_idx >= num_bits) {
            break;
        }
    }
    
    return victim;
}

void PseudoLRU::reset() {
    for (int i = 0; i < num_bits; i++) {
        bits[i] = 0;
    }
}

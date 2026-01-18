#!/usr/bin/env python3
"""
Cache Eviction Policy Simulator

Compares LRU, FIFO, Random, and Pseudo-LRU eviction policies
"""

from abc import ABC, abstractmethod
from typing import List
import random


class EvictionPolicy(ABC):
    """Abstract base class for eviction policies"""
    
    def __init__(self, num_ways: int):
        self.num_ways = num_ways
    
    @abstractmethod
    def access(self, way: int) -> None:
        """Mark a way as accessed"""
        pass
    
    @abstractmethod
    def get_victim(self) -> int:
        """Return the way index to evict"""
        pass
    
    @abstractmethod
    def reset(self) -> None:
        """Reset policy state"""
        pass


# ============================================================================
# LRU - Least Recently Used (Counter-based, like your actual cache)
# ============================================================================

class LRU(EvictionPolicy):
    """
    LRU using counter approach (matches your C++ implementation)
    """
    def __init__(self, num_ways: int):
        super().__init__(num_ways)
        self.access_counter = 0
        self.last_access = [0] * num_ways
        
        # Initialize with staggered values so way 0 is default victim
        for i in range(num_ways):
            self.last_access[i] = i
        self.access_counter = num_ways
    
    def access(self, way: int) -> None:
        """Mark way as most recently used"""
        if 0 <= way < self.num_ways:
            self.last_access[way] = self.access_counter
            self.access_counter += 1
    
    def get_victim(self) -> int:
        """Return way with minimum access counter (LRU)"""
        return min(range(self.num_ways), key=lambda w: self.last_access[w])
    
    def reset(self) -> None:
        """Reset to initial state"""
        self.access_counter = 0
        for i in range(self.num_ways):
            self.last_access[i] = i
        self.access_counter = self.num_ways


# ============================================================================
# FIFO - First In, First Out
# ============================================================================

class FIFO(EvictionPolicy):
    """
    FIFO eviction - evict the block loaded first
    Tracks insertion order, not access recency
    """
    def __init__(self, num_ways: int):
        super().__init__(num_ways)
        self.insertion_order = list(range(num_ways))  # Initial order
        self.next_victim_idx = 0  # Which position to evict next
    
    def access(self, way: int) -> None:
        """
        FIFO doesn't care about access patterns.
        We only track insertion order.
        """
        pass  # Do nothing on access
    
    def get_victim(self) -> int:
        """
        Return the way that's been in the cache longest.
        Rotate through ways in circular fashion.
        """
        victim_way = self.insertion_order[self.next_victim_idx]
        self.next_victim_idx = (self.next_victim_idx + 1) % self.num_ways
        return victim_way
    
    def reset(self) -> None:
        """Reset to initial state"""
        self.insertion_order = list(range(self.num_ways))
        self.next_victim_idx = 0


# ============================================================================
# Random - Random Replacement
# ============================================================================

class Random(EvictionPolicy):
    """
    Random eviction - pick any way at random
    Surprisingly effective for some workloads
    """
    def __init__(self, num_ways: int):
        super().__init__(num_ways)
    
    def access(self, way: int) -> None:
        """Random doesn't track accesses"""
        pass
    
    def get_victim(self) -> int:
        """Return a random way"""
        return random.randint(0, self.num_ways - 1)
    
    def reset(self) -> None:
        """Nothing to reset"""
        pass


# ============================================================================
# Pseudo-LRU (pLRU) - Approximation using bit tree
# ============================================================================

class PseudoLRU(EvictionPolicy):
    """
    Pseudo-LRU using a binary tree of bits.
    
    For 4-way cache, uses 3 bits to approximate LRU behavior:
    
        bit0
        /  \\
      bit1  bit2
      / \\   / \\
     W0 W1 W2 W3
    
    Each bit determines which subtree contains the vic
    tim.
    On access, flip bits along the path from leaf to root.
    """
    def __init__(self, num_ways: int):
        super().__init__(num_ways)
        assert num_ways in [2, 4, 8, 16], "pLRU only supports power-of-2 ways"
        
        self.num_bits = num_ways - 1
        self.bits = [0] * self.num_bits
    
    def access(self, way: int) -> None:
        """
        Update bits to mark this way as recently used.
        Flip bits along path from way to root.
        """
        # For 4-way, bits indexed 0-2 form a tree
        # Traverse from leaf (way) to root, flipping bits
        
        bit_idx = 0
        pos = way
        depth = 0
        
        # Determine tree depth
        max_depth = 0
        temp = self.num_ways
        while temp > 1:
            temp //= 2
            max_depth += 1
        
        # Flip bits along path
        for level in range(max_depth):
            # Determine which half of current subtree
            subtree_size = 1 << (max_depth - level - 1)
            is_right = (pos >= subtree_size)
            
            self.bits[bit_idx] = 1 if not is_right else 0
            
            pos = pos % subtree_size if not is_right else pos - subtree_size
            bit_idx = 2 * bit_idx + (2 if is_right else 1)
            
            if bit_idx >= self.num_bits:
                break
    
    def get_victim(self) -> int:
        """
        Traverse tree using bits to find victim.
        """
        bit_idx = 0
        victim = 0
        max_depth = 0
        temp = self.num_ways
        while temp > 1:
            temp //= 2
            max_depth += 1
        
        for level in range(max_depth):
            # Bit tells us which direction to go
            go_right = self.bits[bit_idx]
            subtree_size = 1 << (max_depth - level - 1)
            
            if go_right:
                victim += subtree_size
            
            bit_idx = 2 * bit_idx + (2 if go_right else 1)
            if bit_idx >= self.num_bits:
                break
        
        return victim
    
    def reset(self) -> None:
        """Reset all bits to 0"""
        self.bits = [0] * self.num_bits


# ============================================================================
# Cache Simulator with Pluggable Policy
# ============================================================================

class SimpleCache:
    """
    Simple 4-way cache simulator to test eviction policies.
    Doesn't implement full cache logic, just tracks hits/misses with policy.
    """
    def __init__(self, num_sets: int, associativity: int, policy: EvictionPolicy):
        self.num_sets = num_sets
        self.associativity = associativity
        self.policy = policy
        
        # Each set contains a list of tags and valid bits
        self.cache = [[set() for _ in range(associativity)] for _ in range(num_sets)]
        self.set_policies = [policy.__class__(associativity) for _ in range(num_sets)]
        
        self.hits = 0
        self.misses = 0
    
    def access(self, address: int, block_size: int = 64) -> bool:
        """
        Access an address. Returns True if hit, False if miss.
        """
        block = address // block_size
        set_idx = block % self.num_sets
        tag = block // self.num_sets
        
        # Check for hit
        for way in range(self.associativity):
            if tag in self.cache[set_idx][way]:
                self.hits += 1
                self.set_policies[set_idx].access(way)
                return True
        
        # Miss - need to evict
        self.misses += 1
        victim_way = self.set_policies[set_idx].get_victim()
        self.cache[set_idx][victim_way] = {tag}
        self.set_policies[set_idx].access(victim_way)
        
        return False
    
    def get_hit_rate(self) -> float:
        """Return hit rate as percentage"""
        total = self.hits + self.misses
        return (self.hits / total * 100) if total > 0 else 0


# ============================================================================
# Test Traces
# ============================================================================

def sequential_trace(num_accesses: int, block_size: int = 64) -> List[int]:
    """Generate sequential access pattern: 0, 64, 128, 192, ..."""
    return [i * block_size for i in range(num_accesses)]


def random_trace(num_accesses: int, max_address: int = 4096, block_size: int = 64) -> List[int]:
    """Generate random access pattern"""
    return [random.randint(0, max_address - 1) for _ in range(num_accesses)]


def locality_trace(num_accesses: int, num_working_set: int = 10, block_size: int = 64) -> List[int]:
    """
    Generate trace with temporal locality.
    Repeatedly access a small working set with occasional jumps.
    """
    trace = []
    for _ in range(num_accesses):
        if random.random() < 0.8:  # 80% stay in working set
            addr = random.randint(0, num_working_set - 1) * block_size
        else:  # 20% random jump
            addr = random.randint(0, 256) * block_size
        trace.append(addr)
    return trace


# ============================================================================
# Comparison
# ============================================================================

def compare_policies(trace: List[int], num_sets: int = 4, associativity: int = 4):
    """Compare all 4 eviction policies on the same trace"""
    
    policies = {
        "LRU":        LRU(associativity),
        "FIFO":       FIFO(associativity),
        "Random":     Random(associativity),
        "Pseudo-LRU": PseudoLRU(associativity),
    }
    
    caches = {
        name: SimpleCache(num_sets, associativity, policy)
        for name, policy in policies.items()
    }
    
    # Simulate all policies on the trace
    for address in trace:
        for cache in caches.values():
            cache.access(address)
    
    # Print results
    print(f"\n{'='*60}")
    print(f"Trace Type: {len(trace)} accesses, {num_sets} sets, {associativity}-way")
    print(f"{'='*60}")
    print(f"{'Policy':<15} {'Hits':<10} {'Misses':<10} {'Hit Rate':<10}")
    print(f"{'-'*60}")
    
    results = {}
    for name, cache in caches.items():
        hit_rate = cache.get_hit_rate()
        results[name] = hit_rate
        print(f"{name:<15} {cache.hits:<10} {cache.misses:<10} {hit_rate:>6.2f}%")
    
    return results


# ============================================================================
# Main - Run Comparisons
# ============================================================================

if __name__ == "__main__":
    print("\n🔴 TEST 1: SEQUENTIAL ACCESS")
    print("Expected: FIFO and LRU should tie (no re-access)")
    compare_policies(sequential_trace(100), num_sets=4, associativity=4)
    
    print("\n🟢 TEST 2: RANDOM ACCESS")
    print("Expected: LRU should win (temporal locality)")
    compare_policies(random_trace(200, max_address=1024), num_sets=4, associativity=4)
    
    print("\n🔵 TEST 3: LOCALITY (working set)")
    print("Expected: LRU and Pseudo-LRU should win (capture temporal locality)")
    compare_policies(locality_trace(300, num_working_set=8), num_sets=4, associativity=4)
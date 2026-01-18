#!/usr/bin/env python3
"""
Hand-trace cache behavior to generate expected results.

This simulates the cache step-by-step using the same logic as the C++ simulator,
so we know what the correct answers should be.

Cache configuration:
- 256 bytes total
- 64-byte blocks
- 4-way associative
- 1 set (256 / 64 / 4 = 1)
"""

from typing import List, Tuple

class SimpleValidationCache:
    """
    Simplified cache for validation (matches your C++ cache behavior).
    """
    
    def __init__(self, num_sets: int = 1, associativity: int = 4, 
                 block_size: int = 64):
        self.num_sets = num_sets
        self.associativity = associativity
        self.block_size = block_size
        
        # Cache storage: cache[set][way] = tag (or -1 if invalid)
        self.cache = [[-1] * associativity for _ in range(num_sets)]
        
        # LRU tracking: last_access[set][way] = counter
        self.last_access = [[i for i in range(associativity)] 
                            for _ in range(num_sets)]
        self.access_counter = associativity
        
        self.hits = 0
        self.misses = 0
        self.access_log = []
    
    def access(self, address: int) -> Tuple[bool, int, int, int]:
        """
        Access an address. Returns (is_hit, set_idx, way, tag).
        """
        block = address // self.block_size
        set_idx = block % self.num_sets
        tag = block // self.num_sets
        
        # Check for hit
        for way in range(self.associativity):
            if self.cache[set_idx][way] == tag:
                self.hits += 1
                # Update LRU
                self.last_access[set_idx][way] = self.access_counter
                self.access_counter += 1
                
                result = (True, set_idx, way, tag)
                self.access_log.append(result)
                return result
        
        # Miss - find victim
        self.misses += 1
        victim_way = min(range(self.associativity),
                        key=lambda w: self.last_access[set_idx][w])
        
        # Evict and load
        self.cache[set_idx][victim_way] = tag
        self.last_access[set_idx][victim_way] = self.access_counter
        self.access_counter += 1
        
        result = (False, set_idx, way, tag)
        self.access_log.append(result)
        return result
    
    def get_cache_state(self) -> str:
        """Return current cache state as string."""
        lines = []
        lines.append("Cache State:")
        for set_idx in range(self.num_sets):
            for way in range(self.associativity):
                tag = self.cache[set_idx][way]
                addr = "INVALID" if tag == -1 else f"Block {tag}"
                lru_val = self.last_access[set_idx][way]
                lines.append(f"  Set {set_idx}, Way {way}: {addr} (LRU counter: {lru_val})")
        return "\n".join(lines)
    
    def get_hit_rate(self) -> float:
        """Return hit rate as percentage."""
        total = self.hits + self.misses
        return (self.hits / total * 100) if total > 0 else 0


def manual_trace(trace_file: str = "test_data/trace.txt",
                 output_file: str = "test_data/expected_results.txt"):
    """
    Manually trace through the access pattern and generate expected results.
    """
    
    # Read trace
    addresses = []
    with open(trace_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            addresses.append(int(line, 16))
    
    # Create cache and trace
    cache = SimpleValidationCache()
    results = []
    
    print("🔍 Tracing cache accesses manually...\n")
    print(f"{'#':<3} {'Address':<10} {'Block':<6} {'Hit/Miss':<10} {'Way':<4} {'Tag':<4}")
    print("=" * 50)
    
    for access_num, addr in enumerate(addresses, 1):
        block = addr // 64
        is_hit, set_idx, way, tag = cache.access(addr)
        hit_miss = "HIT" if is_hit else "MISS"
        
        print(f"{access_num:<3} 0x{addr:04X}     {block:<6} {hit_miss:<10} {way:<4} {tag:<4}")
        
        results.append({
            'access_num': access_num,
            'address': f"0x{addr:04X}",
            'block': block,
            'set': set_idx,
            'way': way,
            'tag': tag,
            'is_hit': is_hit,
            'hit_miss': hit_miss
        })
    
    print("=" * 50)
    print(f"\nTotal: {cache.hits} hits, {cache.misses} misses")
    print(f"Hit Rate: {cache.get_hit_rate():.2f}%")
    print(f"\n{cache.get_cache_state()}\n")
    
    # Write detailed results
    with open(output_file, 'w') as f:
        f.write("EXPECTED CACHE VALIDATION RESULTS\n")
        f.write("=" * 80 + "\n\n")
        
        f.write("Cache Configuration:\n")
        f.write("  Cache size: 256 bytes\n")
        f.write("  Block size: 64 bytes\n")
        f.write("  Associativity: 4-way\n")
        f.write("  Number of sets: 1\n\n")
        
        f.write("Access Trace:\n")
        f.write("-" * 80 + "\n")
        f.write(f"{'#':<3} {'Address':<10} {'Block':<6} {'Set':<4} {'Way':<4} {'Tag':<4} {'Result':<10}\n")
        f.write("-" * 80 + "\n")
        
        for r in results:
            f.write(f"{r['access_num']:<3} {r['address']:<10} {r['block']:<6} {r['set']:<4} "
                   f"{r['way']:<4} {r['tag']:<4} {r['hit_miss']:<10}\n")
        
        f.write("-" * 80 + "\n")
        f.write(f"Total Hits:   {cache.hits}\n")
        f.write(f"Total Misses: {cache.misses}\n")
        f.write(f"Hit Rate:     {cache.get_hit_rate():.2f}%\n")
        f.write("=" * 80 + "\n")
    
    print(f"✅ Expected results written to {output_file}")
    
    return results, cache.hits, cache.misses


if __name__ == "__main__":
    manual_trace()

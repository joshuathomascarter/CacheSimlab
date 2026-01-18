#!/usr/bin/env python3
"""
Generate a predictable test trace for cache validation.

This creates a controlled access pattern that can be hand-traced
to verify cache simulator correctness.

Cache configuration (for reference):
- Cache size: 256 bytes
- Block size: 64 bytes
- Associativity: 4-way
- Number of sets: 1 (256 / 64 / 4 = 1)

Address decoding:
- Offset bits: 6 (log2(64))
- Index bits: 0 (log2(1))
- Tag bits: remaining
"""

def generate_trace(output_file: str = "test_data/trace.txt"):
    """
    Generate predictable access pattern.
    
    Pattern:
    1. Fill cache (4 accesses to different blocks in same set)
    2. Access first block again (HIT)
    3. Access a 5th block (MISS, evict oldest via LRU)
    4. Random accesses to test LRU behavior
    """
    
    trace = []
    
    # ========================================================================
    # PHASE 1: Fill the cache (4 accesses, all to same set)
    # ========================================================================
    # Cache has 1 set, 4 ways. Each way holds 1 block.
    # Accesses to different blocks fill the cache sequentially.
    
    trace.append(0x0000)    # Access 1: Block 0, Way 0, MISS
    trace.append(0x0040)    # Access 2: Block 1, Way 1, MISS
    trace.append(0x0080)    # Access 3: Block 2, Way 2, MISS
    trace.append(0x00C0)    # Access 4: Block 3, Way 3, MISS
    
    # ========================================================================
    # PHASE 2: Test HIT - Re-access block 0 (should be in Way 0)
    # ========================================================================
    trace.append(0x0000)    # Access 5: Block 0, Way 0, HIT
    
    # ========================================================================
    # PHASE 3: Test LRU eviction - Access new block, evict LRU
    # ========================================================================
    # LRU order after Access 5: [1, 2, 3, 0]  (0 is MRU)
    # Accessing block 4 should evict block 1 (LRU)
    
    trace.append(0x0100)    # Access 6: Block 4, Way 1 (evicts Block 1), MISS
    
    # ========================================================================
    # PHASE 4: Verify eviction - Re-access evicted block (should MISS)
    # ========================================================================
    trace.append(0x0040)    # Access 7: Block 1, should MISS (was evicted)
    
    # ========================================================================
    # PHASE 5: Complex access pattern to test LRU state tracking
    # ========================================================================
    # After Access 7: Cache has [4, 1, 2, 3]
    # LRU order: [4, 2, 3, 1] (1 is MRU from recent access)
    
    trace.append(0x0080)    # Access 8: Block 2, HIT (now MRU)
    trace.append(0x00C0)    # Access 9: Block 3, HIT (now MRU)
    trace.append(0x0140)    # Access 10: Block 5, MISS (evicts Block 4, which is LRU)
    
    # ========================================================================
    # PHASE 6: Verify multiple evictions
    # ========================================================================
    trace.append(0x0180)    # Access 11: Block 6, MISS (evicts Block 2, LRU)
    trace.append(0x01C0)    # Access 12: Block 7, MISS (evicts Block 3, LRU)
    
    # ========================================================================
    # PHASE 7: Re-access to verify state
    # ========================================================================
    trace.append(0x0000)    # Access 13: Block 0, MISS (was evicted)
    trace.append(0x0040)    # Access 14: Block 1, HIT (should be in cache)
    trace.append(0x0100)    # Access 15: Block 4, MISS (was evicted in Access 10)
    
    # Write to file
    with open(output_file, 'w') as f:
        for addr in trace:
            f.write(f"0x{addr:04X}\n")
    
    print(f"✅ Generated {len(trace)} addresses to {output_file}")
    print(f"\nTrace details:")
    for i, addr in enumerate(trace, 1):
        block = addr // 64
        print(f"  Access {i:2d}: 0x{addr:04X} (Block {block})")
    
    return trace


if __name__ == "__main__":
    generate_trace()

#!/usr/bin/env python3
"""
Reuse Distance (Stack Distance) Analysis

Computes reuse distance for each memory access, which is the number of unique
addresses accessed between two consecutive accesses to the same address.

Key insight: If reuse distance < cache size, it's a cache hit!
"""

import numpy as np
import matplotlib.pyplot as plt
from collections import OrderedDict
from typing import List, Dict, Tuple, Optional


def load_trace(filename: str) -> List[int]:
    """
    Load a memory trace from file.
    Expects one address per line (hex or decimal).
    """
    addresses = []
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if line.startswith('0x') or line.startswith('0X'):
                addresses.append(int(line, 16))
            else:
                addresses.append(int(line))
    return addresses


def compute_reuse_distance(trace: List[int], block_size: int = 64) -> List[int]:
    """
    Compute the reuse distance (stack distance) for each access.
    
    Reuse distance = number of UNIQUE blocks accessed between two consecutive
    accesses to the same block. First access to any block has distance = -1 (infinity).
    
    Args:
        trace: List of memory addresses
        block_size: Cache block size (addresses in same block share distance)
    
    Returns:
        List of reuse distances (-1 for first access, >= 0 otherwise)
    
    Algorithm:
        Track last access index for each block. For each new access:
        1. If block seen before: count unique blocks between last access and now
        2. If first access: distance = -1 (compulsory miss)
    """
    # Convert to block addresses (multiple addresses per block map to same block_id)
    block_trace = [addr // block_size for addr in trace]
    
    last_access = {}  # Maps block_id -> index of last access in trace
    distances = []
    
    for i, block in enumerate(block_trace):
        if block in last_access:
            # Count unique blocks between last access to this block and now
            distance = len(seen_since_last)
        else:
            distance = -1  # First access (compulsory miss)
        
        distances.append(distance)

        # Update tracking: this block becomes the reference point
        # Clear the "seen since last access to this block" and add current block
        seen_since_last = set()
        last_access[block] = i
        seen_since_last.add(block)

    return distances
        


def compute_reuse_histogram(distances: List[int], 
                            max_distance: int = 1000) -> Dict[int, int]:
    """
    Create histogram of reuse distances.
    
    Args:
        distances: List of reuse distances
        max_distance: Distances >= this are grouped into one bucket
    
    Returns:
        Dictionary mapping distance -> count
    """
    histogram = {}
    
    for d in distances:
        if d == -1:
            key = "inf"  # First access (infinite distance)
        elif d >= max_distance:
            key = f">={max_distance}"
        else:
            key = d
        
        histogram[key] = histogram.get(key, 0) + 1
    
    return histogram


def plot_reuse_histogram(distances: List[int], 
                         output_file: Optional[str] = None,
                         max_distance: int = 500):
    """
    Visualize the reuse distance distribution.
    
    Args:
        distances: List of reuse distances
        output_file: If provided, save plot to this file
        max_distance: Maximum distance to show individually
    """
    # Filter out first accesses (-1) and cap at max_distance
    finite_distances = [d for d in distances if d >= 0]
    capped_distances = [min(d, max_distance) for d in finite_distances]
    
    first_accesses = distances.count(-1)
    
    plt.figure(figsize=(12, 5))
    
    # Histogram
    plt.subplot(1, 2, 1)
    plt.hist(capped_distances, bins=min(100, max_distance), 
             edgecolor='black', alpha=0.7)
    plt.xlabel('Reuse Distance (blocks)')
    plt.ylabel('Frequency')
    plt.title(f'Reuse Distance Distribution\n({first_accesses} first accesses not shown)')
    plt.grid(True, alpha=0.3)
    
    # Cumulative distribution (miss rate curve)
    plt.subplot(1, 2, 2)
    sorted_distances = sorted(finite_distances)
    cumulative = np.arange(1, len(sorted_distances) + 1) / len(sorted_distances)
    
    plt.plot(sorted_distances, cumulative, linewidth=1.5)
    plt.xlabel('Cache Size (blocks)')
    plt.ylabel('Hit Rate')
    plt.title('Predicted Hit Rate vs Cache Size')
    plt.grid(True, alpha=0.3)
    plt.xlim(0, max_distance)
    
    # Add markers for common cache sizes
    cache_sizes = [16, 32, 64, 128, 256]
    for size in cache_sizes:
        if size <= max_distance:
            hit_rate = sum(1 for d in finite_distances if d < size) / len(finite_distances)
            plt.axvline(x=size, linestyle='--', alpha=0.5)
            plt.annotate(f'{size}: {hit_rate:.1%}', 
                        xy=(size, hit_rate), 
                        xytext=(size+10, hit_rate-0.05))
    
    plt.tight_layout()
    
    if output_file:
        plt.savefig(output_file, dpi=150)
        print(f"Saved plot to {output_file}")
    else:
        plt.show()


def predict_hit_rate(distances: List[int], cache_blocks: int) -> float:
    """
    Predict cache hit rate for a given cache size.
    
    Based on stack distance property:
    - If reuse distance < cache_size, it's a hit
    - First accesses (distance = -1) are always misses
    
    Args:
        distances: List of reuse distances
        cache_blocks: Number of blocks in cache
    
    Returns:
        Predicted hit rate (0.0 to 1.0)
    """
    if not distances:
        return 0.0
    
    hits = sum(1 for d in distances if d >= 0 and d < cache_blocks)
    return hits / len(distances)


def predict_miss_rate_curve(distances: List[int], 
                            max_cache_blocks: int = 256) -> List[Tuple[int, float]]:
    """
    Generate miss rate curve for different cache sizes.
    
    Args:
        distances: List of reuse distances
        max_cache_blocks: Maximum cache size to evaluate
    
    Returns:
        List of (cache_size, miss_rate) tuples
    """
    curve = []
    
    for cache_size in range(1, max_cache_blocks + 1):
        hit_rate = predict_hit_rate(distances, cache_size)
        miss_rate = 1.0 - hit_rate
        curve.append((cache_size, miss_rate))
    
    return curve


def analyze_reuse_distance(trace: List[int], block_size: int = 64):
    """
    Print comprehensive reuse distance analysis.
    
    Args:
        trace: List of memory addresses
        block_size: Cache block size
    """
    print("Computing reuse distances...")
    distances = compute_reuse_distance_fast(trace, block_size)
    
    # Statistics
    finite = [d for d in distances if d >= 0]
    first_accesses = distances.count(-1)
    
    print()
    print("=" * 50)
    print("REUSE DISTANCE ANALYSIS")
    print("=" * 50)
    print(f"Total accesses: {len(trace)}")
    print(f"First accesses (compulsory misses): {first_accesses}")
    print(f"Reuses: {len(finite)}")
    print()
    
    if finite:
        print("Reuse Distance Statistics:")
        print("-" * 50)
        print(f"  Mean: {np.mean(finite):.1f} blocks")
        print(f"  Median: {np.median(finite):.1f} blocks")
        print(f"  Std Dev: {np.std(finite):.1f} blocks")
        print(f"  Min: {min(finite)} blocks")
        print(f"  Max: {max(finite)} blocks")
        print(f"  25th percentile: {np.percentile(finite, 25):.1f} blocks")
        print(f"  75th percentile: {np.percentile(finite, 75):.1f} blocks")
        print(f"  95th percentile: {np.percentile(finite, 95):.1f} blocks")
        print()
    
    # Predicted hit rates
    print("Predicted Hit Rates:")
    print("-" * 50)
    cache_sizes = [8, 16, 32, 64, 128, 256, 512, 1024]
    
    for size in cache_sizes:
        hit_rate = predict_hit_rate(distances, size)
        bytes_size = size * block_size
        print(f"  {size:4d} blocks ({bytes_size:6d} bytes): {hit_rate*100:5.1f}% hit rate")
    
    return distances


# ============================================================================
# Main - Example Usage
# ============================================================================

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1:
        # Load trace from file
        trace_file = sys.argv[1]
        print(f"Loading trace from {trace_file}...")
        trace = load_trace(trace_file)
    else:
        # Generate example trace with different access patterns
        print("Generating example trace...")
        np.random.seed(42)
        
        trace = []
        
        # Pattern 1: Tight loop (small reuse distance)
        for _ in range(10):
            for addr in range(0, 256, 4):
                trace.append(addr)
        
        # Pattern 2: Strided access
        for _ in range(5):
            for addr in range(0, 4096, 64):
                trace.append(addr)
        
        # Pattern 3: Random access (large reuse distance)
        trace.extend(np.random.randint(0, 65536, size=1000))
        
        # Pattern 4: Working set that exceeds cache
        for _ in range(3):
            for addr in range(0, 8192, 8):
                trace.append(addr)
    
    # Run analysis
    distances = analyze_reuse_distance(trace, block_size=64)
    
    print("\nGenerating plots...")
    plot_reuse_histogram(distances, output_file="reuse_distance_analysis.png")


def test_reuse_distance_bug():
    """Test that reuse distance counts UNIQUE blocks, not total accesses"""
    # Block trace: [A, B, A, C, A]
    trace = [0, 64, 0, 128, 0]  # addresses that map to blocks 0, 1, 0, 2, 0
    distances = compute_reuse_distance(trace, block_size=64)
    
    # Expected: [-1, -1, 1, -1, 1]
    # (A first, B first, A with 1 unique block between (B), C first, A with 1 unique between (C))
    assert distances == [-1, -1, 1, -1, 1], f"Got {distances}"
    print("✅ Reuse distance bug test passed!")


if __name__ == "__main__":
    test_reuse_distance_bug()

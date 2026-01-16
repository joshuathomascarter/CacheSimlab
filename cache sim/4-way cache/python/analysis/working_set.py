#!/usr/bin/env python3
"""
Working Set Analysis

Analyzes the working set of a memory trace - the set of unique addresses
accessed within a time window. Useful for understanding cache size requirements.
"""

import numpy as np
import matplotlib.pyplot as plt
from collections import deque
from typing import List, Tuple, Optional


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
            # Handle hex (0x...) or decimal
            if line.startswith('0x') or line.startswith('0X'):
                addresses.append(int(line, 16))
            else:
                addresses.append(int(line))
    return addresses


def calculate_working_set(trace: List[int], window_size: int, 
                          block_size: int = 64) -> List[int]:
    """
    Calculate the working set size over time using a sliding window.
    
    Args:
        trace: List of memory addresses
        window_size: Number of accesses in the sliding window
        block_size: Cache block size (addresses in same block count as one)
    
    Returns:
        List of working set sizes (one per position in trace)
    """
    if len(trace) < window_size:
        return [len(set(addr // block_size for addr in trace))]
    
    # Convert to block addresses
    block_trace = [addr // block_size for addr in trace]
    
    working_set_sizes = []
    
    # Initialize window with first window_size elements
    window = deque(block_trace[:window_size])
    block_count = {}  # block -> count in window
    
    for block in window:
        block_count[block] = block_count.get(block, 0) + 1
    
    working_set_sizes.append(len(block_count))
    
    # Slide window through trace
    for i in range(window_size, len(block_trace)):
        # Remove oldest element
        old_block = window.popleft()
        block_count[old_block] -= 1
        if block_count[old_block] == 0:
            del block_count[old_block]
        
        # Add new element
        new_block = block_trace[i]
        window.append(new_block)
        block_count[new_block] = block_count.get(new_block, 0) + 1
        
        working_set_sizes.append(len(block_count))
    
    return working_set_sizes


def plot_working_set_over_time(trace: List[int], window_size: int = 1000,
                                block_size: int = 64, 
                                output_file: Optional[str] = None):
    """
    Visualize how working set size changes over time.
    
    Args:
        trace: List of memory addresses
        window_size: Sliding window size
        block_size: Cache block size
        output_file: If provided, save plot to this file
    """
    ws_sizes = calculate_working_set(trace, window_size, block_size)
    
    plt.figure(figsize=(12, 6))
    
    # Main plot
    plt.subplot(1, 2, 1)
    plt.plot(ws_sizes, linewidth=0.5, alpha=0.7)
    plt.axhline(y=np.mean(ws_sizes), color='r', linestyle='--', 
                label=f'Mean: {np.mean(ws_sizes):.1f}')
    plt.xlabel('Time (access number)')
    plt.ylabel('Working Set Size (blocks)')
    plt.title(f'Working Set Over Time (window={window_size})')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    # Histogram
    plt.subplot(1, 2, 2)
    plt.hist(ws_sizes, bins=50, edgecolor='black', alpha=0.7)
    plt.axvline(x=np.mean(ws_sizes), color='r', linestyle='--', 
                label=f'Mean: {np.mean(ws_sizes):.1f}')
    plt.axvline(x=np.percentile(ws_sizes, 95), color='orange', linestyle='--',
                label=f'95th %ile: {np.percentile(ws_sizes, 95):.1f}')
    plt.xlabel('Working Set Size (blocks)')
    plt.ylabel('Frequency')
    plt.title('Working Set Size Distribution')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    if output_file:
        plt.savefig(output_file, dpi=150)
        print(f"Saved plot to {output_file}")
    else:
        plt.show()


def simulate_cache_hit_rate(trace: List[int], cache_blocks: int, 
                            block_size: int = 64) -> float:
    """
    Simulate a fully-associative LRU cache and return hit rate.
    
    Args:
        trace: List of memory addresses
        cache_blocks: Number of blocks in cache
        block_size: Cache block size
    
    Returns:
        Hit rate (0.0 to 1.0)
    """
    block_trace = [addr // block_size for addr in trace]
    
    cache = []  # List acting as LRU stack (front = MRU, back = LRU)
    hits = 0
    
    for block in block_trace:
        if block in cache:
            hits += 1
            # Move to front (MRU)
            cache.remove(block)
            cache.insert(0, block)
        else:
            # Insert at front
            cache.insert(0, block)
            # Evict if over capacity
            if len(cache) > cache_blocks:
                cache.pop()
    
    return hits / len(trace) if trace else 0.0


def estimate_cache_size_needed(trace: List[int], target_hit_rate: float,
                                block_size: int = 64,
                                max_blocks: int = 4096) -> Tuple[int, int]:
    """
    Find minimum cache size (in blocks and bytes) to achieve target hit rate.
    Uses binary search for efficiency.
    
    Args:
        trace: List of memory addresses
        target_hit_rate: Desired hit rate (e.g., 0.90 for 90%)
        block_size: Cache block size in bytes
        max_blocks: Maximum number of blocks to consider
    
    Returns:
        Tuple of (blocks_needed, bytes_needed)
    """
    # Binary search for minimum cache size
    low, high = 1, max_blocks
    best = max_blocks
    
    while low <= high:
        mid = (low + high) // 2
        hit_rate = simulate_cache_hit_rate(trace, mid, block_size)
        
        if hit_rate >= target_hit_rate:
            best = mid
            high = mid - 1
        else:
            low = mid + 1
    
    return best, best * block_size


def analyze_working_set(trace: List[int], block_size: int = 64):
    """
    Print comprehensive working set analysis.
    
    Args:
        trace: List of memory addresses
        block_size: Cache block size
    """
    block_trace = [addr // block_size for addr in trace]
    unique_blocks = len(set(block_trace))
    
    print("=" * 50)
    print("WORKING SET ANALYSIS")
    print("=" * 50)
    print(f"Total accesses: {len(trace)}")
    print(f"Unique blocks accessed: {unique_blocks}")
    print(f"Block size: {block_size} bytes")
    print(f"Minimum memory footprint: {unique_blocks * block_size} bytes")
    print()
    
    # Analyze at different window sizes
    window_sizes = [100, 500, 1000, 5000]
    print("Working Set by Window Size:")
    print("-" * 50)
    
    for ws in window_sizes:
        if len(trace) >= ws:
            sizes = calculate_working_set(trace, ws, block_size)
            print(f"  Window {ws:5d}: mean={np.mean(sizes):6.1f}, "
                  f"max={max(sizes):5d}, min={min(sizes):5d} blocks")
    
    print()
    
    # Cache size recommendations
    print("Cache Size Recommendations:")
    print("-" * 50)
    
    targets = [0.80, 0.90, 0.95, 0.99]
    for target in targets:
        blocks, bytes_needed = estimate_cache_size_needed(trace, target, block_size)
        print(f"  {target*100:4.0f}% hit rate: {blocks:5d} blocks = {bytes_needed:7d} bytes "
              f"({bytes_needed/1024:.1f} KB)")


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
        # Generate example trace
        print("Generating example trace...")
        np.random.seed(42)
        
        # Mix of sequential and random access
        trace = []
        
        # Sequential loop (high locality)
        for _ in range(5):
            for addr in range(0, 4096, 4):
                trace.append(addr)
        
        # Random access (low locality)
        trace.extend(np.random.randint(0, 1024*1024, size=5000))
        
        # Another sequential loop
        for _ in range(3):
            for addr in range(8192, 12288, 8):
                trace.append(addr)
    
    # Run analysis
    analyze_working_set(trace, block_size=64)
    
    print("\nGenerating plots...")
    plot_working_set_over_time(trace, window_size=500, block_size=64,
                               output_file="working_set_analysis.png")

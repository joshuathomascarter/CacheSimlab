#!/usr/bin/env python3
"""
Random Trace Generator

Generates memory access traces with random and locality-based patterns for cache
simulation testing. Supports both uniform random access and realistic patterns
with temporal/spatial locality (hot/cold regions).
"""

import csv
import random
import argparse
from typing import List, Tuple, Dict
from enum import Enum


class AccessType(Enum):
    """Memory access types"""
    READ = "READ"
    WRITE = "WRITE"


def generate_random(addr_range: Tuple[int, int], count: int, seed: int = None,
                   read_ratio: float = 0.7) -> List[Tuple[int, str, int]]:
    """
    Generate random memory accesses for worst-case cache testing.
    
    Args:
        addr_range: Tuple of (min_address, max_address)
        count: Number of accesses to generate
        seed: Random seed for reproducibility (None for random)
        read_ratio: Ratio of READ vs WRITE operations (0.0 to 1.0)
    
    Returns:
        List of tuples: (address, access_type, timestamp)
        
    Note:
        Using a seed ensures the same "random" sequence for debugging.
    """
    if seed is not None:
        random.seed(seed)
    
    trace = []
    min_addr, max_addr = addr_range
    
    for timestamp in range(count):
        # Generate random address in range
        address = random.randint(min_addr, max_addr)
        
        # Determine access type based on ratio
        access_type = AccessType.READ if random.random() < read_ratio else AccessType.WRITE
        
        trace.append((address, access_type.value, timestamp))
    
    return trace


def generate_with_locality(hot_regions: List[Tuple[int, int]], 
                          cold_range: Tuple[int, int],
                          count: int,
                          hot_ratio: float = 0.8,
                          seed: int = None,
                          read_ratio: float = 0.7) -> List[Tuple[int, str, int]]:
    """
    Generate realistic traces with temporal locality (hot/cold regions).
    
    This models real-world programs where certain memory regions are accessed
    frequently (hot spots like loops, frequently used data structures) while
    others are rarely accessed (cold regions).
    
    Args:
        hot_regions: List of (start_addr, end_addr) tuples for frequently accessed regions
        cold_range: Tuple of (min_addr, max_addr) for the entire address space
        count: Number of accesses to generate
        hot_ratio: Probability of accessing hot regions (0.0 to 1.0)
        seed: Random seed for reproducibility
        read_ratio: Ratio of READ vs WRITE operations
    
    Returns:
        List of tuples: (address, access_type, timestamp)
        
    Example:
        hot_regions = [(0x1000, 0x2000), (0x5000, 0x6000)]  # Two hot spots
        cold_range = (0x0, 0x10000)  # Entire 64KB space
        hot_ratio = 0.8  # 80% accesses go to hot regions
    """
    if seed is not None:
        random.seed(seed)
    
    trace = []
    
    for timestamp in range(count):
        # Decide if this access goes to hot or cold region
        if random.random() < hot_ratio and hot_regions:
            # Access hot region - pick a random hot region, then random address within it
            hot_region = random.choice(hot_regions)
            address = random.randint(hot_region[0], hot_region[1])
        else:
            # Access cold region - anywhere in the full address space
            address = random.randint(cold_range[0], cold_range[1])
        
        # Determine access type
        access_type = AccessType.READ if random.random() < read_ratio else AccessType.WRITE
        
        trace.append((address, access_type.value, timestamp))
    
    return trace


def generate_strided_random(base_addrs: List[int], stride: int, 
                           iterations: int, seed: int = None,
                           read_ratio: float = 0.7) -> List[Tuple[int, str, int]]:
    """
    Generate random accesses with strided patterns (models array traversals).
    
    Args:
        base_addrs: List of base addresses to start from
        stride: Byte offset between consecutive accesses
        iterations: Number of iterations through the pattern
        seed: Random seed for reproducibility
        read_ratio: Ratio of READ vs WRITE operations
    
    Returns:
        List of tuples: (address, access_type, timestamp)
        
    Example:
        Models accessing multiple arrays with random starting points but
        sequential access within each array.
    """
    if seed is not None:
        random.seed(seed)
    
    trace = []
    timestamp = 0
    
    for _ in range(iterations):
        # Pick a random base address
        base = random.choice(base_addrs)
        
        # Generate strided accesses from this base
        num_accesses = random.randint(5, 20)  # Random burst length
        for i in range(num_accesses):
            address = base + (i * stride)
            access_type = AccessType.READ if random.random() < read_ratio else AccessType.WRITE
            trace.append((address, access_type.value, timestamp))
            timestamp += 1
    
    return trace


def generate_zipf_distribution(addr_range: Tuple[int, int], count: int,
                               alpha: float = 1.5, seed: int = None,
                               read_ratio: float = 0.7) -> List[Tuple[int, str, int]]:
    """
    Generate accesses following Zipf distribution (power-law).
    
    This models real-world access patterns where a small number of addresses
    are accessed very frequently (follows the 80-20 rule).
    
    Args:
        addr_range: Tuple of (min_address, max_address)
        count: Number of accesses to generate
        alpha: Zipf parameter (higher = more skewed, typical: 1.0-2.0)
        seed: Random seed for reproducibility
        read_ratio: Ratio of READ vs WRITE operations
    
    Returns:
        List of tuples: (address, access_type, timestamp)
    """
    if seed is not None:
        random.seed(seed)
    
    min_addr, max_addr = addr_range
    num_unique_addrs = 1000  # Number of unique addresses to choose from
    
    # Generate Zipf-distributed ranks
    ranks = list(range(1, num_unique_addrs + 1))
    weights = [1.0 / (rank ** alpha) for rank in ranks]
    
    # Normalize weights
    total_weight = sum(weights)
    weights = [w / total_weight for w in weights]
    
    # Create address pool
    addresses = [random.randint(min_addr, max_addr) for _ in range(num_unique_addrs)]
    
    trace = []
    for timestamp in range(count):
        # Select address based on Zipf distribution
        address = random.choices(addresses, weights=weights)[0]
        
        # Determine access type
        access_type = AccessType.READ if random.random() < read_ratio else AccessType.WRITE
        
        trace.append((address, access_type.value, timestamp))
    
    return trace


def write_trace_csv(trace: List[Tuple[int, str, int]], path: str) -> None:
    """
    Write trace data to CSV file.
    
    Args:
        trace: List of (address, access_type, timestamp) tuples
        path: Output file path
    """
    with open(path, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['address', 'access_type', 'timestamp'])
        for address, access_type, timestamp in trace:
            writer.writerow([address, access_type, timestamp])
    
    print(f"Trace written to {path} ({len(trace)} entries)")


def analyze_trace(trace: List[Tuple[int, str, int]]) -> Dict:
    """
    Analyze trace statistics.
    
    Args:
        trace: List of (address, access_type, timestamp) tuples
    
    Returns:
        Dictionary with trace statistics
    """
    addresses = [addr for addr, _, _ in trace]
    unique_addrs = set(addresses)
    
    read_count = sum(1 for _, atype, _ in trace if atype == 'READ')
    write_count = len(trace) - read_count
    
    return {
        'total_accesses': len(trace),
        'unique_addresses': len(unique_addrs),
        'reuse_ratio': len(trace) / len(unique_addrs) if unique_addrs else 0,
        'read_count': read_count,
        'write_count': write_count,
        'read_ratio': read_count / len(trace) if trace else 0,
        'min_address': min(addresses) if addresses else 0,
        'max_address': max(addresses) if addresses else 0,
    }


def main():
    """Command-line interface for random trace generation"""
    parser = argparse.ArgumentParser(
        description='Generate random memory access traces for cache simulation'
    )
    
    subparsers = parser.add_subparsers(dest='mode', help='Generation mode')
    
    # Uniform random mode
    uniform_parser = subparsers.add_parser('uniform', help='Uniform random accesses')
    uniform_parser.add_argument('--min-addr', type=int, default=0)
    uniform_parser.add_argument('--max-addr', type=int, required=True)
    uniform_parser.add_argument('--count', type=int, required=True)
    uniform_parser.add_argument('--seed', type=int, default=None)
    uniform_parser.add_argument('--read-ratio', type=float, default=0.7)
    uniform_parser.add_argument('--output', type=str, required=True)
    
    # Locality mode
    locality_parser = subparsers.add_parser('locality', help='Accesses with hot/cold regions')
    locality_parser.add_argument('--hot-regions', type=str, required=True,
                                help='Hot regions as "start1-end1,start2-end2,..."')
    locality_parser.add_argument('--min-addr', type=int, default=0)
    locality_parser.add_argument('--max-addr', type=int, required=True)
    locality_parser.add_argument('--count', type=int, required=True)
    locality_parser.add_argument('--hot-ratio', type=float, default=0.8)
    locality_parser.add_argument('--seed', type=int, default=None)
    locality_parser.add_argument('--read-ratio', type=float, default=0.7)
    locality_parser.add_argument('--output', type=str, required=True)
    
    # Zipf mode
    zipf_parser = subparsers.add_parser('zipf', help='Zipf distribution (power-law)')
    zipf_parser.add_argument('--min-addr', type=int, default=0)
    zipf_parser.add_argument('--max-addr', type=int, required=True)
    zipf_parser.add_argument('--count', type=int, required=True)
    zipf_parser.add_argument('--alpha', type=float, default=1.5)
    zipf_parser.add_argument('--seed', type=int, default=None)
    zipf_parser.add_argument('--read-ratio', type=float, default=0.7)
    zipf_parser.add_argument('--output', type=str, required=True)
    
    args = parser.parse_args()
    
    # Generate trace based on mode
    if args.mode == 'uniform':
        trace = generate_random((args.min_addr, args.max_addr), args.count, 
                               args.seed, args.read_ratio)
    elif args.mode == 'locality':
        # Parse hot regions
        hot_regions = []
        for region_str in args.hot_regions.split(','):
            start, end = map(int, region_str.split('-'))
            hot_regions.append((start, end))
        
        trace = generate_with_locality(hot_regions, (args.min_addr, args.max_addr),
                                       args.count, args.hot_ratio, args.seed, 
                                       args.read_ratio)
    elif args.mode == 'zipf':
        trace = generate_zipf_distribution((args.min_addr, args.max_addr), args.count,
                                          args.alpha, args.seed, args.read_ratio)
    else:
        parser.print_help()
        return
    
    # Write trace
    write_trace_csv(trace, args.output)
    
    # Print analysis
    stats = analyze_trace(trace)
    print(f"\nTrace Analysis:")
    print(f"  Total accesses: {stats['total_accesses']}")
    print(f"  Unique addresses: {stats['unique_addresses']}")
    print(f"  Reuse ratio: {stats['reuse_ratio']:.2f}")
    print(f"  Read/Write: {stats['read_count']}/{stats['write_count']} "
          f"({stats['read_ratio']:.1%} reads)")
    print(f"  Address range: 0x{stats['min_address']:X} - 0x{stats['max_address']:X}")


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""
Sequential Trace Generator

Generates memory access traces with sequential patterns for cache simulation testing.
This is useful for testing spatial locality and cache performance with predictable
access patterns.
"""

import csv
import argparse
from typing import List, Tuple
from enum import Enum


class AccessType(Enum):
    """Memory access types"""
    READ = "READ"
    WRITE = "WRITE"


def generate_sequential(start: int, count: int, stride: int, 
                       access_type: AccessType = AccessType.READ) -> List[Tuple[int, str, int]]:
    """
    Generate a sequential memory access trace.
    
    Args:
        start: Starting memory address
        count: Number of addresses to generate
        stride: Byte offset between consecutive addresses
        access_type: Type of memory access (READ or WRITE)
    
    Returns:
        List of tuples: (address, access_type, timestamp)
        
    Example:
        generate_sequential(0, 5, 64) produces:
        [(0, 'READ', 0), (64, 'READ', 1), (128, 'READ', 2), (192, 'READ', 3), (256, 'READ', 4)]
    """
    trace = []
    current_addr = start
    
    for timestamp in range(count):
        trace.append((current_addr, access_type.value, timestamp))
        current_addr += stride
    
    return trace


def generate_sequential_with_pattern(start: int, count: int, stride: int,
                                     read_ratio: float = 0.7) -> List[Tuple[int, str, int]]:
    """
    Generate a sequential trace with mixed READ/WRITE pattern.
    
    Args:
        start: Starting memory address
        count: Number of addresses to generate
        stride: Byte offset between consecutive addresses
        read_ratio: Ratio of READ operations (0.0 to 1.0)
    
    Returns:
        List of tuples: (address, access_type, timestamp)
    """
    import random
    trace = []
    current_addr = start
    
    for timestamp in range(count):
        # Determine access type based on ratio
        access_type = AccessType.READ if random.random() < read_ratio else AccessType.WRITE
        trace.append((current_addr, access_type.value, timestamp))
        current_addr += stride
    
    return trace


def write_trace_csv(trace: List[Tuple[int, str, int]], path: str) -> None:
    """
    Write trace data to CSV file.
    
    Args:
        trace: List of (address, access_type, timestamp) tuples
        path: Output file path
        
    CSV Format:
        address,access_type,timestamp
        0,READ,0
        64,READ,1
        ...
    """
    with open(path, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # Write header
        writer.writerow(['address', 'access_type', 'timestamp'])
        # Write trace data
        for address, access_type, timestamp in trace:
            writer.writerow([address, access_type, timestamp])
    
    print(f"Trace written to {path} ({len(trace)} entries)")


def write_trace_binary(trace: List[Tuple[int, str, int]], path: str) -> None:
    """
    Write trace data to binary file for faster loading.
    
    Args:
        trace: List of (address, access_type, timestamp) tuples
        path: Output file path
        
    Binary Format (per entry):
        8 bytes: address (uint64)
        1 byte: access_type (0=READ, 1=WRITE)
        8 bytes: timestamp (uint64)
    """
    import struct
    
    with open(path, 'wb') as binfile:
        for address, access_type, timestamp in trace:
            # Pack: address (Q=uint64), type (B=uint8), timestamp (Q=uint64)
            access_code = 0 if access_type == 'READ' else 1
            binfile.write(struct.pack('QBQ', address, access_code, timestamp))
    
    print(f"Binary trace written to {path} ({len(trace)} entries)")


def main():
    """Command-line interface for sequential trace generation"""
    parser = argparse.ArgumentParser(
        description='Generate sequential memory access traces for cache simulation'
    )
    parser.add_argument('--start', type=int, default=0,
                       help='Starting memory address (default: 0)')
    parser.add_argument('--count', type=int, required=True,
                       help='Number of memory accesses to generate')
    parser.add_argument('--stride', type=int, default=64,
                       help='Byte offset between addresses (default: 64)')
    parser.add_argument('--output', type=str, required=True,
                       help='Output file path')
    parser.add_argument('--format', choices=['csv', 'binary'], default='csv',
                       help='Output format (default: csv)')
    parser.add_argument('--access-type', choices=['read', 'write', 'mixed'], 
                       default='read',
                       help='Access type pattern (default: read)')
    parser.add_argument('--read-ratio', type=float, default=0.7,
                       help='Read ratio for mixed pattern (default: 0.7)')
    
    args = parser.parse_args()
    
    # Generate trace based on access type
    if args.access_type == 'read':
        trace = generate_sequential(args.start, args.count, args.stride, AccessType.READ)
    elif args.access_type == 'write':
        trace = generate_sequential(args.start, args.count, args.stride, AccessType.WRITE)
    else:  # mixed
        trace = generate_sequential_with_pattern(args.start, args.count, args.stride, 
                                                args.read_ratio)
    
    # Write to file
    if args.format == 'csv':
        write_trace_csv(trace, args.output)
    else:
        write_trace_binary(trace, args.output)
    
    # Print summary
    print(f"\nTrace Summary:")
    print(f"  Total accesses: {len(trace)}")
    print(f"  Address range: {trace[0][0]} - {trace[-1][0]}")
    print(f"  Stride: {args.stride} bytes")


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""
Parse C++ cache simulator output and convert to validation format.

This script reads the output from your C++ test_cache binary and extracts
hit/miss information in a format that can be compared with expected results.
"""

import subprocess
import os
from typing import List, Dict

def run_cpp_simulator(trace_file: str, cpp_binary: str = None) -> str:
    """
    Run the C++ cache simulator with the trace file.
    
    Args:
        trace_file: Path to input trace
        cpp_binary: Path to compiled C++ binary (auto-detect if None)
    
    Returns:
        Output from the C++ program
    """
    
    # Auto-detect C++ binary if not provided
    if cpp_binary is None:
        possible_paths = [
            "/Users/joshcarter/Desktop/memory_sim/cache sim/4-way cache/test_cache",
            "./test_cache",
            "../c++/test_cache",
        ]
        
        for path in possible_paths:
            if os.path.exists(path):
                cpp_binary = path
                break
    
    if cpp_binary is None:
        raise FileNotFoundError(
            "Could not find test_cache binary. "
            "Please compile the C++ code or provide binary path."
        )
    
    print(f"Running C++ simulator: {cpp_binary}")
    print(f"With trace: {trace_file}\n")
    
    try:
        # Run with trace file as argument
        result = subprocess.run(
            [cpp_binary, trace_file],
            capture_output=True,
            text=True,
            timeout=10
        )
        
        if result.returncode != 0:
            print(f"⚠️  C++ binary exited with code {result.returncode}")
            print(f"stderr: {result.stderr}")
        
        return result.stdout
    
    except FileNotFoundError:
        raise FileNotFoundError(f"C++ binary not found at {cpp_binary}")
    except subprocess.TimeoutExpired:
        raise RuntimeError("C++ simulator timed out (>10s)")


def parse_cpp_output(cpp_output: str, 
                     output_file: str = "test_data/cpp_results.txt") -> Dict:
    """
    Parse C++ output into structured format.
    
    Expected C++ output format (you may need to adjust based on your actual output):
    - Each line: "Access N: addr=0xXXXX block=X set=X way=X tag=X HIT/MISS"
    - Summary: "Total: X hits, Y misses, Z% hit rate"
    """
    
    results = []
    hits = 0
    misses = 0
    
    lines = cpp_output.strip().split('\n')
    
    print("Parsing C++ output...")
    
    for line in lines:
        line = line.strip()
        if not line:
            continue
        
        # Try to parse access line
        if 'Access' in line or 'access' in line:
            try:
                # Try to extract information (adjust regex based on actual output format)
                parts = line.split()
                
                # Generic parser - looks for HIT/MISS keyword
                if 'HIT' in line.upper():
                    hits += 1
                    hit_miss = "HIT"
                elif 'MISS' in line.upper():
                    misses += 1
                    hit_miss = "MISS"
                else:
                    continue
                
                # Try to extract access number
                access_num = None
                for part in parts:
                    if part.isdigit():
                        access_num = int(part)
                        break
                
                if access_num:
                    results.append({
                        'access_num': access_num,
                        'hit_miss': hit_miss,
                        'raw_line': line
                    })
            
            except Exception as e:
                print(f"Warning: Could not parse line: {line}")
                continue
        
        # Try to parse summary line
        if 'Total' in line or 'total' in line or 'Summary' in line:
            # Try to extract hit/miss counts
            try:
                if 'hit' in line.lower():
                    # Parse "X hits" format
                    for i, part in enumerate(parts):
                        if 'hit' in part.lower() and i > 0:
                            try:
                                hits = int(parts[i-1])
                            except:
                                pass
                if 'miss' in line.lower():
                    for i, part in enumerate(parts):
                        if 'miss' in part.lower() and i > 0:
                            try:
                                misses = int(parts[i-1])
                            except:
                                pass
            except:
                pass
    
    # Write results
    with open(output_file, 'w') as f:
        f.write("C++ CACHE SIMULATOR RESULTS\n")
        f.write("=" * 80 + "\n\n")
        
        f.write("Access Trace Results:\n")
        f.write("-" * 80 + "\n")
        f.write(f"{'#':<3} {'Result':<10} {'Raw Output':<60}\n")
        f.write("-" * 80 + "\n")
        
        for r in results:
            f.write(f"{r['access_num']:<3} {r['hit_miss']:<10} {r['raw_line'][:60]:<60}\n")
        
        f.write("-" * 80 + "\n")
        f.write(f"Total Hits:   {hits}\n")
        f.write(f"Total Misses: {misses}\n")
        total = hits + misses
        hit_rate = (hits / total * 100) if total > 0 else 0
        f.write(f"Hit Rate:     {hit_rate:.2f}%\n")
        f.write("=" * 80 + "\n")
    
    print(f"✅ C++ results written to {output_file}")
    
    return {
        'results': results,
        'hits': hits,
        'misses': misses,
        'hit_rate': (hits / (hits + misses) * 100) if (hits + misses) > 0 else 0
    }


if __name__ == "__main__":
    import sys
    
    trace_file = sys.argv[1] if len(sys.argv) > 1 else "test_data/trace.txt"
    cpp_binary = sys.argv[2] if len(sys.argv) > 2 else None
    
    try:
        cpp_output = run_cpp_simulator(trace_file, cpp_binary)
        print(f"C++ Output:\n{cpp_output}\n")
        
        parse_cpp_output(cpp_output)
    
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)

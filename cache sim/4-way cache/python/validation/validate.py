#!/usr/bin/env python3
"""
Compare expected results vs C++ simulator results.

This validates that your C++ cache simulator produces the same results
as hand-tracing through the access pattern.
"""

import re
from typing import Dict, List, Tuple


def parse_results_file(filename: str) -> Dict:
    """Parse a results file (expected or actual) into structured data."""
    
    results = {
        'accesses': [],
        'hits': 0,
        'misses': 0,
        'hit_rate': 0.0
    }
    
    in_trace = False
    
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            
            # Start of trace section
            if 'Access Trace' in line or '---' in line:
                in_trace = True
                continue
            
            # End of trace section
            if in_trace and ('=' in line or not line):
                in_trace = False
                continue
            
            # Parse access lines (format: # Address Block Set Way Tag Result)
            if in_trace and line and not '#' in line[:1]:
                try:
                    parts = line.split()
                    if len(parts) >= 7:
                        access_num = int(parts[0])
                        address = parts[1]
                        block = int(parts[2])
                        set_idx = int(parts[3])
                        way = int(parts[4])
                        tag = int(parts[5])
                        hit_miss = parts[6]
                        
                        results['accesses'].append({
                            'access_num': access_num,
                            'address': address,
                            'block': block,
                            'set': set_idx,
                            'way': way,
                            'tag': tag,
                            'hit_miss': hit_miss
                        })
                except Exception as e:
                    pass  # Skip malformed lines
            
            # Parse summary statistics
            if 'Total Hits' in line:
                try:
                    results['hits'] = int(line.split(':')[1].strip())
                except:
                    pass
            
            if 'Total Misses' in line:
                try:
                    results['misses'] = int(line.split(':')[1].strip())
                except:
                    pass
            
            if 'Hit Rate' in line:
                try:
                    rate_str = line.split(':')[1].strip().rstrip('%')
                    results['hit_rate'] = float(rate_str)
                except:
                    pass
    
    return results


def compare_results(expected_file: str = "test_data/expected_results.txt",
                    actual_file: str = "test_data/cpp_results.txt",
                    report_file: str = "test_data/validation_report.txt") -> bool:
    """
    Compare expected vs actual results.
    
    Returns: True if all tests pass, False otherwise
    """
    
    print("🔍 Loading results...\n")
    
    expected = parse_results_file(expected_file)
    actual = parse_results_file(actual_file)
    
    print(f"Expected: {expected['hits']} hits, {expected['misses']} misses ({expected['hit_rate']:.2f}%)")
    print(f"Actual:   {actual['hits']} hits, {actual['misses']} misses ({actual['hit_rate']:.2f}%)\n")
    
    # Compare
    mismatches = []
    passed = True
    
    # Check counts
    if expected['hits'] != actual['hits']:
        mismatches.append(f"Hit count mismatch: expected {expected['hits']}, got {actual['hits']}")
        passed = False
    
    if expected['misses'] != actual['misses']:
        mismatches.append(f"Miss count mismatch: expected {expected['misses']}, got {actual['misses']}")
        passed = False
    
    # Check individual accesses
    for i in range(min(len(expected['accesses']), len(actual['accesses']))):
        exp = expected['accesses'][i]
        act = actual['accesses'][i]
        
        if exp['hit_miss'] != act['hit_miss']:
            mismatches.append(
                f"Access {i+1}: expected {exp['hit_miss']}, got {act['hit_miss']} "
                f"(addr={exp['address']})"
            )
            passed = False
    
    # Check length mismatch
    if len(expected['accesses']) != len(actual['accesses']):
        mismatches.append(
            f"Access count mismatch: expected {len(expected['accesses'])} accesses, "
            f"got {len(actual['accesses'])}"
        )
        passed = False
    
    # Write report
    with open(report_file, 'w') as f:
        f.write("CACHE VALIDATION REPORT\n")
        f.write("=" * 80 + "\n\n")
        
        if passed:
            f.write("✅ VALIDATION PASSED\n\n")
        else:
            f.write("❌ VALIDATION FAILED\n\n")
        
        f.write("Summary:\n")
        f.write(f"  Expected hits:   {expected['hits']}\n")
        f.write(f"  Actual hits:     {actual['hits']}\n")
        f.write(f"  Expected misses: {expected['misses']}\n")
        f.write(f"  Actual misses:   {actual['misses']}\n")
        f.write(f"  Expected rate:   {expected['hit_rate']:.2f}%\n")
        f.write(f"  Actual rate:     {actual['hit_rate']:.2f}%\n")
        f.write("\n")
        
        if mismatches:
            f.write(f"Mismatches ({len(mismatches)}):\n")
            f.write("-" * 80 + "\n")
            for i, mismatch in enumerate(mismatches, 1):
                f.write(f"  {i}. {mismatch}\n")
        else:
            f.write("No mismatches found. ✅\n")
        
        f.write("\n")
        f.write("Detailed Comparison:\n")
        f.write("-" * 80 + "\n")
        f.write(f"{'#':<3} {'Expected':<12} {'Actual':<12} {'Match':<6}\n")
        f.write("-" * 80 + "\n")
        
        for i in range(max(len(expected['accesses']), len(actual['accesses']))):
            exp_result = expected['accesses'][i]['hit_miss'] if i < len(expected['accesses']) else "N/A"
            act_result = actual['accesses'][i]['hit_miss'] if i < len(actual['accesses']) else "N/A"
            match = "✅" if exp_result == act_result else "❌"
            
            f.write(f"{i+1:<3} {exp_result:<12} {act_result:<12} {match:<6}\n")
        
        f.write("=" * 80 + "\n")
    
    # Print results
    print("=" * 80)
    print("VALIDATION REPORT")
    print("=" * 80)
    
    if passed:
        print("✅ ALL TESTS PASSED")
    else:
        print("❌ TESTS FAILED")
        print(f"\nMismatches ({len(mismatches)}):")
        for mismatch in mismatches:
            print(f"  - {mismatch}")
    
    print(f"\nReport written to {report_file}")
    print("=" * 80)
    
    return passed


if __name__ == "__main__":
    import sys
    
    expected_file = sys.argv[1] if len(sys.argv) > 1 else "test_data/expected_results.txt"
    actual_file = sys.argv[2] if len(sys.argv) > 2 else "test_data/cpp_results.txt"
    
    try:
        passed = compare_results(expected_file, actual_file)
        sys.exit(0 if passed else 1)
    
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

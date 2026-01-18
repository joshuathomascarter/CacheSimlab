#!/bin/bash
#
# Integration test runner for cache validation
#
# This script runs the full validation pipeline:
# 1. Generate test trace
# 2. Manually calculate expected results
# 3. Run C++ simulator
# 4. Parse C++ output
# 5. Compare and validate
#

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║              CACHE SIMULATOR VALIDATION PIPELINE                           ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
echo ""

# ============================================================================
# STEP 1: Generate trace
# ============================================================================
echo "📝 STEP 1: Generating test trace..."
echo "────────────────────────────────────────────────────────────────────────────"

python3 generate_trace.py
echo ""

# ============================================================================
# STEP 2: Calculate expected results (hand-trace)
# ============================================================================
echo "🔍 STEP 2: Computing expected results (manual trace)..."
echo "────────────────────────────────────────────────────────────────────────────"

python3 manual_trace.py
echo ""

# ============================================================================
# STEP 3: Run C++ simulator
# ============================================================================
echo "⚙️  STEP 3: Running C++ cache simulator..."
echo "────────────────────────────────────────────────────────────────────────────"

CPP_BINARY="../test_cache"
if [ ! -f "$CPP_BINARY" ]; then
    echo "⚠️  C++ binary not found at $CPP_BINARY"
    echo "   Trying to build..."
    cd ..
    if [ -f "CMakeLists.txt" ]; then
        cmake -B build && cmake --build build
        CPP_BINARY="build/test_cache"
    fi
    cd "$SCRIPT_DIR"
fi

if [ ! -f "$CPP_BINARY" ]; then
    echo "❌ ERROR: Could not find or build C++ binary"
    echo "   Expected at: $CPP_BINARY"
    exit 1
fi

echo "Using C++ binary: $CPP_BINARY"
python3 parse_cpp_output.py test_data/trace.txt "$CPP_BINARY" || {
    echo "⚠️  C++ simulator did not produce results"
    echo "   Make sure your C++ binary accepts trace file as argument"
}
echo ""

# ============================================================================
# STEP 4: Compare results
# ============================================================================
echo "✔️  STEP 4: Comparing expected vs actual results..."
echo "────────────────────────────────────────────────────────────────────────────"

python3 validate.py test_data/expected_results.txt test_data/cpp_results.txt

if [ $? -eq 0 ]; then
    RESULT="✅ PASSED"
else
    RESULT="❌ FAILED"
fi

echo ""
echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║                     VALIDATION RESULT: $RESULT                          ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "📊 Output files:"
echo "   - test_data/trace.txt                (Input addresses)"
echo "   - test_data/expected_results.txt     (Hand-calculated ground truth)"
echo "   - test_data/cpp_results.txt          (C++ simulator output)"
echo "   - test_data/validation_report.txt    (Comparison report)"
echo ""

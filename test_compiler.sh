#!/bin/bash

# Comprehensive test script for the C-like compiler

set -e

echo "======================================"
echo "C-like Language Compiler Test Suite"
echo "======================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if compiler exists
if [ ! -f "build/compiler" ]; then
    echo "Error: Compiler not found. Run 'make build' first."
    exit 1
fi

# Function to test a program
test_program() {
    local name=$1
    local file=$2
    local expected=$3
    
    echo -n "Testing $name... "
    
    # Compile
    ./build/compiler "$file" "${file%.c}.ll" > /dev/null 2>&1
    
    # Run with lli-20 and capture exit code
    lli-20 "${file%.c}.ll" > /dev/null 2>&1
    local result=$?
    
    if [ "$result" -eq "$expected" ]; then
        echo -e "${GREEN}✓ PASS${NC} (returned $result)"
        return 0
    else
        echo -e "${RED}✗ FAIL${NC} (expected $expected, got $result)"
        return 1
    fi
}

# Run tests
echo "Running compiler tests..."
echo ""

passed=0
failed=0

# Test 1: Simple arithmetic
if test_program "Simple arithmetic" "examples/simple.c" 50; then
    passed=$((passed + 1))
else
    failed=$((failed + 1))
fi

# Test 2: Factorial
if test_program "Factorial recursion" "examples/factorial.c" 120; then
    passed=$((passed + 1))
else
    failed=$((failed + 1))
fi

# Test 3: Fibonacci
if test_program "Fibonacci iteration" "examples/fibonacci.c" 55; then
    passed=$((passed + 1))
else
    failed=$((failed + 1))
fi

# Test 4: Control flow (returns 106 = (20 + 4950) % 256)
if test_program "Control flow" "examples/control_flow.c" 106; then
    passed=$((passed + 1))
else
    failed=$((failed + 1))
fi

# Test 5: Floating point (returns 0)
if test_program "Floating point ops" "examples/float_ops.c" 0; then
    passed=$((passed + 1))
else
    failed=$((failed + 1))
fi

echo ""
echo "======================================"
echo "Test Results:"
echo -e "${GREEN}Passed: $passed${NC}"
echo -e "${RED}Failed: $failed${NC}"
echo "======================================"

if [ "$failed" -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed.${NC}"
    exit 1
fi

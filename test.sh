#!/bin/bash

echo "Building C-like Language Compiler..."

# Try to build with CMake first, fallback to Makefile
if command -v cmake &> /dev/null && command -v llvm-config &> /dev/null; then
    echo "Using CMake build system..."
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
    COMPILER_PATH="./build/clike-compiler"
elif command -v make &> /dev/null && command -v llvm-config &> /dev/null; then
    echo "Using Makefile build system..."
    make clean
    make release
    COMPILER_PATH="./clike-compiler"
else
    echo "Error: Could not find required build tools (cmake/make and llvm-config)"
    exit 1
fi

if [ ! -f "$COMPILER_PATH" ]; then
    echo "Error: Build failed - compiler executable not found"
    exit 1
fi

echo "Build successful! Compiler executable: $COMPILER_PATH"
echo ""

echo "Testing examples..."
echo "==================="

# Test each example file
for example in examples/*.cl; do
    if [ -f "$example" ]; then
        echo ""
        echo "Testing: $example"
        echo "-------------------"
        $COMPILER_PATH "$example"
        echo ""
    fi
done

echo ""
echo "Testing interactive mode (type 'exit' to quit):"
echo "==============================================="
echo "Try entering: def test(x) { return x * 2; }"
echo "Or: 5 + 3 * 2"
echo ""
$COMPILER_PATH -i
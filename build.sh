#!/bin/bash

# Build script for C-like Language Compiler

set -e

echo "==================================="
echo "C-like Language LLVM IR Compiler"
echo "==================================="
echo ""

# Create build directory
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

# Configure with CMake
echo "Configuring with CMake..."
cd build
cmake -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm .. > /dev/null

# Build
echo "Building compiler..."
make

echo ""
echo "✓ Build successful!"
echo ""
echo "Usage: ./build/compiler <input_file> [output_file]"
echo ""
echo "Try it out:"
echo "  ./build/compiler examples/simple.c simple.ll"
echo "  lli-20 simple.ll && echo \"Exit code: \$?\""

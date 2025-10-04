#!/bin/bash

# Build script for C-like Language Compiler

set -e

echo "Building C-like Language Compiler..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: build/bin/clike_compiler"

# Test with an example
if [ -f "bin/clike_compiler" ]; then
    echo ""
    echo "Testing with factorial example..."
    ../bin/clike_compiler ../examples/factorial.clike
fi
cmake_minimum_required(VERSION 3.13)
project(CLikeCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add executable
add_executable(clike_compiler
    src/main.cpp
    src/ast.cpp
    src/ir_generator.cpp
    src/lexer.cpp
    src/parser.cpp
)

# Link LLVM libraries directly
target_link_libraries(clike_compiler LLVMCore LLVMSupport)

# Set output directory
set_target_properties(clike_compiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
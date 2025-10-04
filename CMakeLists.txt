cmake_minimum_required(VERSION 3.13.4)
project(CLikeLanguageCompiler)

# Find required packages
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add LLVM include directories
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the libraries that correspond to the LLVM components
llvm_map_components_to_libnames(llvm_libs support core irreader analysis executionengine instcombine object runtimedyld scalaropts transformutils)

# Add executable
add_executable(clike-compiler
    main.cpp
    ast.cpp
    lexer.cpp
    parser.cpp
    codegen.cpp
)

# Link against LLVM libraries
target_link_libraries(clike-compiler ${llvm_libs})

# Compiler-specific options
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(clike-compiler PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# Enable optimization for release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
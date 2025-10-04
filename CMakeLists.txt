cmake_minimum_required(VERSION 3.13)
project(CLikeCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Handle zstd dependency for LLVM
find_package(zstd QUIET)
if(NOT zstd_FOUND)
    # Try to find it manually
    find_library(ZSTD_LIBRARY NAMES zstd libzstd.so.1)
    if(ZSTD_LIBRARY)
        add_library(zstd::libzstd_shared SHARED IMPORTED)
        set_target_properties(zstd::libzstd_shared PROPERTIES
            IMPORTED_LOCATION "${ZSTD_LIBRARY}"
        )
        message(STATUS "Created zstd target from ${ZSTD_LIBRARY}")
    else()
        message(WARNING "zstd library not found, LLVM linking may fail")
    endif()
endif()

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Add LLVM definitions and include directories
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the LLVM libraries
llvm_map_components_to_libnames(llvm_libs 
    support 
    core 
    irreader 
    analysis
    native
    target
)

# Add executable
add_executable(compiler
    main.cpp
    codegen.cpp
)

# Link LLVM libraries
target_link_libraries(compiler ${llvm_libs})

# Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(compiler PRIVATE -Wall -Wextra)
endif()

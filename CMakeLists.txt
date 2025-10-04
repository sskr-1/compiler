cmake_minimum_required(VERSION 3.16)
project(mini_c_codegen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Some LLVM builds reference an imported target "zstd::libzstd_shared" in
# LLVMSupport's interface, but certain distros don't provide a zstd CMake
# package. Provide a minimal imported target that points to the system libzstd
# so that LLVM's config can be consumed without errors.
find_library(ZSTD_LIBRARY NAMES zstd libzstd)
if (ZSTD_LIBRARY AND NOT TARGET zstd::libzstd_shared)
  add_library(zstd::libzstd_shared SHARED IMPORTED GLOBAL)
  set_target_properties(zstd::libzstd_shared PROPERTIES
    IMPORTED_LOCATION "${ZSTD_LIBRARY}"
  )
endif()

# Allow user to override LLVM_DIR externally if needed
# Example: cmake -S . -B build -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm
find_package(LLVM CONFIG REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(mini_codegen
  src/main.cpp
  src/CodeGen.cpp
)

target_include_directories(mini_codegen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Prefer using component libraries for stability
if (COMMAND llvm_map_components_to_libnames)
  llvm_map_components_to_libnames(LLVM_LIBS core support)
  target_link_libraries(mini_codegen PRIVATE ${LLVM_LIBS})
else()
  # Fallback to imported target if available
  if (TARGET LLVM)
    target_link_libraries(mini_codegen PRIVATE LLVM)
  else()
    message(FATAL_ERROR "Could not determine how to link LLVM. Please set LLVM_DIR or install llvm-dev.")
  endif()
endif()

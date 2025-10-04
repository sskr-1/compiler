cmake_minimum_required(VERSION 3.16)
project(ccl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Some LLVM builds export a dependency on zstd::libzstd_shared in LLVMSupport's
# INTERFACE. Define a stub target if it's missing so find_package(LLVM) works
# even when zstd is not installed.
if (NOT TARGET zstd::libzstd_shared)
  add_library(zstd::libzstd_shared INTERFACE IMPORTED)
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVM at ${LLVM_DIR}")

add_executable(ccl
  src/main.cpp
  src/lexer.cpp
  src/parser.cpp
  src/codegen.cpp
)

target_include_directories(ccl PRIVATE src ${LLVM_INCLUDE_DIRS})

# Use llvm_map_components_to_libnames for portable linking across LLVM versions
llvm_map_components_to_libnames(LLVM_LIBS core support)
target_link_libraries(ccl PRIVATE ${LLVM_LIBS})

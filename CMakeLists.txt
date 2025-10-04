cmake_minimum_required(VERSION 3.13)
project(CLikeCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add LLVM libraries
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate lexer and parser
FLEX_TARGET(CLikeLexer lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
BISON_TARGET(CLikeParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)

# Add the generated files to the parser target
add_flex_bison_dependency(CLikeLexer CLikeParser)

# Create executable
add_executable(clike_compiler
    main.cpp
    ast_nodes.cpp
    codegen.cpp
    ${BISON_CLikeParser_OUTPUTS}
    ${FLEX_CLikeLexer_OUTPUTS}
)

# Link libraries
target_link_libraries(clike_compiler ${llvm_libs})

# Set compiler flags
target_compile_options(clike_compiler PRIVATE -Wall -Wextra)

# Copy test files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/tests DESTINATION ${CMAKE_BINARY_DIR})
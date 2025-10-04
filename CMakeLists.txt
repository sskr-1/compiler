cmake_minimum_required(VERSION 3.12)
project(LLVM_IR_Generator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Print LLVM information
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libraries: ${LLVM_LIBRARY_DIRS}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add executable
add_executable(llvm_codegen
    lex.yy.c
    parser.tab.c
    parser.tab.h
    main.cpp
    ast.cpp
    codegen.cpp
    ast.h
    codegen.h
)

# Link against LLVM libraries
llvm_map_components_to_libnames(llvm_libs
    core
    irreader
    orcjit
    support
    native
    mcjit
    interpreter
)

target_link_libraries(llvm_codegen ${llvm_libs})

# Set compiler flags for Flex/Bison generated code
set_source_files_properties(
    lex.yy.c
    parser.tab.c
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-function -Wno-sign-compare"
)

# Custom target to generate parser and lexer
add_custom_target(generate_parser
    COMMAND bison -d parser.y
    COMMAND flex lex.l
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating parser and lexer files"
)

# Make the main target depend on the generated files
add_dependencies(llvm_codegen generate_parser)

# Copy generated files to build directory for convenience
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.l COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.y COPYONLY)
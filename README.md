# C-like Language LLVM IR Code Generator

A compiler for a C-like language that generates LLVM IR code. This project demonstrates how to build a complete compiler frontend with lexical analysis, parsing, and LLVM IR code generation.

## Features

The compiler supports the following C-like language features:

### Data Types
- `int` - 32-bit integers
- `float` - single precision floating point
- `double` - double precision floating point
- `void` - for functions with no return value

### Operators
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `<`, `>`, `==`, `!=`, `<=`, `>=`
- Unary: `-` (negation), `!` (logical not)
- Assignment: `=`

### Control Flow
- `if` / `else` statements
- `while` loops
- `return` statements

### Functions
- Function declarations with parameters
- Function calls
- Recursion support

## Building

### Prerequisites
- CMake 3.13 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- LLVM development libraries (version 10 or higher recommended)

### Installing LLVM

**Ubuntu/Debian:**
```bash
sudo apt-get install llvm-dev clang
```

**macOS:**
```bash
brew install llvm
export LLVM_DIR=/usr/local/opt/llvm/lib/cmake/llvm
```

**Fedora/RHEL:**
```bash
sudo dnf install llvm-devel
```

### Compilation

```bash
mkdir build
cd build
cmake ..
make
```

This will create a `compiler` executable in the build directory.

## Usage

```bash
./compiler <input_file> [output_file]
```

- `input_file`: Path to the C-like source file
- `output_file`: (Optional) Path for the output LLVM IR file (default: `output.ll`)

### Examples

Compile a simple program:
```bash
./compiler ../examples/simple.c simple.ll
```

Compile the factorial example:
```bash
./compiler ../examples/factorial.c factorial.ll
```

## Example Programs

The `examples/` directory contains several example programs:

1. **simple.c** - Basic arithmetic operations
2. **factorial.c** - Recursive factorial function
3. **fibonacci.c** - Iterative fibonacci calculation
4. **control_flow.c** - Demonstrates if/else and while loops
5. **float_ops.c** - Floating point arithmetic

## Language Syntax

### Variable Declaration
```c
int x = 10;
double y = 3.14;
float z;
```

### Function Definition
```c
int add(int a, int b) {
    return a + b;
}
```

### Control Structures

**If/Else:**
```c
if (x > 10) {
    y = x * 2;
} else {
    y = x / 2;
}
```

**While Loop:**
```c
while (i < 10) {
    sum = sum + i;
    i = i + 1;
}
```

## Architecture

The compiler consists of the following components:

1. **Lexer** (`lexer.h`) - Tokenizes the input source code
2. **Parser** (`parser.h`) - Builds an Abstract Syntax Tree (AST)
3. **AST** (`ast.h`) - Defines node types for the syntax tree
4. **Code Generator** (`codegen.h`, `codegen.cpp`) - Generates LLVM IR from AST
5. **Main Driver** (`main.cpp`) - Orchestrates the compilation process

## Generated LLVM IR

The compiler generates standard LLVM IR that can be:
- Compiled to machine code using `llc`
- Optimized using `opt`
- Linked into executables using `clang` or `gcc`

### Example Workflow

```bash
# Compile to LLVM IR
./compiler examples/factorial.c factorial.ll

# Optimize the IR
opt -O3 factorial.ll -o factorial_opt.bc

# Compile to native code
llc factorial_opt.bc -o factorial.s

# Create executable
clang factorial.s -o factorial

# Run
./factorial
echo $?  # Print the return value
```

## Limitations

Current limitations of the language:
- No pointer types
- No arrays
- No structs/classes
- No for loops (only while)
- No string literals
- No standard library functions
- Limited type system

## Extending the Compiler

To add new features:

1. Update the lexer to recognize new tokens
2. Modify the parser to handle new syntax
3. Add new AST node types
4. Implement code generation for new nodes

## License

This is an educational project demonstrating LLVM IR code generation.

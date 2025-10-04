# C-like Language LLVM IR Code Generator

A complete compiler frontend that generates LLVM IR for a C-like programming language, implemented in C++.

## Features

- **Lexical Analysis**: Complete lexer supporting C-like tokens
- **Syntax Analysis**: Parser with comprehensive grammar rules
- **Abstract Syntax Tree**: Full AST representation of the language
- **LLVM IR Generation**: Complete code generator that produces LLVM IR
- **Optimization**: Built-in IR optimization passes
- **Multiple Data Types**: Support for int, float, char, bool, and void types
- **Control Flow**: if/else, while, for loops with break/continue
- **Functions**: Function definitions, calls, and parameters
- **Variables**: Local variables with scoping
- **Expressions**: Arithmetic, logical, comparison, and assignment operations

## Language Features

### Data Types
- `int` - 32-bit integers
- `float` - Single precision floating point
- `char` - 8-bit characters
- `bool` - Boolean values (true/false)
- `void` - No return value

### Control Structures
- `if` / `else` statements
- `while` loops
- `for` loops
- `break` and `continue` statements
- `return` statements

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Assignment: `=`, `+=`, `-=`, `*=`, `/=`
- Increment/Decrement: `++`, `--` (prefix and postfix)

### Functions
- Function definitions with parameters
- Function calls
- Return values
- Recursive functions

## Building

### Prerequisites
- LLVM development libraries
- Flex (lexical analyzer generator)
- Bison (parser generator)
- CMake 3.13 or higher
- C++17 compatible compiler

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
make

# The executable will be created as 'clike_compiler'
```

## Usage

```bash
# Compile a C-like program and generate LLVM IR
./clike_compiler input_file.cl

# Output LLVM IR to a file
./clike_compiler -o output.ll input_file.cl

# Optimize the generated IR
./clike_compiler -O input_file.cl

# Verify the generated IR
./clike_compiler -v input_file.cl

# Print the AST instead of generating IR
./clike_compiler -ast input_file.cl

# Show help
./clike_compiler --help
```

## Example Programs

The `tests/` directory contains several example programs:

- `hello_world.cl` - Simple program with variables and arithmetic
- `factorial.cl` - Recursive factorial function
- `loops.cl` - While and for loop examples
- `arithmetic.cl` - Various arithmetic and logical operations
- `functions.cl` - Function definitions and calls
- `variables.cl` - Variable declarations and assignments

## Example Usage

```bash
# Compile and run a simple example
./clike_compiler tests/hello_world.cl

# Generate optimized IR
./clike_compiler -O -o hello_optimized.ll tests/hello_world.cl
```

## Generated LLVM IR Example

For the following C-like code:
```c
int main() {
    int x = 42;
    int y = 10;
    int sum = x + y;
    return sum;
}
```

The compiler generates LLVM IR like:
```llvm
define i32 @main() {
entry:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %sum = alloca i32, align 4
  store i32 42, i32* %x, align 4
  store i32 10, i32* %y, align 4
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %addtmp = add i32 %0, %1
  store i32 %addtmp, i32* %sum, align 4
  %2 = load i32, i32* %sum, align 4
  ret i32 %2
}
```

## Project Structure

```
├── CMakeLists.txt      # Build configuration
├── lex.l              # Lexer specification
├── parser.y           # Parser specification
├── ast_nodes.h        # AST node class definitions
├── ast_nodes.cpp      # AST node implementations
├── codegen.h          # LLVM IR code generator header
├── codegen.cpp        # LLVM IR code generator implementation
├── main.cpp           # Main driver program
├── tests/             # Example programs
│   ├── hello_world.cl
│   ├── factorial.cl
│   ├── loops.cl
│   ├── arithmetic.cl
│   ├── functions.cl
│   └── variables.cl
└── README.md          # This file
```

## Architecture

The compiler follows a traditional multi-pass architecture:

1. **Lexical Analysis**: `lex.l` defines tokens and lexical rules
2. **Syntax Analysis**: `parser.y` defines grammar rules and builds AST
3. **AST Construction**: `ast_nodes.h/cpp` defines the AST structure
4. **Code Generation**: `codegen.h/cpp` generates LLVM IR from AST
5. **Optimization**: Optional IR optimization passes
6. **Output**: Generated LLVM IR

## Contributing

This is a complete educational implementation of a C-like language compiler. The code is well-documented and structured to be easily extensible for additional language features.

## License

This project is provided as educational material for learning compiler construction and LLVM IR generation.
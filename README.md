# LLVM IR Code Generator for C-like Language

This project implements a complete LLVM IR (Intermediate Representation) code generator for a C-like programming language. It uses Flex and Bison for lexical analysis and parsing, and LLVM's C++ API for code generation.

## Features

- **Complete C-like syntax** supporting:
  - Variable declarations and assignments
  - Function declarations and definitions
  - Control flow statements (if, while, for)
  - Arithmetic and logical expressions
  - Function calls
  - Return statements

- **LLVM IR generation** with:
  - Proper type handling (int, float, double, char, void)
  - Function definitions and calls
  - Control flow implementation
  - Variable allocation and management

## Project Structure

```
├── ast.h, ast.cpp           # Abstract Syntax Tree definitions and implementations
├── codegen.h, codegen.cpp   # LLVM IR code generation
├── lex.l                    # Flex lexer definition
├── parser.y                 # Bison parser definition
├── parser.tab.h             # Parser token definitions
├── main.cpp                 # Main driver program
├── CMakeLists.txt           # Build configuration
├── sample.cl                # Sample C-like program
└── README.md                # This file
```

## Prerequisites

- **CMake** 3.12 or higher
- **LLVM** 10.0 or higher with development headers
- **Flex** (for lexer generation)
- **Bison** (for parser generation)
- **C++17** compatible compiler

## Building the Project

1. **Install dependencies** (Ubuntu/Debian):
   ```bash
   sudo apt-get update
   sudo apt-get install cmake llvm-10-dev libllvm10 flex bison
   ```

2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure and build**:
   ```bash
   cmake ..
   make
   ```

   This will automatically generate the lexer and parser files using Flex and Bison.

## Usage

### Basic Usage

```bash
./llvm_codegen sample.cl
```

This will parse `sample.cl` and output the generated LLVM IR to stdout.

### Output to File

```bash
./llvm_codegen -o output.ll sample.cl
```

This will write the LLVM IR to `output.ll`.

### Command Line Options

- `-o <file>` : Specify output file for LLVM IR
- `-h, --help` : Show help message
- `-v, --version` : Show version information

## Language Syntax

### Variable Declarations

```c
int x;
float y = 3.14;
int arr[10];
```

### Function Definitions

```c
int add(int a, int b) {
    return a + b;
}

void printHello() {
    // function body
}
```

### Control Flow

```c
if (x > 0) {
    return x;
} else {
    return 0;
}

while (i < 10) {
    i = i + 1;
}

for (i = 0; i < 10; i = i + 1) {
    sum = sum + i;
}
```

### Expressions

```c
int result = (a + b) * c / d;
int isEqual = (x == y);
int logical = (a && b) || (c && d);
```

## Sample Program

The included `sample.cl` file demonstrates:

```c
// Function to calculate factorial
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Main function
int main() {
    int i;
    int result;

    for (i = 1; i <= 5; i = i + 1) {
        result = factorial(i);
    }

    return result;
}
```

## Architecture

### 1. Lexical Analysis (lex.l)
- Tokenizes input source code
- Recognizes keywords, identifiers, literals, operators, and punctuation

### 2. Syntax Analysis (parser.y)
- Parses tokens according to C-like grammar
- Builds Abstract Syntax Tree (AST)

### 3. AST Node Classes (ast.h, ast.cpp)
- Define data structures for program elements
- Implement printing and code generation methods

### 4. Code Generation (codegen.h, codegen.cpp)
- Traverses AST to generate LLVM IR
- Manages LLVM context, module, and IR builder
- Handles symbol table and type conversion

### 5. Main Driver (main.cpp)
- Coordinates lexing, parsing, and code generation
- Handles command-line arguments and file I/O

## Extending the Language

To add new features:

1. **Add tokens** in `lex.l` and `parser.tab.h`
2. **Update grammar** in `parser.y`
3. **Create AST nodes** in `ast.h` and `ast.cpp`
4. **Implement code generation** in AST node methods
5. **Add type support** in `codegen.cpp`

## Troubleshooting

### Common Issues

1. **Missing LLVM development headers**:
   - Install `libllvm-10-dev` or equivalent package

2. **Flex/Bison not found**:
   - Install `flex` and `bison` packages

3. **Build errors with generated files**:
   - Clean build directory and rebuild: `rm -rf build && mkdir build && cd build && cmake .. && make`

### Debugging

- Use the `-v` flag to check LLVM version compatibility
- The program prints AST representation for debugging
- LLVM IR is output to help verify code generation

## Future Enhancements

Potential improvements for a production compiler:

- **Type checking and inference**
- **Array and pointer support**
- **String handling**
- **Struct and union definitions**
- **Global variable support**
- **Error recovery in parser**
- **Optimization passes**
- **Code generation for different targets**

## License

This project is provided as-is for educational purposes. Feel free to use and modify as needed.
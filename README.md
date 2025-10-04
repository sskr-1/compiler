# C-like Language Compiler with LLVM IR Generation

This project implements a compiler for a C-like language that generates LLVM Intermediate Representation (IR) code. The compiler is written in C++ and uses the LLVM library for IR generation.

## Features

- **Lexical Analysis**: Tokenizes C-like language constructs
- **Parsing**: Recursive descent parser with AST generation
- **AST**: Abstract Syntax Tree representation of the language
- **LLVM IR Generation**: Converts AST to LLVM IR
- **Language Constructs**:
  - Functions with parameters
  - Variables with type inference (double)
  - Arithmetic operations (+, -, *, /)
  - Comparison operations (<, >, <=, >=, ==, !=)
  - Control flow (if-else, while loops)
  - Function calls
  - Return statements
  - Block statements with scoping

## Language Syntax

### Functions
```c
functionName(param1, param2) {
    // function body
    return expression;
}
```

### Variables
```c
var variableName = expression;
```

### Control Flow
```c
if (condition) {
    // then block
} else {
    // else block
}

while (condition) {
    // loop body
}
```

### Expressions
- Numbers: `42`, `3.14`
- Variables: `x`, `myVar`
- Binary operations: `a + b`, `x * y`, `a < b`
- Function calls: `factorial(5)`

## Building

### Prerequisites
- CMake 3.13 or higher
- LLVM development libraries
- C++17 compatible compiler

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
# Compile a C-like program to LLVM IR
./clike_compiler input.clike

# Output to a file
./clike_compiler -o output.ll input.clike

# Verbose output
./clike_compiler -v input.clike
```

## Example Programs

See the `examples/` directory for sample C-like programs:

- `factorial.clike`: Recursive factorial calculation
- `fibonacci.clike`: Fibonacci sequence calculation
- `simple.clike`: Basic arithmetic operations
- `loops.clike`: Loop constructs and function calls

## Project Structure

```
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── ast.h              # AST node definitions
│   ├── ir_generator.h     # LLVM IR generator
│   ├── lexer.h            # Lexical analyzer
│   └── parser.h           # Parser
├── src/                   # Source files
│   ├── ast.cpp            # AST implementations
│   ├── ir_generator.cpp   # IR generator implementation
│   ├── lexer.cpp          # Lexer implementation
│   ├── parser.cpp         # Parser implementation
│   └── main.cpp           # Main driver program
└── examples/              # Example programs
    ├── factorial.clike
    ├── fibonacci.clike
    ├── simple.clike
    └── loops.clike
```

## LLVM IR Output

The compiler generates LLVM IR code that can be further processed by LLVM tools:

```llvm
define double @factorial(double %n) {
entry:
  %result = alloca double
  store double 1.000000e+00, double* %result
  %i = alloca double
  store double 1.000000e+00, double* %i
  br label %loop

loop:
  %i.load = load double, double* %i
  %cmp = fcmp ule double %i.load, %n
  br i1 %cmp, label %loopbody, label %afterloop

loopbody:
  %result.load = load double, double* %result
  %multmp = fmul double %result.load, %i.load
  store double %multmp, double* %result
  %i.load2 = load double, double* %i
  %addtmp = fadd double %i.load2, 1.000000e+00
  store double %addtmp, double* %i
  br label %loop

afterloop:
  %result.load3 = load double, double* %result
  ret double %result.load3
}
```

## Extending the Language

The compiler is designed to be extensible. To add new language features:

1. **Lexer**: Add new token types in `lexer.h` and implement recognition in `lexer.cpp`
2. **Parser**: Add parsing rules in `parser.cpp`
3. **AST**: Define new AST node types in `ast.h`
4. **IR Generation**: Implement code generation in `ir_generator.cpp`

## License

This project is provided as an educational example of compiler construction using LLVM.
# C-like Language Compiler with LLVM IR Generation

This is a complete implementation of an LLVM IR code generator for a C-like programming language, written in C++. The compiler includes a lexer, parser, Abstract Syntax Tree (AST), and LLVM IR code generation.

## Features

### Language Features
- **Variable Declarations**: `int x = 5;`, `double y = 3.14;`, `bool flag = true;`
- **Function Definitions**: `def functionName(param1 param2) { ... }`
- **Function Calls**: `functionName(arg1, arg2)`
- **Arithmetic Operations**: `+`, `-`, `*`, `/`
- **Comparison Operations**: `<`, `>`, `==`, `!=`, `<=`, `>=`
- **Assignment**: `x = expression`
- **Control Flow**: `if condition then { ... } else { ... }`
- **Return Statements**: `return expression;`
- **External Function Declarations**: `extern functionName(params);`

### Compiler Features
- **Lexical Analysis**: Tokenizes source code into language tokens
- **Syntax Analysis**: Parses tokens into Abstract Syntax Tree (AST)
- **Code Generation**: Generates LLVM IR from AST
- **Optimization**: Basic LLVM optimization passes
- **Interactive Mode**: REPL for testing expressions and functions
- **File Compilation**: Compile complete source files

## Architecture

### Components

1. **Lexer** (`lexer.h/cpp`): Tokenizes input source code
2. **Parser** (`parser.h/cpp`): Builds AST from tokens using recursive descent parsing
3. **AST** (`ast.h/cpp`): Abstract Syntax Tree node definitions and code generation
4. **CodeGen** (`codegen.h/cpp`): LLVM IR code generation context and utilities
5. **Main** (`main.cpp`): Driver program with interactive and file compilation modes

### AST Node Types

- **Expressions**: Numbers, Variables, Binary Operations, Function Calls, If Expressions
- **Statements**: Variable Declarations, Assignments, Blocks, Return Statements
- **Functions**: Prototypes and Function Definitions

## Building

### Prerequisites
- C++17 compatible compiler (GCC, Clang)
- LLVM development libraries (version 10 or later)
- CMake (optional) or Make

### Install LLVM (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install llvm-dev clang cmake
```

### Install LLVM (macOS with Homebrew)
```bash
brew install llvm cmake
```

### Build Options

#### Option 1: CMake (Recommended)
```bash
mkdir build
cd build
cmake ..
make
```

#### Option 2: Makefile
```bash
make release
```

#### Quick Test
```bash
./test.sh
```

## Usage

### Command Line Options
```bash
./clike-compiler <input_file>     # Compile a file
./clike-compiler -i               # Interactive mode
./clike-compiler --help           # Show help
```

### Interactive Mode
```bash
$ ./clike-compiler -i
C-like Language Compiler - Interactive Mode
Type expressions/functions and see the generated LLVM IR.
Type 'exit' to quit.

>> def add(x y) { return x + y; }
Parsed a function definition:
define double @add(double %x, double %y) {
entry:
  %x1 = alloca double, align 8
  %y2 = alloca double, align 8
  store double %x, ptr %x1, align 8
  store double %y, ptr %y2, align 8
  %x3 = load double, ptr %x1, align 8
  %y4 = load double, ptr %y2, align 8
  %addtmp = fadd double %x3, %y4
  ret double %addtmp
}

>> 5 + 3 * 2
Parsed a top-level expression:
define double @__anon_expr() {
entry:
  %multmp = fmul double 3.000000e+00, 2.000000e+00
  %addtmp = fadd double 5.000000e+00, %multmp
  ret double %addtmp
}

>> exit
```

### File Compilation
Create a source file (e.g., `program.cl`):
```c
def factorial(n) {
    if n < 2 then {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

def main() {
    double result = factorial(5);
    return result;
}
```

Compile it:
```bash
./clike-compiler program.cl
```

## Example Programs

### Basic Arithmetic
```c
def calculate(x y) {
    double temp = x + y;
    return temp * 2;
}
```

### Conditional Logic
```c
def max(a b) {
    if a > b then {
        return a;
    } else {
        return b;
    }
}
```

### Variable Declarations
```c
def test() {
    int x = 10;
    double y = 3.14;
    bool flag = true;
    return x + y;
}
```

### External Functions
```c
extern sin(x);
extern printf(format ...);

def trigonometry(angle) {
    return sin(angle) * 2.0;
}
```

## Generated LLVM IR

The compiler generates human-readable LLVM IR that can be:
- Further compiled with `llc` to assembly
- Optimized with `opt`
- Executed with `lli` (LLVM interpreter)
- Linked with other LLVM modules

Example compilation pipeline:
```bash
# Generate IR
./clike-compiler program.cl > program.ll

# Optimize IR
opt -O2 program.ll -o program_opt.ll

# Compile to assembly
llc program_opt.ll -o program.s

# Assemble and link
clang program.s -o program
```

## Language Grammar (Simplified)

```
Program     ::= (Definition | Extern | Expression)*
Definition  ::= 'def' Prototype Block
Extern      ::= 'extern' Prototype
Prototype   ::= Identifier '(' Identifier* ')'
Block       ::= '{' Statement* '}'
Statement   ::= VarDecl | Assignment | Return | Expression | Block | If
VarDecl     ::= Type Identifier ('=' Expression)? ';'
Assignment  ::= Identifier '=' Expression ';'
Return      ::= 'return' Expression? ';'
If          ::= 'if' Expression 'then' Statement ('else' Statement)?
Expression  ::= Primary (BinOp Primary)*
Primary     ::= Number | Identifier | Call | '(' Expression ')'
Call        ::= Identifier '(' (Expression (',' Expression)*)? ')'
BinOp       ::= '+' | '-' | '*' | '/' | '<' | '>' | '=='
Type        ::= 'int' | 'double' | 'bool' | 'void'
```

## Error Handling

The compiler provides basic error reporting for:
- Syntax errors during parsing
- Undefined variables and functions
- Type mismatches
- Invalid function calls (wrong number of arguments)

## Limitations

- No advanced type system (mostly uses double precision floating point)
- No arrays or complex data structures
- No loops (while, for) - only recursion
- Limited standard library (only extern declarations)
- No memory management beyond basic stack allocation

## Extension Ideas

1. **Add loops**: `for` and `while` statements
2. **Add arrays**: `int arr[10]` syntax
3. **Add structs**: Custom data types
4. **Add pointers**: Memory address manipulation
5. **Add standard library**: Built-in functions (printf, malloc, etc.)
6. **Add modules**: Import/export system
7. **Better error messages**: Line numbers and better diagnostics
8. **JIT compilation**: Execute code directly

## Contributing

This is a educational compiler implementation. Feel free to extend it with additional features or improvements!

## License

This project is provided as-is for educational purposes.
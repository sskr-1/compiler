# Quick Start Guide

Get up and running with the C-like language compiler in 5 minutes!

## Prerequisites

- Linux system with LLVM 20 installed
- CMake 3.13+
- C++17 compatible compiler

## Installation

### Step 1: Clone or download the project

```bash
cd /workspace
```

### Step 2: Build the compiler

```bash
./build.sh
```

This will:
- Create a `build` directory
- Configure CMake
- Compile the compiler
- Output: `build/compiler` executable

### Step 3: Verify the build

```bash
./test_compiler.sh
```

Expected output:
```
✓ PASS - Simple arithmetic (returned 50)
✓ PASS - Factorial recursion (returned 120)
✓ PASS - Fibonacci iteration (returned 55)
✓ PASS - Control flow (returned 106)
✓ PASS - Floating point ops (returned 0)

All tests passed!
```

## Your First Program

### 1. Create a source file

```bash
cat > hello.c << 'EOF'
int main() {
    int x = 42;
    int y = x + 8;
    return y;
}
EOF
```

### 2. Compile to LLVM IR

```bash
./build/compiler hello.c hello.ll
```

### 3. View the generated IR

```bash
cat hello.ll
```

### 4. Run the program

```bash
lli-20 hello.ll
echo $?  # Prints: 50
```

## Examples

### Factorial

```c
int factorial(int n) {
    if (n < 2) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    return factorial(5);  // Returns 120
}
```

Compile and run:
```bash
./build/compiler factorial.c factorial.ll
lli-20 factorial.ll && echo $?
```

### Fibonacci

```c
int fibonacci(int n) {
    int a = 0;
    int b = 1;
    int i = 0;
    
    while (i < n) {
        int temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }
    
    return a;
}

int main() {
    return fibonacci(10);  // Returns 55
}
```

Compile and run:
```bash
./build/compiler fibonacci.c fibonacci.ll
lli-20 fibonacci.ll && echo $?
```

## Language Reference

### Data Types
- `int` - 32-bit integer
- `float` - single precision float
- `double` - double precision float
- `void` - no return value

### Operators
```c
// Arithmetic
int a = 10 + 5 - 3 * 2 / 4;

// Comparison
if (x > 10) { }
if (x < 20) { }
if (x == 15) { }
if (x != 0) { }

// Unary
int neg = -x;
int not = !flag;
```

### Control Flow

**If/Else:**
```c
if (condition) {
    // then block
} else {
    // else block
}
```

**While Loop:**
```c
while (condition) {
    // loop body
}
```

**Return:**
```c
return expression;
return;  // for void functions
```

### Functions

```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(10, 20);
    return result;
}
```

## Compiling to Native Code

### Option 1: Direct compilation

```bash
./build/compiler program.c program.ll
llc program.ll -o program.s
clang program.s -o program
./program
echo $?
```

### Option 2: With optimization

```bash
./build/compiler program.c program.ll
opt -O3 program.ll -o program.bc
llc program.bc -o program.s
clang program.s -o program
./program
echo $?
```

## Troubleshooting

### Compiler not found
```bash
make build
```

### LLVM not found
The compiler expects LLVM 20 at `/usr/lib/llvm-20/`. If your LLVM is elsewhere:

```bash
cd build
cmake -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm ..
make
```

### Parse errors
Check your syntax:
- All statements end with `;`
- All blocks use `{ }`
- Function parameters have types
- Variables must be declared before use

## Tips

1. **View IR**: Always check the generated `.ll` file to understand what code is generated

2. **Optimize**: Use `opt -mem2reg` to convert stack allocations to SSA registers

3. **Debug**: Use `llvm-dis` to disassemble bitcode back to readable IR

4. **Verify**: LLVM automatically verifies IR correctness

5. **Profile**: Use LLVM's profiling tools to analyze performance

## Next Steps

- Read [README.md](README.md) for detailed features
- Check [ARCHITECTURE.md](ARCHITECTURE.md) for technical details
- See [SUMMARY.md](SUMMARY.md) for project overview
- Explore `examples/` directory for more programs

## Getting Help

1. Check compiler output for error messages
2. Verify syntax matches the examples
3. Review the example programs in `examples/`
4. Read the language reference above

Happy coding! 🚀

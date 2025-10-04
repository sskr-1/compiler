# Architecture Overview

This document provides a detailed overview of the LLVM IR code generator architecture for the C-like language compiler.

## Components

### 1. Lexer (`lexer.h`)

The lexer performs lexical analysis, converting the input source code into tokens.

**Key Features:**
- Recognizes keywords: `int`, `float`, `double`, `void`, `if`, `else`, `while`, `return`
- Handles identifiers, integers, and floating-point literals
- Supports single-line comments (`//`)
- Recognizes operators: `+`, `-`, `*`, `/`, `<`, `>`, `<=`, `>=`, `==`, `!=`, `=`, `!`

**Design:**
- Simple character-by-character scanning
- Stores current token state (identifier string, numeric values)
- Returns token type codes for parsing

### 2. Parser (`parser.h`)

The parser implements a recursive descent parser with operator precedence climbing for expressions.

**Key Features:**
- **Expression Parsing:** Handles binary operators with correct precedence and associativity
- **Statement Parsing:** Supports variable declarations, assignments, if/else, while loops, return
- **Function Parsing:** Handles function declarations with parameters and body
- **Operator Precedence:**
  - Assignment: 2
  - Equality/Inequality: 10
  - Comparison: 20
  - Addition/Subtraction: 30
  - Multiplication/Division: 40

**Design Pattern:**
- Top-down recursive descent
- Each grammar rule has a corresponding parsing function
- Operator precedence climbing for binary expressions
- Left-to-right associativity

### 3. Abstract Syntax Tree (`ast.h`)

Defines the AST node types that represent the program structure.

**Node Hierarchy:**
```
Node (base)
├── ExprNode (expressions)
│   ├── IntegerNode
│   ├── DoubleNode
│   ├── IdentifierNode
│   ├── BinaryOpNode
│   ├── UnaryOpNode
│   ├── FunctionCallNode
│   └── AssignmentNode
├── StmtNode (statements)
│   ├── VariableDeclNode
│   ├── BlockNode
│   ├── IfStmtNode
│   ├── WhileStmtNode
│   ├── ReturnStmtNode
│   └── ExprStmtNode
├── FunctionNode
└── ProgramNode (root)
```

**Design:**
- Each node has a `codeGen()` method for LLVM IR generation
- Uses `std::unique_ptr` for automatic memory management
- Visitor pattern through virtual methods

### 4. Code Generator (`codegen.h`, `codegen.cpp`)

Generates LLVM IR from the AST.

**CodeGenContext:**
- **LLVMContext:** LLVM's context object for type uniquing
- **IRBuilder:** Convenient API for generating LLVM instructions
- **Module:** Container for all generated functions and globals
- **namedValues:** Symbol table mapping variable names to stack allocations
- **currentFunction:** Tracks the function being compiled

**Code Generation Strategy:**

1. **Variables:** All variables are allocated on the stack using `alloca`
   - Enables simple SSA form through load/store operations
   - Stack allocations created in function entry block

2. **Type System:**
   - int → i32 (32-bit integer)
   - float → float (32-bit floating point)
   - double → double (64-bit floating point)
   - void → void
   - Automatic type conversions where needed

3. **Control Flow:**
   - **If statements:** Create basic blocks for then, else, and merge
   - **While loops:** Create basic blocks for condition, body, and after
   - **Short-circuit evaluation:** Not implemented (evaluates both sides)

4. **Functions:**
   - Parameters are copied to stack allocations
   - Return statements use LLVM's `ret` instruction
   - Function verification ensures well-formed IR

### 5. Main Driver (`main.cpp`)

Orchestrates the compilation pipeline.

**Pipeline:**
1. Read source file
2. Initialize LLVM targets
3. Lex and parse source code
4. Generate LLVM IR from AST
5. Print IR to stdout
6. Save IR to file

## Data Flow

```
Source Code (.c file)
    ↓
[Lexer] → Token Stream
    ↓
[Parser] → Abstract Syntax Tree (AST)
    ↓
[Code Generator] → LLVM IR Module
    ↓
Output IR (.ll file)
```

## LLVM IR Generation Details

### Expression Evaluation

All expressions are evaluated using a stack-based approach with explicit load/store operations.

**Example:**
```c
int a = 10;
int b = a + 5;
```

**Generated IR:**
```llvm
%a = alloca i32
store i32 10, ptr %a
%a1 = load i32, ptr %a
%addtmp = add i32 %a1, 5
%b = alloca i32
store i32 %addtmp, ptr %b
```

### Control Flow

**If Statement Example:**
```c
if (x > 0) {
    return 1;
} else {
    return -1;
}
```

**Generated IR Structure:**
```llvm
entry:
  %cond = icmp sgt i32 %x, 0
  br i1 %cond, label %then, label %else

then:
  ret i32 1

else:
  ret i32 -1

ifcont:  ; (may be unreachable if both branches return)
  ...
```

### Function Calls

Function calls use LLVM's `call` instruction with proper argument passing.

**Example:**
```c
int add(int a, int b) { return a + b; }
int main() { return add(3, 4); }
```

**Generated IR:**
```llvm
define i32 @add(i32 %a, i32 %b) {
  ; ... function body ...
}

define i32 @main() {
  %result = call i32 @add(i32 3, i32 4)
  ret i32 %result
}
```

## Memory Management

The compiler uses modern C++ smart pointers for automatic memory management:

- **std::unique_ptr:** Exclusive ownership of AST nodes
- **std::move:** Transfer ownership during AST construction
- **RAII:** Automatic cleanup when objects go out of scope

## Error Handling

Current error handling is basic:
- Parse errors print to stderr and return nullptr
- Code generation errors print diagnostics
- LLVM's verification catches IR errors

## Optimization Opportunities

The current implementation generates unoptimized IR. LLVM's optimization passes can be applied:

```bash
opt -O3 input.ll -o optimized.bc
```

Common optimizations that would help:
- **Mem2Reg:** Convert stack allocations to SSA registers
- **Constant folding:** Evaluate constant expressions at compile time
- **Dead code elimination:** Remove unreachable code
- **Inlining:** Inline small functions

## Extensibility

To add new features:

1. **New operators:** Add tokens to lexer, update parser precedence
2. **New statements:** Add AST node, parser function, code generation
3. **New types:** Update type mapping in CodeGenContext
4. **Arrays/pointers:** Extend type system and add indexing expressions
5. **For loops:** Add AST node and desugar to while loop

## Testing and Validation

The generated IR can be:
1. **Interpreted:** `lli-20 output.ll`
2. **Compiled:** `llc output.ll -o output.s`
3. **Optimized:** `opt -O3 output.ll -o output.bc`
4. **Validated:** LLVM automatically verifies IR correctness

## Performance Considerations

Current implementation priorities:
- **Correctness over speed:** Simple but correct code generation
- **Stack allocations:** Easy but not optimal (Mem2Reg pass fixes this)
- **No CSE:** Common subexpression elimination left to LLVM
- **Clear IR:** Human-readable output for debugging

## Limitations

1. No preprocessor
2. No includes/imports
3. No structs/classes
4. No pointers/arrays
5. No standard library
6. Limited type system
7. No string literals
8. No for loops (only while)
9. No break/continue
10. No switch statements

## Future Enhancements

Potential improvements:
- Add more data types (structs, arrays, pointers)
- Implement a standard library
- Add optimization hints
- Better error messages with line numbers
- Debug information generation (DWARF)
- Symbol table for semantic analysis
- Type checking before code generation

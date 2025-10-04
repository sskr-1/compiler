#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " <input_file>" << std::endl;
    std::cout << "       " << programName << " -i  (interactive mode)" << std::endl;
    std::cout << std::endl;
    std::cout << "This is a compiler for a C-like language that generates LLVM IR." << std::endl;
    std::cout << std::endl;
    std::cout << "Language features:" << std::endl;
    std::cout << "  - Variable declarations: int x = 5; double y = 3.14;" << std::endl;
    std::cout << "  - Function definitions: def foo(x y) { return x + y; }" << std::endl;
    std::cout << "  - Function calls: foo(1, 2)" << std::endl;
    std::cout << "  - Control flow: if x < y then { ... } else { ... }" << std::endl;
    std::cout << "  - Arithmetic: +, -, *, /, <, >" << std::endl;
    std::cout << "  - Assignments: x = y + 1" << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void handleTopLevelExpression(Parser& parser, CodeGenContext& context) {
    if (auto FnAST = parser.parseTopLevelFunction()) {
        if (auto FnIR = FnAST->codegen(context)) {
            std::cout << "Parsed a top-level expression:" << std::endl;
            FnIR->print(llvm::outs());
            std::cout << std::endl;
        }
    } else {
        Lexer::getNextTok(); // Skip token for error recovery
    }
}

void handleDefinition(Parser& parser, CodeGenContext& context) {
    if (auto FnAST = parser.parseFunctionDefinition()) {
        if (auto FnIR = FnAST->codegenFunction(context)) {
            std::cout << "Parsed a function definition:" << std::endl;
            FnIR->print(llvm::outs());
            std::cout << std::endl;
        }
    } else {
        Lexer::getNextTok(); // Skip token for error recovery
    }
}

void handleExtern(Parser& parser, CodeGenContext& context) {
    if (auto ProtoAST = parser.parseExternDeclaration()) {
        if (auto FnIR = ProtoAST->codegenFunction(context)) {
            std::cout << "Parsed an extern declaration:" << std::endl;
            FnIR->print(llvm::outs());
            std::cout << std::endl;
        }
    } else {
        Lexer::getNextTok(); // Skip token for error recovery
    }
}

void interactiveMode() {
    std::cout << "C-like Language Compiler - Interactive Mode" << std::endl;
    std::cout << "Type expressions/functions and see the generated LLVM IR." << std::endl;
    std::cout << "Type 'exit' to quit." << std::endl;
    std::cout << std::endl;
    
    CodeGenContext context("interactive_module");
    Parser parser;
    
    while (true) {
        std::cout << ">> ";
        
        std::string line;
        if (!std::getline(std::cin, line)) {
            break; // EOF
        }
        
        if (line == "exit" || line == "quit") {
            break;
        }
        
        if (line.empty()) {
            continue;
        }
        
        // Set the input for the lexer
        Lexer::getInstance().setInput(line);
        
        // Prime the first token
        Lexer::getNextTok();
        
        // Parse and generate code
        switch (Lexer::CurTok) {
            case tok_eof:
                break;
            case ';': // ignore top-level semicolons
                Lexer::getNextTok();
                break;
            case tok_def:
                handleDefinition(parser, context);
                break;
            case tok_extern:
                handleExtern(parser, context);
                break;
            default:
                handleTopLevelExpression(parser, context);
                break;
        }
    }
    
    std::cout << "\nFull module:" << std::endl;
    context.printModule();
}

void compileFile(const std::string& filename) {
    std::string sourceCode = readFile(filename);
    if (sourceCode.empty()) {
        return;
    }
    
    std::cout << "Compiling file: " << filename << std::endl;
    std::cout << std::endl;
    
    CodeGenContext context("main_module");
    Parser parser;
    
    // Set the input for the lexer
    Lexer::getInstance().setInput(sourceCode);
    
    // Parse the entire program
    auto nodes = parser.parseProgram();
    
    // Generate code for all nodes
    for (auto& node : nodes) {
        if (auto fnNode = dynamic_cast<FunctionAST*>(node.get())) {
            if (auto fnIR = fnNode->codegenFunction(context)) {
                std::cout << "Generated function:" << std::endl;
                fnIR->print(llvm::outs());
                std::cout << std::endl;
            }
        } else if (auto protoNode = dynamic_cast<PrototypeAST*>(node.get())) {
            if (auto fnIR = protoNode->codegenFunction(context)) {
                std::cout << "Generated extern:" << std::endl;
                fnIR->print(llvm::outs());
                std::cout << std::endl;
            }
        }
    }
    
    std::cout << "\n=== Full LLVM IR Module ===" << std::endl;
    context.printModule();
    
    // Verify the module
    if (context.verifyModule()) {
        std::cout << "\n✓ Module verification passed!" << std::endl;
    } else {
        std::cout << "\n✗ Module verification failed!" << std::endl;
    }
    
    // Optimize the module
    context.optimizeModule();
    std::cout << "\n=== Optimized LLVM IR Module ===" << std::endl;
    context.printModule();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string arg = argv[1];
    
    if (arg == "-h" || arg == "--help") {
        printUsage(argv[0]);
        return 0;
    } else if (arg == "-i" || arg == "--interactive") {
        interactiveMode();
        return 0;
    } else {
        compileFile(arg);
        return 0;
    }
}
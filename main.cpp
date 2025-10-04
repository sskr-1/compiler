#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [output_file]" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argc > 2 ? argv[2] : "output.ll";
    
    // Read input file
    std::string source = readFile(inputFile);
    if (source.empty()) {
        return 1;
    }
    
    std::cout << "Compiling " << inputFile << "..." << std::endl;
    
    // Initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // Lex and parse
    Lexer lexer(source);
    Parser parser(lexer);
    
    std::cout << "Parsing..." << std::endl;
    auto program = parser.parseProgram();
    if (!program) {
        std::cerr << "Failed to parse program" << std::endl;
        return 1;
    }
    
    // Generate code
    std::cout << "Generating LLVM IR..." << std::endl;
    CodeGenContext context;
    program->codeGen(context);
    
    // Print IR to stdout
    std::cout << "\n=== Generated LLVM IR ===\n" << std::endl;
    context.printIR();
    
    // Save to file
    context.saveIRToFile(outputFile);
    std::cout << "\nIR saved to " << outputFile << std::endl;
    
    return 0;
}

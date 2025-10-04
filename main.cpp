#include <iostream>
#include <fstream>
#include <memory>
#include <cstring>
#include "ast_nodes.h"
#include "codegen.h"

// External variables from parser
extern ASTNode* g_root;
extern int yyparse();
extern FILE* yyin;
extern int yylineno;

// Global code generator
std::unique_ptr<CodeGenerator> g_codegen;

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options] <input_file>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -o <file>      Output LLVM IR to file (default: stdout)\n";
    std::cout << "  -O            Optimize the generated IR\n";
    std::cout << "  -v            Verify the generated IR\n";
    std::cout << "  -ast          Print the AST instead of generating IR\n";
    std::cout << "  --version     Show version information\n";
}

void printVersion() {
    std::cout << "CLike Compiler v1.0.0\n";
    std::cout << "LLVM IR Code Generator for C-like Language\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    bool optimize = false;
    bool verify = false;
    bool print_ast = false;
    std::string output_file;
    std::string input_file;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printVersion();
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o requires an output filename\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-O") == 0) {
            optimize = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            verify = true;
        } else if (strcmp(argv[i], "-ast") == 0) {
            print_ast = true;
        } else if (argv[i][0] == '-') {
            std::cerr << "Error: Unknown option " << argv[i] << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            if (input_file.empty()) {
                input_file = argv[i];
            } else {
                std::cerr << "Error: Multiple input files specified\n";
                return 1;
            }
        }
    }
    
    if (input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // Open input file
    FILE* input = fopen(input_file.c_str(), "r");
    if (!input) {
        std::cerr << "Error: Cannot open input file " << input_file << "\n";
        return 1;
    }
    
    // Set global input file for lexer
    yyin = input;
    
    // Initialize code generator
    g_codegen = std::make_unique<CodeGenerator>();
    
    // Parse input
    std::cout << "Parsing " << input_file << "...\n";
    int parse_result = yyparse();
    fclose(input);
    
    if (parse_result != 0) {
        std::cerr << "Error: Parsing failed\n";
        return 1;
    }
    
    if (!g_root) {
        std::cerr << "Error: No AST generated\n";
        return 1;
    }
    
    // Print AST if requested
    if (print_ast) {
        std::cout << "\nAbstract Syntax Tree:\n";
        std::cout << "===================\n";
        g_root->print();
        std::cout << "\n";
    }
    
    // Generate LLVM IR
    std::cout << "Generating LLVM IR...\n";
    llvm::Value* result = g_root->codegen(*g_codegen);
    
    if (!result) {
        std::cerr << "Error: Code generation failed\n";
        return 1;
    }
    
    // Verify IR if requested
    if (verify) {
        std::cout << "Verifying generated IR...\n";
        if (!g_codegen->verifyModule()) {
            std::cerr << "Error: IR verification failed\n";
            return 1;
        }
        std::cout << "IR verification passed\n";
    }
    
    // Optimize IR if requested
    if (optimize) {
        std::cout << "Optimizing IR...\n";
        g_codegen->optimizeIR();
    }
    
    // Output IR
    if (output_file.empty()) {
        std::cout << "\nGenerated LLVM IR:\n";
        std::cout << "=================\n";
        g_codegen->printIR();
    } else {
        std::ofstream out(output_file);
        if (!out) {
            std::cerr << "Error: Cannot open output file " << output_file << "\n";
            return 1;
        }
        
        // Redirect LLVM output to file
        llvm::raw_os_ostream out_stream(out);
        g_codegen->getModule()->print(out_stream, nullptr);
        out.close();
        
        std::cout << "LLVM IR written to " << output_file << "\n";
    }
    
    // Clean up
    delete g_root;
    
    std::cout << "Compilation completed successfully!\n";
    return 0;
}
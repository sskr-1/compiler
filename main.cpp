#include <iostream>
#include <fstream>
#include <string>
#include "ast.h"
#include "codegen.h"

// Forward declarations
extern int yyparse();
extern FILE* yyin;
extern int line_num;

// Global program variable (defined in parser.y)
extern Program* program;

void usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [options] <input_file>" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -o <output_file>    Specify output LLVM IR file (default: stdout)" << std::endl;
    std::cerr << "  -h, --help          Show this help message" << std::endl;
    std::cerr << "  -v, --version       Show version information" << std::endl;
}

void version() {
    std::cout << "LLVM IR Code Generator for C-like Language v1.0" << std::endl;
    std::cout << "Built with LLVM " << LLVM_VERSION_STRING << std::endl;
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            version();
            return 0;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o option requires an argument" << std::endl;
                usage(argv[0]);
                return 1;
            }
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option " << arg << std::endl;
            usage(argv[0]);
            return 1;
        } else {
            if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Error: Multiple input files specified" << std::endl;
                usage(argv[0]);
                return 1;
            }
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        usage(argv[0]);
        return 1;
    }

    // Open input file
    yyin = fopen(input_file.c_str(), "r");
    if (!yyin) {
        std::cerr << "Error: Could not open input file '" << input_file << "'" << std::endl;
        return 1;
    }

    // Reset line number
    line_num = 1;

    // Initialize code generator
    CodeGenerator codegen("main_module");

    // Parse the input
    std::cout << "Parsing input file: " << input_file << std::endl;
    if (yyparse() != 0) {
        std::cerr << "Parse failed" << std::endl;
        fclose(yyin);
        return 1;
    }

    fclose(yyin);

    if (!program) {
        std::cerr << "Error: No program parsed" << std::endl;
        return 1;
    }

    // Generate LLVM IR
    std::cout << "Generating LLVM IR..." << std::endl;
    llvm::Value* result = codegen.generateCode(program);

    if (!result) {
        std::cerr << "Code generation failed" << std::endl;
        return 1;
    }

    // Output LLVM IR
    if (output_file.empty()) {
        std::cout << "\nGenerated LLVM IR:" << std::endl;
        codegen.printIRToStdout();
    } else {
        codegen.printIR(output_file);
        std::cout << "LLVM IR written to: " << output_file << std::endl;
    }

    // Print AST for debugging (optional)
    std::cout << "\nAST representation:" << std::endl;
    program->print();

    // Clean up
    delete program;

    std::cout << "\nCompilation completed successfully!" << std::endl;
    return 0;
}
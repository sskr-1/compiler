#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "ir_generator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Module.h>

class CodeGenVisitor : public clike::ASTVisitor {
private:
    clike::IRGenerator& generator_;
    
public:
    CodeGenVisitor(clike::IRGenerator& generator) : generator_(generator) {}
    
    void visit(clike::NumberExpr& node) override {
        node.codegen(generator_);
    }
    
    void visit(clike::VariableExpr& node) override {
        node.codegen(generator_);
    }
    
    void visit(clike::BinaryExpr& node) override {
        node.getLeft()->accept(*this);
        node.getRight()->accept(*this);
        node.codegen(generator_);
    }
    
    void visit(clike::CallExpr& node) override {
        for (auto& arg : node.getArgs()) {
            arg->accept(*this);
        }
        node.codegen(generator_);
    }
    
    void visit(clike::ExprStmt& node) override {
        node.getExpr()->accept(*this);
        node.codegen(generator_);
    }
    
    void visit(clike::VarDeclStmt& node) override {
        if (node.getInit()) {
            node.getInit()->accept(*this);
        }
        node.codegen(generator_);
    }
    
    void visit(clike::IfStmt& node) override {
        node.getCondition()->accept(*this);
        node.getThenStmt()->accept(*this);
        if (node.getElseStmt()) {
            node.getElseStmt()->accept(*this);
        }
        node.codegen(generator_);
    }
    
    void visit(clike::WhileStmt& node) override {
        node.getCondition()->accept(*this);
        node.getBody()->accept(*this);
        node.codegen(generator_);
    }
    
    void visit(clike::ReturnStmt& node) override {
        if (node.getExpr()) {
            node.getExpr()->accept(*this);
        }
        node.codegen(generator_);
    }
    
    void visit(clike::BlockStmt& node) override {
        for (auto& stmt : node.getStatements()) {
            stmt->accept(*this);
        }
        node.codegen(generator_);
    }
    
    void visit(clike::FunctionDef& node) override {
        node.getBody()->accept(*this);
        node.codegen(generator_);
    }
    
    void visit(clike::Program& node) override {
        for (auto& func : node.getFunctions()) {
            func->accept(*this);
        }
        node.codegen(generator_);
    }
};

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

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] <input_file>\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output>    Output LLVM IR to file\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --verbose  Verbose output\n";
}

int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string outputFile;
    bool verbose = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg[0] != '-') {
            inputFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // Read input file
    std::string sourceCode = readFile(inputFile);
    if (sourceCode.empty()) {
        return 1;
    }
    
    if (verbose) {
        std::cout << "Source code:\n" << sourceCode << "\n" << std::endl;
    }
    
    try {
        // Create lexer and parser
        clike::Lexer lexer(sourceCode);
        clike::Parser parser(lexer);
        
        // Parse the program
        auto program = parser.parseProgram();
        if (!program) {
            std::cerr << "Error: Failed to parse program" << std::endl;
            return 1;
        }
        
        if (verbose) {
            std::cout << "Parsing successful!" << std::endl;
        }
        
        // Create IR generator and generate LLVM IR
        clike::IRGenerator generator;
        llvm::Module* module = generator.codegen(*program);
        
        if (!module) {
            std::cerr << "Error: Failed to generate LLVM IR" << std::endl;
            return 1;
        }
        
        // Verify the module
        if (!generator.verifyModule()) {
            std::cerr << "Error: Generated LLVM IR is invalid" << std::endl;
            return 1;
        }
        
        if (verbose) {
            std::cout << "LLVM IR generation successful!" << std::endl;
        }
        
        // Output the LLVM IR
        if (outputFile.empty()) {
            generator.printModule();
        } else {
            std::error_code error;
            llvm::raw_fd_ostream output(outputFile, error);
            if (error) {
                std::cerr << "Error: Could not open output file " << outputFile << ": " 
                          << error.message() << std::endl;
                return 1;
            }
            generator.printModule(output);
            std::cout << "LLVM IR written to " << outputFile << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
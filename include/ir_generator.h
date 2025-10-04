#pragma once

#include <memory>
#include <string>
#include <map>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

// Forward declarations
namespace clike {
    class NumberExpr;
    class VariableExpr;
    class BinaryExpr;
    class CallExpr;
    class ExprStmt;
    class VarDeclStmt;
    class IfStmt;
    class WhileStmt;
    class ReturnStmt;
    class BlockStmt;
    class FunctionDef;
    class Program;
}

namespace clike {

class IRGenerator {
public:
    IRGenerator();
    ~IRGenerator();

    // Module management
    llvm::Module* createModule(const std::string& name);
    void printModule(llvm::raw_ostream& os = llvm::outs());
    bool verifyModule();
    
    // Code generation for expressions
    llvm::Value* codegen(NumberExpr& expr);
    llvm::Value* codegen(VariableExpr& expr);
    llvm::Value* codegen(BinaryExpr& expr);
    llvm::Value* codegen(CallExpr& expr);
    
    // Code generation for statements
    llvm::Value* codegen(ExprStmt& stmt);
    llvm::Value* codegen(VarDeclStmt& stmt);
    llvm::Value* codegen(IfStmt& stmt);
    llvm::Value* codegen(WhileStmt& stmt);
    llvm::Value* codegen(ReturnStmt& stmt);
    llvm::Value* codegen(BlockStmt& stmt);
    
    // Code generation for functions and programs
    llvm::Function* codegen(FunctionDef& func);
    llvm::Module* codegen(Program& program);
    
    // Utility functions
    llvm::Type* getDoubleType();
    llvm::Function* getFunction(const std::string& name);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, const std::string& varName);
    
    // Symbol table management
    void enterScope();
    void exitScope();
    void addSymbol(const std::string& name, llvm::Value* value);
    llvm::Value* getSymbol(const std::string& name);

private:
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::Module> module_;
    
    // Symbol table (scoped)
    std::vector<std::map<std::string, llvm::Value*>> symbolTable_;
    
    // Helper functions
    llvm::Value* createBinaryOperation(char op, llvm::Value* left, llvm::Value* right);
    llvm::Function* createMainFunction();
    void setupBuiltinFunctions();
};

} // namespace clike
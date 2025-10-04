#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <map>
#include <stack>
#include <memory>
#include "ast.h"

// LLVM includes
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>

class CodeGenerator {
private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;

    // Symbol table for variables and functions
    std::map<std::string, llvm::Value*> named_values;
    std::map<std::string, llvm::Function*> function_decls;

    // Loop context for break/continue (simplified for this example)
    std::stack<llvm::BasicBlock*> loop_blocks;

public:
    CodeGenerator(const std::string& module_name = "main_module");

    // Get LLVM types
    llvm::Type* getLLVMType(Type* type);
    llvm::Type* getLLVMType(const std::string& type_name);

    // Symbol table management
    void setNamedValue(const std::string& name, llvm::Value* value);
    llvm::Value* getNamedValue(const std::string& name);
    void setFunction(const std::string& name, llvm::Function* func);
    llvm::Function* getFunction(const std::string& name);

    // Code generation methods for each AST node type
    llvm::Value* generateCode(Expression* expr);
    llvm::Value* generateCode(Statement* stmt);
    llvm::Value* generateCode(Declaration* decl);
    llvm::Value* generateCode(Program* program);

    // Utility methods
    llvm::BasicBlock* getCurrentBasicBlock() { return builder.GetInsertBlock(); }
    llvm::Function* getCurrentFunction() { return builder.GetInsertBlock()->getParent(); }

    // Loop management
    void pushLoopBlock(llvm::BasicBlock* block) { loop_blocks.push(block); }
    void popLoopBlock() { loop_blocks.pop(); }
    llvm::BasicBlock* getCurrentLoopBlock() {
        return loop_blocks.empty() ? nullptr : loop_blocks.top();
    }

    // Output LLVM IR to file or stdout
    void printIR(const std::string& filename = "");
    void printIRToStdout();

    // Error handling
    llvm::Value* logError(const std::string& str);
    llvm::Function* logErrorF(const std::string& str);

    // Access to LLVM components
    llvm::LLVMContext& getContext() { return context; }
    llvm::IRBuilder<>& getBuilder() { return builder; }
    llvm::Module* getModule() { return module.get(); }
};

#endif // CODEGEN_H
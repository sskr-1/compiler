#pragma once

#include "ast.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <string>
#include <memory>

class CodeGenContext {
private:
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::map<std::string, llvm::Value*> NamedValues;
    std::map<std::string, llvm::AllocaInst*> NamedAllocas;

public:
    CodeGenContext(const std::string& moduleName);
    
    // Getters for LLVM objects
    llvm::LLVMContext& getContext() { return *TheContext; }
    llvm::IRBuilder<>& getBuilder() { return *Builder; }
    llvm::Module& getModule() { return *TheModule; }
    
    // Variable management
    llvm::Value* getNamedValue(const std::string& name);
    void setNamedValue(const std::string& name, llvm::Value* value);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, 
                                            const std::string& varName,
                                            llvm::Type* type);
    
    // Type utilities
    llvm::Type* getTypeFromString(const std::string& typeName);
    
    // Module operations
    void printModule();
    bool verifyModule();
    void optimizeModule();
    
    // Scope management
    void pushScope();
    void popScope();

private:
    std::vector<std::map<std::string, llvm::Value*>> scopeStack;
};
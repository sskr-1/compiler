#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <map>
#include <string>
#include <memory>
#include <iostream>

class CodeGenContext {
public:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::AllocaInst*> namedValues;
    llvm::Function* currentFunction;
    
    CodeGenContext() : builder(context) {
        module = std::make_unique<llvm::Module>("main", context);
        currentFunction = nullptr;
    }
    
    llvm::Type* getType(const std::string& typeStr) {
        if (typeStr == "int") {
            return llvm::Type::getInt32Ty(context);
        } else if (typeStr == "float") {
            return llvm::Type::getFloatTy(context);
        } else if (typeStr == "double") {
            return llvm::Type::getDoubleTy(context);
        } else if (typeStr == "void") {
            return llvm::Type::getVoidTy(context);
        }
        return llvm::Type::getInt32Ty(context); // default to int
    }
    
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function,
                                             const std::string& varName,
                                             llvm::Type* type) {
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(),
                                     function->getEntryBlock().begin());
        return tmpBuilder.CreateAlloca(type, nullptr, varName);
    }
    
    void printIR() {
        module->print(llvm::outs(), nullptr);
    }
    
    void saveIRToFile(const std::string& filename) {
        std::error_code EC;
        llvm::raw_fd_ostream file(filename, EC, llvm::sys::fs::OF_None);
        if (EC) {
            std::cerr << "Error opening file: " << EC.message() << std::endl;
            return;
        }
        module->print(file, nullptr);
    }
};

#endif // CODEGEN_H

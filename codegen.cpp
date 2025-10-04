#include "codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/IR/LegacyPassManager.h>

CodeGenContext::CodeGenContext(const std::string& moduleName) {
    TheContext = std::make_unique<llvm::LLVMContext>();
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
    TheModule = std::make_unique<llvm::Module>(moduleName, *TheContext);
}

llvm::Value* CodeGenContext::getNamedValue(const std::string& name) {
    auto it = NamedValues.find(name);
    if (it != NamedValues.end()) {
        return it->second;
    }
    return nullptr;
}

void CodeGenContext::setNamedValue(const std::string& name, llvm::Value* value) {
    NamedValues[name] = value;
}

llvm::AllocaInst* CodeGenContext::createEntryBlockAlloca(llvm::Function* function,
                                                        const std::string& varName,
                                                        llvm::Type* type) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, varName);
}

llvm::Type* CodeGenContext::getTypeFromString(const std::string& typeName) {
    if (typeName == "double" || typeName == "float") {
        return llvm::Type::getDoubleTy(*TheContext);
    } else if (typeName == "int") {
        return llvm::Type::getInt32Ty(*TheContext);
    } else if (typeName == "bool") {
        return llvm::Type::getInt1Ty(*TheContext);
    } else if (typeName == "void") {
        return llvm::Type::getVoidTy(*TheContext);
    }
    // Default to double for unknown types
    return llvm::Type::getDoubleTy(*TheContext);
}

void CodeGenContext::printModule() {
    TheModule->print(llvm::outs(), nullptr);
}

bool CodeGenContext::verifyModule() {
    std::string errorStr;
    llvm::raw_string_ostream errorStream(errorStr);
    if (llvm::verifyModule(*TheModule, &errorStream)) {
        llvm::errs() << "Module verification failed:\n" << errorStr << "\n";
        return false;
    }
    return true;
}

void CodeGenContext::optimizeModule() {
    // Create a pass manager and optimization passes
    auto passManager = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());
    
    // Add some basic optimization passes
    passManager->add(llvm::createInstructionCombiningPass());
    passManager->add(llvm::createReassociatePass());
    passManager->add(llvm::createGVNPass());
    passManager->add(llvm::createCFGSimplificationPass());
    
    passManager->doInitialization();
    
    // Run the optimizer on all functions
    for (auto& function : *TheModule) {
        passManager->run(function);
    }
}

void CodeGenContext::pushScope() {
    scopeStack.push_back(NamedValues);
}

void CodeGenContext::popScope() {
    if (!scopeStack.empty()) {
        NamedValues = scopeStack.back();
        scopeStack.pop_back();
    }
}
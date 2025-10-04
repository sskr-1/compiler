#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include "AST.h"

namespace mini {

struct CodeGenContext {
  std::unique_ptr<llvm::LLVMContext> llvmContext;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

  // Stack of local scopes; each maps variable name to its allocation
  std::vector<std::unordered_map<std::string, llvm::AllocaInst*>> localScopes;

  explicit CodeGenContext(const std::string& moduleName)
      : llvmContext(std::make_unique<llvm::LLVMContext>()),
        module(std::make_unique<llvm::Module>(moduleName, *llvmContext)),
        builder(std::make_unique<llvm::IRBuilder<>>(*llvmContext)) {
    pushScope();
  }

  void pushScope() { localScopes.emplace_back(); }

  void popScope() {
    if (!localScopes.empty()) localScopes.pop_back();
  }

  llvm::AllocaInst* getVariableAlloca(const std::string& name) {
    for (auto scopeIt = localScopes.rbegin(); scopeIt != localScopes.rend(); ++scopeIt) {
      auto found = scopeIt->find(name);
      if (found != scopeIt->end()) return found->second;
    }
    return nullptr;
  }

  void setVariableAlloca(const std::string& name, llvm::AllocaInst* alloca) {
    if (localScopes.empty()) pushScope();
    localScopes.back()[name] = alloca;
  }

  llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function,
                                           const std::string& variableName,
                                           llvm::Type* variableType) {
    llvm::IRBuilder<> tempBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tempBuilder.CreateAlloca(variableType, nullptr, variableName);
  }

  llvm::Type* getI32Type() { return llvm::Type::getInt32Ty(*llvmContext); }
  llvm::Type* getI1Type() { return llvm::Type::getInt1Ty(*llvmContext); }

  llvm::Value* ensureI32(llvm::Value* value) {
    if (value->getType()->isIntegerTy(32)) return value;
    if (value->getType()->isIntegerTy(1)) {
      return builder->CreateZExt(value, getI32Type(), "zext_bool_to_i32");
    }
    return value;  // Fallback; callers should ensure correct typing
  }

  llvm::Value* toBoolean(llvm::Value* value) {
    if (value->getType()->isIntegerTy(1)) return value;
    if (value->getType()->isIntegerTy(32)) {
      auto zero = llvm::ConstantInt::get(getI32Type(), 0, true);
      return builder->CreateICmpNE(value, zero, "cmp_ne_zero");
    }
    return value;  // Fallback
  }
};

}  // namespace mini

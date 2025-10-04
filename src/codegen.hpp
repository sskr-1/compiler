#pragma once

#include "ast.hpp"
#include <map>
#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

class CodeGen {
public:
  explicit CodeGen(const std::string &moduleName);

  std::string emitIR(const TranslationUnit &tu);

private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

  // Map variable name to its stack allocation (alloca)
  std::map<std::string, llvm::AllocaInst*> namedAllocas;

  llvm::Type *getIntType();
  llvm::Function *getFunction(const std::string &name);

  // Declaration helpers
  llvm::Function *declareExtern(const ExternDecl &decl);
  llvm::Function *declareFunction(const ::Function &fn);

  // Codegen helpers
  llvm::Value *codegenExpr(const Expr &expr);
  llvm::Value *codegenBinary(const BinaryExpr &bin);
  llvm::Value *codegenUnary(const UnaryExpr &un);
  llvm::Value *codegenCall(const CallExpr &call);
  llvm::Value *codegenAssign(const AssignExpr &as);

  void codegenStmt(const Stmt &stmt, llvm::Function *currentFunction);
  void codegenVarDecl(const VarDeclStmt &vd);
  void codegenIf(const IfStmt &ifs, llvm::Function *currentFunction);
  void codegenWhile(const WhileStmt &ws, llvm::Function *currentFunction);
};

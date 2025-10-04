#include "codegen.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <sstream>
#include <stdexcept>

using namespace llvm;

CodeGen::CodeGen(const std::string &moduleName) {
  context = std::make_unique<LLVMContext>();
  module = std::make_unique<Module>(moduleName, *context);
  builder = std::make_unique<IRBuilder<>>(*context);
}

Type *CodeGen::getIntType() { return Type::getInt32Ty(*context); }

llvm::Function *CodeGen::getFunction(const std::string &name) {
  if (auto *f = module->getFunction(name)) return f;
  return nullptr;
}

llvm::Function *CodeGen::declareExtern(const ExternDecl &decl) {
  std::vector<Type*> paramTys(decl.params.size(), getIntType());
  FunctionType *ft = FunctionType::get(getIntType(), paramTys, false);
  llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, decl.name, module.get());
  unsigned idx = 0;
  for (auto &arg : f->args()) { arg.setName(decl.params[idx++].name); }
  return f;
}

llvm::Function *CodeGen::declareFunction(const ::Function &fn) {
  std::vector<Type*> paramTys(fn.params.size(), getIntType());
  FunctionType *ft = FunctionType::get(getIntType(), paramTys, false);
  llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, fn.name, module.get());
  unsigned idx = 0;
  for (auto &arg : f->args()) { arg.setName(fn.params[idx++].name); }
  return f;
}

static AllocaInst *createEntryBlockAlloca(llvm::Function *f, const std::string &name, Type *ty, IRBuilder<> &builder) {
  IRBuilder<> tmp(&f->getEntryBlock(), f->getEntryBlock().begin());
  return tmp.CreateAlloca(ty, nullptr, name);
}

Value *CodeGen::codegenExpr(const Expr &expr) {
  if (auto *n = dynamic_cast<const NumberExpr*>(&expr)) {
    return ConstantInt::get(getIntType(), n->value, true);
  }
  if (auto *v = dynamic_cast<const VariableExpr*>(&expr)) {
    auto it = namedAllocas.find(v->name);
    if (it == namedAllocas.end()) throw std::runtime_error("Unknown variable: " + v->name);
    return builder->CreateLoad(getIntType(), it->second, v->name.c_str());
  }
  if (auto *b = dynamic_cast<const BinaryExpr*>(&expr)) {
    return codegenBinary(*b);
  }
  if (auto *u = dynamic_cast<const UnaryExpr*>(&expr)) {
    return codegenUnary(*u);
  }
  if (auto *c = dynamic_cast<const CallExpr*>(&expr)) {
    return codegenCall(*c);
  }
  if (auto *a = dynamic_cast<const AssignExpr*>(&expr)) {
    return codegenAssign(*a);
  }
  throw std::runtime_error("Unhandled expression");
}

Value *CodeGen::codegenBinary(const BinaryExpr &bin) {
  Value *L = codegenExpr(*bin.lhs);
  Value *R = codegenExpr(*bin.rhs);
  if (bin.op == "+") return builder->CreateAdd(L, R, "addtmp");
  if (bin.op == "-") return builder->CreateSub(L, R, "subtmp");
  if (bin.op == "*") return builder->CreateMul(L, R, "multmp");
  if (bin.op == "/") return builder->CreateSDiv(L, R, "divtmp");
  if (bin.op == "%") return builder->CreateSRem(L, R, "remtmp");
  if (bin.op == "==") return builder->CreateZExt(builder->CreateICmpEQ(L, R), getIntType(), "eqtmp");
  if (bin.op == "!=") return builder->CreateZExt(builder->CreateICmpNE(L, R), getIntType(), "netmp");
  if (bin.op == "<") return builder->CreateZExt(builder->CreateICmpSLT(L, R), getIntType(), "lttmp");
  if (bin.op == "<=") return builder->CreateZExt(builder->CreateICmpSLE(L, R), getIntType(), "letmp");
  if (bin.op == ">") return builder->CreateZExt(builder->CreateICmpSGT(L, R), getIntType(), "gttmp");
  if (bin.op == ">=") return builder->CreateZExt(builder->CreateICmpSGE(L, R), getIntType(), "getmp");
  if (bin.op == "&&") {
    // simple non-short-circuit: (L != 0) & (R != 0)
    Value *lne = builder->CreateICmpNE(L, ConstantInt::get(getIntType(), 0));
    Value *rne = builder->CreateICmpNE(R, ConstantInt::get(getIntType(), 0));
    Value *andv = builder->CreateAnd(lne, rne);
    return builder->CreateZExt(andv, getIntType(), "andtmp");
  }
  if (bin.op == "||") {
    Value *lne = builder->CreateICmpNE(L, ConstantInt::get(getIntType(), 0));
    Value *rne = builder->CreateICmpNE(R, ConstantInt::get(getIntType(), 0));
    Value *orv = builder->CreateOr(lne, rne);
    return builder->CreateZExt(orv, getIntType(), "ortmp");
  }
  throw std::runtime_error("Unknown binary operator: " + bin.op);
}

Value *CodeGen::codegenUnary(const UnaryExpr &un) {
  Value *V = codegenExpr(*un.operand);
  if (un.op == "-") return builder->CreateNeg(V, "negtmp");
  if (un.op == "+") return V;
  if (un.op == "!") {
    Value *isZero = builder->CreateICmpEQ(V, ConstantInt::get(getIntType(), 0));
    return builder->CreateZExt(isZero, getIntType(), "nottmp");
  }
  throw std::runtime_error("Unknown unary operator: " + un.op);
}

Value *CodeGen::codegenCall(const CallExpr &call) {
  llvm::Function *callee = getFunction(call.callee);
  if (!callee) throw std::runtime_error("Unknown function referenced: " + call.callee);
  std::vector<Value*> argsV;
  for (auto &a : call.args) argsV.push_back(codegenExpr(*a));
  return builder->CreateCall(callee, argsV, call.callee == "printf" ? "" : "calltmp");
}

Value *CodeGen::codegenAssign(const AssignExpr &as) {
  auto it = namedAllocas.find(as.name);
  if (it == namedAllocas.end()) throw std::runtime_error("Unknown variable: " + as.name);
  Value *val = codegenExpr(*as.value);
  builder->CreateStore(val, it->second);
  return val;
}

void CodeGen::codegenVarDecl(const VarDeclStmt &vd) {
  llvm::Function *func = builder->GetInsertBlock()->getParent();
  AllocaInst *alloca = createEntryBlockAlloca(func, vd.name, getIntType(), *builder);
  namedAllocas[vd.name] = alloca;
  if (vd.init) {
    Value *initV = codegenExpr(*vd.init);
    builder->CreateStore(initV, alloca);
  }
}

void CodeGen::codegenIf(const IfStmt &ifs, llvm::Function *currentFunction) {
  Value *condV = codegenExpr(*ifs.cond);
  condV = builder->CreateICmpNE(condV, ConstantInt::get(getIntType(), 0), "ifcond");

  BasicBlock *thenBB = BasicBlock::Create(*context, "then", currentFunction);
  BasicBlock *elseBB = BasicBlock::Create(*context, "else");
  BasicBlock *mergeBB = BasicBlock::Create(*context, "ifcont");

  builder->CreateCondBr(condV, thenBB, elseBB);

  builder->SetInsertPoint(thenBB);
  for (auto &s : ifs.thenStmts) codegenStmt(*s, currentFunction);
  if (!thenBB->getTerminator()) builder->CreateBr(mergeBB);

  currentFunction->insert(currentFunction->end(), elseBB);
  builder->SetInsertPoint(elseBB);
  for (auto &s : ifs.elseStmts) codegenStmt(*s, currentFunction);
  if (!elseBB->getTerminator()) builder->CreateBr(mergeBB);

  currentFunction->insert(currentFunction->end(), mergeBB);
  builder->SetInsertPoint(mergeBB);
}

void CodeGen::codegenWhile(const WhileStmt &ws, llvm::Function *currentFunction) {
  BasicBlock *condBB = BasicBlock::Create(*context, "loop.cond", currentFunction);
  BasicBlock *bodyBB = BasicBlock::Create(*context, "loop.body");
  BasicBlock *afterBB = BasicBlock::Create(*context, "loop.after");

  builder->CreateBr(condBB);

  builder->SetInsertPoint(condBB);
  Value *condV = codegenExpr(*ws.cond);
  condV = builder->CreateICmpNE(condV, ConstantInt::get(getIntType(), 0), "loopcond");
  builder->CreateCondBr(condV, bodyBB, afterBB);

  currentFunction->insert(currentFunction->end(), bodyBB);
  builder->SetInsertPoint(bodyBB);
  for (auto &s : ws.body) codegenStmt(*s, currentFunction);
  if (!bodyBB->getTerminator()) builder->CreateBr(condBB);

  currentFunction->insert(currentFunction->end(), afterBB);
  builder->SetInsertPoint(afterBB);
}

void CodeGen::codegenStmt(const Stmt &stmt, llvm::Function *currentFunction) {
  if (auto *r = dynamic_cast<const ReturnStmt*>(&stmt)) {
    Value *retV = codegenExpr(*r->value);
    builder->CreateRet(retV);
    return;
  }
  if (auto *e = dynamic_cast<const ExprStmt*>(&stmt)) {
    (void)codegenExpr(*e->expr);
    return;
  }
  if (auto *v = dynamic_cast<const VarDeclStmt*>(&stmt)) { codegenVarDecl(*v); return; }
  if (auto *i = dynamic_cast<const IfStmt*>(&stmt)) { codegenIf(*i, currentFunction); return; }
  if (auto *w = dynamic_cast<const WhileStmt*>(&stmt)) { codegenWhile(*w, currentFunction); return; }
  throw std::runtime_error("Unhandled statement");
}

std::string CodeGen::emitIR(const TranslationUnit &tu) {
  // First, declare externs and functions
  for (auto &e : tu.externs) declareExtern(*e);
  for (auto &f : tu.functions) declareFunction(*f);

  for (auto &fup : tu.functions) {
    llvm::Function *f = module->getFunction(fup->name);
    if (!f) throw std::runtime_error("Function not found after declaration");

    BasicBlock *entry = BasicBlock::Create(*context, "entry", f);
    builder->SetInsertPoint(entry);

    namedAllocas.clear();
    // Create allocas for params and store incoming values
    unsigned idx = 0;
    for (auto &arg : f->args()) {
      auto alloca = createEntryBlockAlloca(f, std::string(arg.getName()), getIntType(), *builder);
      namedAllocas[std::string(arg.getName())] = alloca;
      builder->CreateStore(&arg, alloca);
      idx++;
    }

    for (auto &s : fup->body) {
      codegenStmt(*s, f);
    }

    // Ensure function is properly terminated
    if (!entry->getTerminator()) {
      // Default return 0 if no explicit return
      builder->CreateRet(ConstantInt::get(getIntType(), 0));
    }

    if (verifyFunction(*f, &errs())) {
      throw std::runtime_error("Generated invalid function IR");
    }
  }

  if (verifyModule(*module, &errs())) {
    throw std::runtime_error("Generated invalid module IR");
  }

  std::string ir;
  raw_string_ostream os(ir);
  module->print(os, nullptr);
  os.flush();
  return ir;
}

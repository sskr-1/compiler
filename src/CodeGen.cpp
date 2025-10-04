#include "CodeGen.h"

#include <stdexcept>
#include <utility>

using namespace llvm;

namespace mini {

// ===== Expressions =====

Value* NumberExpr::codegen(CodeGenContext& context) {
  return ConstantInt::get(context.getI32Type(), static_cast<std::int64_t>(value), true);
}

Value* VariableExpr::codegen(CodeGenContext& context) {
  AllocaInst* alloca = context.getVariableAlloca(name);
  if (!alloca) {
    throw std::runtime_error("Undefined variable: " + name);
  }
  return context.builder->CreateLoad(context.getI32Type(), alloca, name.c_str());
}

Value* AssignExpr::codegen(CodeGenContext& context) {
  AllocaInst* alloca = context.getVariableAlloca(variableName);
  if (!alloca) {
    throw std::runtime_error("Assignment to undefined variable: " + variableName);
  }
  Value* rhsValue = value->codegen(context);
  rhsValue = context.ensureI32(rhsValue);
  context.builder->CreateStore(rhsValue, alloca);
  return rhsValue;
}

static Value* emitComparison(IRBuilder<>& builder, CodeGenContext& context, BinaryOp op, Value* lhs, Value* rhs) {
  Value* cmp = nullptr;
  switch (op) {
    case BinaryOp::LT: cmp = builder.CreateICmpSLT(lhs, rhs, "cmplt"); break;
    case BinaryOp::GT: cmp = builder.CreateICmpSGT(lhs, rhs, "cmpgt"); break;
    case BinaryOp::LE: cmp = builder.CreateICmpSLE(lhs, rhs, "cmple"); break;
    case BinaryOp::GE: cmp = builder.CreateICmpSGE(lhs, rhs, "cmpge"); break;
    case BinaryOp::EQ: cmp = builder.CreateICmpEQ(lhs, rhs, "cmpeq"); break;
    case BinaryOp::NE: cmp = builder.CreateICmpNE(lhs, rhs, "cmpne"); break;
    default: break;
  }
  if (!cmp) return nullptr;
  return builder.CreateZExt(cmp, context.getI32Type(), "bool2i32");
}

Value* BinaryExpr::codegen(CodeGenContext& context) {
  Value* lhsValue = lhs->codegen(context);
  Value* rhsValue = rhs->codegen(context);
  lhsValue = context.ensureI32(lhsValue);
  rhsValue = context.ensureI32(rhsValue);

  switch (op) {
    case BinaryOp::Add:
      return context.builder->CreateAdd(lhsValue, rhsValue, "addtmp");
    case BinaryOp::Sub:
      return context.builder->CreateSub(lhsValue, rhsValue, "subtmp");
    case BinaryOp::Mul:
      return context.builder->CreateMul(lhsValue, rhsValue, "multmp");
    case BinaryOp::Div:
      return context.builder->CreateSDiv(lhsValue, rhsValue, "divtmp");
    case BinaryOp::LT:
    case BinaryOp::GT:
    case BinaryOp::LE:
    case BinaryOp::GE:
    case BinaryOp::EQ:
    case BinaryOp::NE:
      if (Value* v = emitComparison(*context.builder, context, op, lhsValue, rhsValue)) return v;
      break;
  }
  throw std::runtime_error("Unsupported binary operator");
}

Value* CallExpr::codegen(CodeGenContext& context) {
  Function* calleeFunction = context.module->getFunction(callee);
  if (!calleeFunction) {
    throw std::runtime_error("Unknown function: " + callee);
  }
  if (calleeFunction->arg_size() != args.size()) {
    throw std::runtime_error("Function argument count mismatch for: " + callee);
  }

  std::vector<Value*> argValues;
  argValues.reserve(args.size());
  for (auto& arg : args) {
    Value* value = arg->codegen(context);
    value = context.ensureI32(value);
    argValues.push_back(value);
  }

  return context.builder->CreateCall(calleeFunction, argValues, callee == "main" ? "" : "calltmp");
}

// ===== Statements =====

void ExprStmt::codegen(CodeGenContext& context) {
  (void)expr->codegen(context);  // result ignored
}

void ReturnStmt::codegen(CodeGenContext& context) {
  Function* currentFunction = context.builder->GetInsertBlock()->getParent();
  if (value) {
    Value* v = value->codegen(context);
    v = context.ensureI32(v);
    context.builder->CreateRet(v);
  } else {
    // Default return 0 for i32
    if (currentFunction->getReturnType()->isIntegerTy(32)) {
      context.builder->CreateRet(ConstantInt::get(context.getI32Type(), 0, true));
    } else if (currentFunction->getReturnType()->isVoidTy()) {
      context.builder->CreateRetVoid();
    } else {
      throw std::runtime_error("Unsupported return type");
    }
  }
}

void VarDeclStmt::codegen(CodeGenContext& context) {
  Function* currentFunction = context.builder->GetInsertBlock()->getParent();
  AllocaInst* alloca = context.createEntryBlockAlloca(currentFunction, name, context.getI32Type());
  context.setVariableAlloca(name, alloca);

  Value* initValue = nullptr;
  if (init) {
    initValue = init->codegen(context);
    initValue = context.ensureI32(initValue);
  } else {
    initValue = ConstantInt::get(context.getI32Type(), 0, true);
  }
  context.builder->CreateStore(initValue, alloca);
}

void BlockStmt::codegen(CodeGenContext& context) {
  context.pushScope();
  for (auto& stmt : statements) {
    // Do not emit into a block that already has a terminator
    BasicBlock* currentBlock = context.builder->GetInsertBlock();
    if (currentBlock && currentBlock->getTerminator()) break;
    stmt->codegen(context);
  }
  context.popScope();
}

void IfStmt::codegen(CodeGenContext& context) {
  Value* condValue = condition->codegen(context);
  condValue = context.toBoolean(condValue);

  Function* function = context.builder->GetInsertBlock()->getParent();

  BasicBlock* thenBB = BasicBlock::Create(*context.llvmContext, "then", function);
  BasicBlock* elseBB = BasicBlock::Create(*context.llvmContext, "else");
  BasicBlock* mergeBB = BasicBlock::Create(*context.llvmContext, "ifend");

  context.builder->CreateCondBr(condValue, thenBB, elseBlock ? elseBB : mergeBB);

  // Then block
  context.builder->SetInsertPoint(thenBB);
  if (thenBlock) thenBlock->codegen(context);
  if (!thenBB->getTerminator()) {
    context.builder->CreateBr(mergeBB);
  }

  // Else block
  if (elseBlock) {
    function->insert(function->end(), elseBB);
    context.builder->SetInsertPoint(elseBB);
    elseBlock->codegen(context);
    if (!elseBB->getTerminator()) {
      context.builder->CreateBr(mergeBB);
    }
  }

  function->insert(function->end(), mergeBB);
  context.builder->SetInsertPoint(mergeBB);
}

void WhileStmt::codegen(CodeGenContext& context) {
  Function* function = context.builder->GetInsertBlock()->getParent();

  BasicBlock* condBB = BasicBlock::Create(*context.llvmContext, "while.cond", function);
  BasicBlock* bodyBB = BasicBlock::Create(*context.llvmContext, "while.body");
  BasicBlock* exitBB = BasicBlock::Create(*context.llvmContext, "while.end");

  context.builder->CreateBr(condBB);

  // Condition
  context.builder->SetInsertPoint(condBB);
  Value* condValue = context.toBoolean(condition->codegen(context));
  context.builder->CreateCondBr(condValue, bodyBB, exitBB);

  // Body
  function->insert(function->end(), bodyBB);
  context.builder->SetInsertPoint(bodyBB);
  body->codegen(context);
  if (!bodyBB->getTerminator()) {
    context.builder->CreateBr(condBB);
  }

  // Exit
  function->insert(function->end(), exitBB);
  context.builder->SetInsertPoint(exitBB);
}

// ===== Functions =====

void FunctionAST::codegen(CodeGenContext& context) {
  // Build function type: i32 (i32, i32, ...)
  std::vector<Type*> argTypes(proto.argNames.size(), context.getI32Type());
  FunctionType* functionType = FunctionType::get(context.getI32Type(), argTypes, false);

  Function* function = Function::Create(functionType, Function::ExternalLinkage, proto.name, context.module.get());

  // Name the arguments
  unsigned idx = 0;
  for (auto& arg : function->args()) {
    arg.setName(proto.argNames[idx++]);
  }

  BasicBlock* entryBB = BasicBlock::Create(*context.llvmContext, "entry", function);
  context.builder->SetInsertPoint(entryBB);

  context.pushScope();
  // Allocate and store arguments into allocas
  for (auto& arg : function->args()) {
    AllocaInst* alloca = context.createEntryBlockAlloca(function, std::string(arg.getName()), context.getI32Type());
    context.builder->CreateStore(&arg, alloca);
    context.setVariableAlloca(std::string(arg.getName()), alloca);
  }

  // Emit body
  if (body) body->codegen(context);

  // Ensure function ends with a return
  if (!entryBB->getParent()->back().getTerminator()) {
    context.builder->CreateRet(ConstantInt::get(context.getI32Type(), 0, true));
  }

  context.popScope();

  // Validate the generated code
  if (verifyFunction(*function, &llvm::errs())) {
    throw std::runtime_error("Function verification failed for: " + proto.name);
  }
}

}  // namespace mini

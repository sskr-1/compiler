#include "ast.h"
#include "codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <iostream>

// NumberExprAST implementation
llvm::Value* NumberExprAST::codegen(CodeGenContext& context) {
    return llvm::ConstantFP::get(context.getContext(), llvm::APFloat(Val));
}

// VariableExprAST implementation  
llvm::Value* VariableExprAST::codegen(CodeGenContext& context) {
    llvm::Value* V = context.getNamedValue(Name);
    if (!V) {
        std::cerr << "Unknown variable name: " << Name << std::endl;
        return nullptr;
    }
    
    // If it's an alloca instruction, load the value
    if (llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(V)) {
        return context.getBuilder().CreateLoad(alloca->getAllocatedType(), alloca, Name.c_str());
    }
    
    return V;
}

// BinaryExprAST implementation
llvm::Value* BinaryExprAST::codegen(CodeGenContext& context) {
    llvm::Value* L = LHS->codegen(context);
    llvm::Value* R = RHS->codegen(context);
    if (!L || !R) return nullptr;
    
    switch (Op) {
        case '+': return context.getBuilder().CreateFAdd(L, R, "addtmp");
        case '-': return context.getBuilder().CreateFSub(L, R, "subtmp");
        case '*': return context.getBuilder().CreateFMul(L, R, "multmp");
        case '/': return context.getBuilder().CreateFDiv(L, R, "divtmp");
        case '<': {
            L = context.getBuilder().CreateFCmpULT(L, R, "cmptmp");
            return context.getBuilder().CreateUIToFP(L, llvm::Type::getDoubleTy(context.getContext()), "booltmp");
        }
        case '>': {
            L = context.getBuilder().CreateFCmpUGT(L, R, "cmptmp");
            return context.getBuilder().CreateUIToFP(L, llvm::Type::getDoubleTy(context.getContext()), "booltmp");
        }
        case '=': {
            L = context.getBuilder().CreateFCmpUEQ(L, R, "cmptmp");
            return context.getBuilder().CreateUIToFP(L, llvm::Type::getDoubleTy(context.getContext()), "booltmp");
        }
        default:
            std::cerr << "Invalid binary operator: " << Op << std::endl;
            return nullptr;
    }
}

// CallExprAST implementation
llvm::Value* CallExprAST::codegen(CodeGenContext& context) {
    llvm::Function* CalleeF = context.getModule().getFunction(Callee);
    if (!CalleeF) {
        std::cerr << "Unknown function referenced: " << Callee << std::endl;
        return nullptr;
    }
    
    if (CalleeF->arg_size() != Args.size()) {
        std::cerr << "Incorrect # arguments passed to function " << Callee << std::endl;
        return nullptr;
    }
    
    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen(context));
        if (!ArgsV.back()) return nullptr;
    }
    
    return context.getBuilder().CreateCall(CalleeF, ArgsV, "calltmp");
}

// PrototypeAST implementation
llvm::Function* PrototypeAST::codegenFunction(CodeGenContext& context) {
    std::vector<llvm::Type*> ArgTypes(Args.size(), 
                                     context.getTypeFromString("double"));
    
    llvm::Type* RetType = context.getTypeFromString(ReturnType);
    llvm::FunctionType* FT = llvm::FunctionType::get(RetType, ArgTypes, false);
    
    llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, &context.getModule());
    
    unsigned Idx = 0;
    for (auto& Arg : F->args()) {
        Arg.setName(Args[Idx++]);
    }
    
    return F;
}

// PrototypeAST codegen implementation for virtual function
llvm::Value* PrototypeAST::codegen(CodeGenContext& context) {
    return codegenFunction(context);
}

// VarDeclAST implementation
llvm::Value* VarDeclAST::codegen(CodeGenContext& context) {
    llvm::Function* function = context.getBuilder().GetInsertBlock()->getParent();
    llvm::Type* varType = context.getTypeFromString(Type);
    
    llvm::AllocaInst* alloca = context.createEntryBlockAlloca(function, Name, varType);
    
    llvm::Value* initVal = nullptr;
    if (InitVal) {
        initVal = InitVal->codegen(context);
        if (!initVal) return nullptr;
        context.getBuilder().CreateStore(initVal, alloca);
    } else {
        // Initialize with default value (0.0 for double, 0 for int, false for bool)
        llvm::Value* defaultVal;
        if (varType->isDoubleTy()) {
            defaultVal = llvm::ConstantFP::get(varType, 0.0);
        } else if (varType->isIntegerTy()) {
            defaultVal = llvm::ConstantInt::get(varType, 0);
        } else {
            defaultVal = llvm::Constant::getNullValue(varType);
        }
        context.getBuilder().CreateStore(defaultVal, alloca);
    }
    
    context.setNamedValue(Name, alloca);
    return alloca;
}

// AssignmentAST implementation
llvm::Value* AssignmentAST::codegen(CodeGenContext& context) {
    llvm::Value* variable = context.getNamedValue(Name);
    if (!variable) {
        std::cerr << "Unknown variable name in assignment: " << Name << std::endl;
        return nullptr;
    }
    
    llvm::Value* val = Value->codegen(context);
    if (!val) return nullptr;
    
    // Ensure we're storing to an alloca instruction
    if (llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable)) {
        context.getBuilder().CreateStore(val, alloca);
        return val;
    } else {
        std::cerr << "Cannot assign to non-variable: " << Name << std::endl;
        return nullptr;
    }
}

// IfExprAST implementation
llvm::Value* IfExprAST::codegen(CodeGenContext& context) {
    llvm::Value* CondV = Cond->codegen(context);
    if (!CondV) return nullptr;
    
    // Convert condition to a bool by comparing non-equal to 0.0
    CondV = context.getBuilder().CreateFCmpONE(CondV, 
                                               llvm::ConstantFP::get(context.getContext(), llvm::APFloat(0.0)),
                                               "ifcond");
    
    llvm::Function* function = context.getBuilder().GetInsertBlock()->getParent();
    
    // Create blocks for the then and else cases. Insert the 'then' block at the end of the function.
    llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(context.getContext(), "then", function);
    llvm::BasicBlock* ElseBB = llvm::BasicBlock::Create(context.getContext(), "else");
    llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(context.getContext(), "ifcont");
    
    context.getBuilder().CreateCondBr(CondV, ThenBB, ElseBB);
    
    // Emit then value
    context.getBuilder().SetInsertPoint(ThenBB);
    llvm::Value* ThenV = Then->codegen(context);
    if (!ThenV) return nullptr;
    
    context.getBuilder().CreateBr(MergeBB);
    ThenBB = context.getBuilder().GetInsertBlock();
    
    // Emit else block
    ElseBB->insertInto(function);
    context.getBuilder().SetInsertPoint(ElseBB);
    
    llvm::Value* ElseV = nullptr;
    if (Else) {
        ElseV = Else->codegen(context);
        if (!ElseV) return nullptr;
    } else {
        ElseV = llvm::ConstantFP::get(context.getContext(), llvm::APFloat(0.0));
    }
    
    context.getBuilder().CreateBr(MergeBB);
    ElseBB = context.getBuilder().GetInsertBlock();
    
    // Emit merge block
    MergeBB->insertInto(function);
    context.getBuilder().SetInsertPoint(MergeBB);
    
    llvm::PHINode* PN = context.getBuilder().CreatePHI(llvm::Type::getDoubleTy(context.getContext()), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    
    return PN;
}

// BlockAST implementation
llvm::Value* BlockAST::codegen(CodeGenContext& context) {
    context.pushScope();
    
    llvm::Value* lastValue = nullptr;
    for (auto& stmt : Statements) {
        lastValue = stmt->codegen(context);
        if (!lastValue) {
            context.popScope();
            return nullptr;
        }
    }
    
    context.popScope();
    return lastValue ? lastValue : llvm::ConstantFP::get(context.getContext(), llvm::APFloat(0.0));
}

// FunctionAST implementation
llvm::Function* FunctionAST::codegenFunction(CodeGenContext& context) {
    llvm::Function* function = Proto->codegenFunction(context);
    if (!function) return nullptr;
    
    // Create a new basic block to start insertion into
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(context.getContext(), "entry", function);
    context.getBuilder().SetInsertPoint(BB);
    
    // Record the function arguments in the NamedValues map
    context.pushScope();
    for (auto& Arg : function->args()) {
        // Create an alloca for this variable
        llvm::AllocaInst* alloca = context.createEntryBlockAlloca(function, std::string(Arg.getName()), Arg.getType());
        
        // Store the initial value into the alloca
        context.getBuilder().CreateStore(&Arg, alloca);
        
        // Add argument to variable symbol table
        context.setNamedValue(std::string(Arg.getName()), alloca);
    }
    
    if (llvm::Value* RetVal = Body->codegen(context)) {
        // If the function doesn't end with a return, add one
        llvm::BasicBlock* currentBB = context.getBuilder().GetInsertBlock();
        if (!currentBB->getTerminator()) {
            if (function->getReturnType()->isVoidTy()) {
                context.getBuilder().CreateRetVoid();
            } else {
                context.getBuilder().CreateRet(RetVal);
            }
        }
        
        context.popScope();
        
        // Validate the generated code, checking for consistency
        if (llvm::verifyFunction(*function, &llvm::errs())) {
            function->eraseFromParent();
            return nullptr;
        }
        
        return function;
    }
    
    // Error reading body, remove function
    function->eraseFromParent();
    context.popScope();
    return nullptr;
}

// FunctionAST codegen implementation for virtual function
llvm::Value* FunctionAST::codegen(CodeGenContext& context) {
    return codegenFunction(context);
}

// ReturnAST implementation
llvm::Value* ReturnAST::codegen(CodeGenContext& context) {
    if (RetVal) {
        llvm::Value* val = RetVal->codegen(context);
        if (!val) return nullptr;
        return context.getBuilder().CreateRet(val);
    } else {
        return context.getBuilder().CreateRetVoid();
    }
}

// ExpressionStmtAST implementation
llvm::Value* ExpressionStmtAST::codegen(CodeGenContext& context) {
    return Expr->codegen(context);
}
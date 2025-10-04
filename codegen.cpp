#include "ast.h"
#include "codegen.h"
#include <llvm/IR/Constants.h>

// Integer literal code generation
llvm::Value* IntegerNode::codeGen(CodeGenContext& context) {
    return llvm::ConstantInt::get(context.context, llvm::APInt(32, value, true));
}

// Double literal code generation
llvm::Value* DoubleNode::codeGen(CodeGenContext& context) {
    return llvm::ConstantFP::get(context.context, llvm::APFloat(value));
}

// Identifier code generation
llvm::Value* IdentifierNode::codeGen(CodeGenContext& context) {
    llvm::AllocaInst* alloca = context.namedValues[name];
    if (!alloca) {
        std::cerr << "Error: Unknown variable name: " << name << std::endl;
        return nullptr;
    }
    return context.builder.CreateLoad(alloca->getAllocatedType(), alloca, name.c_str());
}

// Binary operation code generation
llvm::Value* BinaryOpNode::codeGen(CodeGenContext& context) {
    llvm::Value* L = lhs->codeGen(context);
    llvm::Value* R = rhs->codeGen(context);
    
    if (!L || !R) return nullptr;
    
    // Handle type promotion if needed
    if (L->getType()->isDoubleTy() && R->getType()->isIntegerTy()) {
        R = context.builder.CreateSIToFP(R, llvm::Type::getDoubleTy(context.context), "cast");
    } else if (L->getType()->isIntegerTy() && R->getType()->isDoubleTy()) {
        L = context.builder.CreateSIToFP(L, llvm::Type::getDoubleTy(context.context), "cast");
    }
    
    bool isFloat = L->getType()->isDoubleTy() || L->getType()->isFloatTy();
    
    switch (op) {
        case '+':
            return isFloat ? context.builder.CreateFAdd(L, R, "addtmp") 
                          : context.builder.CreateAdd(L, R, "addtmp");
        case '-':
            return isFloat ? context.builder.CreateFSub(L, R, "subtmp")
                          : context.builder.CreateSub(L, R, "subtmp");
        case '*':
            return isFloat ? context.builder.CreateFMul(L, R, "multmp")
                          : context.builder.CreateMul(L, R, "multmp");
        case '/':
            return isFloat ? context.builder.CreateFDiv(L, R, "divtmp")
                          : context.builder.CreateSDiv(L, R, "divtmp");
        case '<':
            L = isFloat ? context.builder.CreateFCmpULT(L, R, "cmptmp")
                       : context.builder.CreateICmpSLT(L, R, "cmptmp");
            return context.builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context.context), "booltmp");
        case '>':
            L = isFloat ? context.builder.CreateFCmpUGT(L, R, "cmptmp")
                       : context.builder.CreateICmpSGT(L, R, "cmptmp");
            return context.builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context.context), "booltmp");
        case 256: // '==' (using higher value to avoid conflict)
            L = isFloat ? context.builder.CreateFCmpUEQ(L, R, "cmptmp")
                       : context.builder.CreateICmpEQ(L, R, "cmptmp");
            return context.builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context.context), "booltmp");
        case 257: // '!='
            L = isFloat ? context.builder.CreateFCmpUNE(L, R, "cmptmp")
                       : context.builder.CreateICmpNE(L, R, "cmptmp");
            return context.builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context.context), "booltmp");
        default:
            std::cerr << "Error: Unknown binary operator: " << op << std::endl;
            return nullptr;
    }
}

// Unary operation code generation
llvm::Value* UnaryOpNode::codeGen(CodeGenContext& context) {
    llvm::Value* operand = expr->codeGen(context);
    if (!operand) return nullptr;
    
    switch (op) {
        case '-':
            if (operand->getType()->isDoubleTy() || operand->getType()->isFloatTy()) {
                return context.builder.CreateFNeg(operand, "negtmp");
            } else {
                return context.builder.CreateNeg(operand, "negtmp");
            }
        case '!':
            // Convert to boolean, then negate
            operand = context.builder.CreateICmpEQ(operand, 
                llvm::ConstantInt::get(context.context, llvm::APInt(32, 0)), "nottmp");
            return context.builder.CreateZExt(operand, 
                llvm::Type::getInt32Ty(context.context), "boolcast");
        default:
            std::cerr << "Error: Unknown unary operator: " << op << std::endl;
            return nullptr;
    }
}

// Function call code generation
llvm::Value* FunctionCallNode::codeGen(CodeGenContext& context) {
    llvm::Function* calleeF = context.module->getFunction(name);
    if (!calleeF) {
        std::cerr << "Error: Unknown function referenced: " << name << std::endl;
        return nullptr;
    }
    
    if (calleeF->arg_size() != args.size()) {
        std::cerr << "Error: Incorrect number of arguments passed to " << name << std::endl;
        return nullptr;
    }
    
    std::vector<llvm::Value*> argsV;
    for (unsigned i = 0; i < args.size(); ++i) {
        argsV.push_back(args[i]->codeGen(context));
        if (!argsV.back()) return nullptr;
    }
    
    return context.builder.CreateCall(calleeF, argsV, "calltmp");
}

// Variable declaration code generation
llvm::Value* VariableDeclNode::codeGen(CodeGenContext& context) {
    llvm::Type* varType = context.getType(type);
    
    llvm::AllocaInst* alloca = context.createEntryBlockAlloca(
        context.currentFunction, name, varType);
    
    if (initExpr) {
        llvm::Value* initVal = initExpr->codeGen(context);
        if (!initVal) return nullptr;
        
        // Handle type conversion if needed
        if (initVal->getType() != varType) {
            if (varType->isDoubleTy() && initVal->getType()->isIntegerTy()) {
                initVal = context.builder.CreateSIToFP(initVal, varType, "cast");
            } else if (varType->isIntegerTy() && initVal->getType()->isDoubleTy()) {
                initVal = context.builder.CreateFPToSI(initVal, varType, "cast");
            }
        }
        
        context.builder.CreateStore(initVal, alloca);
    }
    
    context.namedValues[name] = alloca;
    return alloca;
}

// Assignment code generation
llvm::Value* AssignmentNode::codeGen(CodeGenContext& context) {
    llvm::Value* val = expr->codeGen(context);
    if (!val) return nullptr;
    
    llvm::AllocaInst* variable = context.namedValues[name];
    if (!variable) {
        std::cerr << "Error: Unknown variable name: " << name << std::endl;
        return nullptr;
    }
    
    // Handle type conversion if needed
    llvm::Type* varType = variable->getAllocatedType();
    if (val->getType() != varType) {
        if (varType->isDoubleTy() && val->getType()->isIntegerTy()) {
            val = context.builder.CreateSIToFP(val, varType, "cast");
        } else if (varType->isIntegerTy() && val->getType()->isDoubleTy()) {
            val = context.builder.CreateFPToSI(val, varType, "cast");
        }
    }
    
    context.builder.CreateStore(val, variable);
    return val;
}

// Block code generation
llvm::Value* BlockNode::codeGen(CodeGenContext& context) {
    llvm::Value* last = nullptr;
    for (auto& stmt : statements) {
        last = stmt->codeGen(context);
    }
    return last;
}

// If statement code generation
llvm::Value* IfStmtNode::codeGen(CodeGenContext& context) {
    llvm::Value* condV = condition->codeGen(context);
    if (!condV) return nullptr;
    
    // Convert condition to boolean
    if (condV->getType()->isDoubleTy()) {
        condV = context.builder.CreateFCmpONE(condV,
            llvm::ConstantFP::get(context.context, llvm::APFloat(0.0)), "ifcond");
    } else {
        condV = context.builder.CreateICmpNE(condV,
            llvm::ConstantInt::get(context.context, llvm::APInt(32, 0)), "ifcond");
    }
    
    llvm::Function* function = context.builder.GetInsertBlock()->getParent();
    
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context.context, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context.context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context.context, "ifcont");
    
    if (elseBlock) {
        context.builder.CreateCondBr(condV, thenBB, elseBB);
    } else {
        context.builder.CreateCondBr(condV, thenBB, mergeBB);
    }
    
    // Then block
    context.builder.SetInsertPoint(thenBB);
    llvm::Value* thenV = thenBlock->codeGen(context);
    if (!thenV) return nullptr;
    
    if (!context.builder.GetInsertBlock()->getTerminator()) {
        context.builder.CreateBr(mergeBB);
    }
    thenBB = context.builder.GetInsertBlock();
    
    // Else block
    if (elseBlock) {
        function->insert(function->end(), elseBB);
        context.builder.SetInsertPoint(elseBB);
        llvm::Value* elseV = elseBlock->codeGen(context);
        if (!elseV) return nullptr;
        
        if (!context.builder.GetInsertBlock()->getTerminator()) {
            context.builder.CreateBr(mergeBB);
        }
        elseBB = context.builder.GetInsertBlock();
    }
    
    // Merge block
    function->insert(function->end(), mergeBB);
    context.builder.SetInsertPoint(mergeBB);
    
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(context.context));
}

// While statement code generation
llvm::Value* WhileStmtNode::codeGen(CodeGenContext& context) {
    llvm::Function* function = context.builder.GetInsertBlock()->getParent();
    
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(context.context, "whilecond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(context.context, "whileloop", function);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(context.context, "afterloop", function);
    
    context.builder.CreateBr(condBB);
    
    // Condition block
    context.builder.SetInsertPoint(condBB);
    llvm::Value* condV = condition->codeGen(context);
    if (!condV) return nullptr;
    
    // Convert condition to boolean
    if (condV->getType()->isDoubleTy()) {
        condV = context.builder.CreateFCmpONE(condV,
            llvm::ConstantFP::get(context.context, llvm::APFloat(0.0)), "whilecond");
    } else {
        condV = context.builder.CreateICmpNE(condV,
            llvm::ConstantInt::get(context.context, llvm::APInt(32, 0)), "whilecond");
    }
    
    context.builder.CreateCondBr(condV, loopBB, afterBB);
    
    // Loop body
    context.builder.SetInsertPoint(loopBB);
    body->codeGen(context);
    
    if (!context.builder.GetInsertBlock()->getTerminator()) {
        context.builder.CreateBr(condBB);
    }
    
    // After loop
    context.builder.SetInsertPoint(afterBB);
    
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(context.context));
}

// Return statement code generation
llvm::Value* ReturnStmtNode::codeGen(CodeGenContext& context) {
    if (expr) {
        llvm::Value* retVal = expr->codeGen(context);
        if (!retVal) return nullptr;
        return context.builder.CreateRet(retVal);
    } else {
        return context.builder.CreateRetVoid();
    }
}

// Expression statement code generation
llvm::Value* ExprStmtNode::codeGen(CodeGenContext& context) {
    return expr->codeGen(context);
}

// Function code generation
llvm::Value* FunctionNode::codeGen(CodeGenContext& context) {
    std::vector<llvm::Type*> paramTypes;
    for (auto& param : params) {
        paramTypes.push_back(context.getType(param->type));
    }
    
    llvm::Type* retType = context.getType(returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);
    
    llvm::Function* function = llvm::Function::Create(funcType,
        llvm::Function::ExternalLinkage, name, context.module.get());
    
    // Set parameter names
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(params[idx++]->name);
    }
    
    // Create entry block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(context.context, "entry", function);
    context.builder.SetInsertPoint(bb);
    
    // Store old function context
    llvm::Function* oldFunction = context.currentFunction;
    std::map<std::string, llvm::AllocaInst*> oldNamedValues = context.namedValues;
    
    context.currentFunction = function;
    context.namedValues.clear();
    
    // Allocate parameters
    idx = 0;
    for (auto& arg : function->args()) {
        llvm::AllocaInst* alloca = context.createEntryBlockAlloca(
            function, std::string(arg.getName()), arg.getType());
        context.builder.CreateStore(&arg, alloca);
        context.namedValues[std::string(arg.getName())] = alloca;
    }
    
    // Generate function body
    if (body) {
        body->codeGen(context);
        
        // Add return if not present
        if (!context.builder.GetInsertBlock()->getTerminator()) {
            if (returnType == "void") {
                context.builder.CreateRetVoid();
            } else {
                context.builder.CreateRet(llvm::Constant::getNullValue(retType));
            }
        }
    }
    
    // Restore old context
    context.currentFunction = oldFunction;
    context.namedValues = oldNamedValues;
    
    // Verify function
    std::string errorStr;
    llvm::raw_string_ostream errorStream(errorStr);
    if (llvm::verifyFunction(*function, &errorStream)) {
        std::cerr << "Error in function " << name << ": " << errorStr << std::endl;
        function->eraseFromParent();
        return nullptr;
    }
    
    return function;
}

// Program code generation
llvm::Value* ProgramNode::codeGen(CodeGenContext& context) {
    for (auto& func : functions) {
        func->codeGen(context);
    }
    return nullptr;
}

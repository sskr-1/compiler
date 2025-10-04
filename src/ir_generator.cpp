#include "ir_generator.h"
#include "ast.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>

namespace clike {

IRGenerator::IRGenerator() 
    : context_(std::make_unique<llvm::LLVMContext>()),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)),
      module_(nullptr) {
    // Initialize symbol table with global scope
    symbolTable_.push_back({});
}

IRGenerator::~IRGenerator() = default;

llvm::Module* IRGenerator::createModule(const std::string& name) {
    module_ = std::make_unique<llvm::Module>(name, *context_);
    setupBuiltinFunctions();
    return module_.get();
}

void IRGenerator::printModule(llvm::raw_ostream& os) {
    if (module_) {
        module_->print(os, nullptr);
    }
}

bool IRGenerator::verifyModule() {
    if (!module_) return false;
    
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    bool isValid = !llvm::verifyModule(*module_, &errorStream);
    
    if (!isValid) {
        std::cerr << "Module verification failed: " << error << std::endl;
    }
    
    return isValid;
}

llvm::Value* IRGenerator::codegen(NumberExpr& expr) {
    return llvm::ConstantFP::get(*context_, llvm::APFloat(expr.getValue()));
}

llvm::Value* IRGenerator::codegen(VariableExpr& expr) {
    llvm::Value* value = getSymbol(expr.getName());
    if (!value) {
        std::cerr << "Unknown variable: " << expr.getName() << std::endl;
        return nullptr;
    }
    
    // Load the value from the alloca
    return builder_->CreateLoad(getDoubleType(), value, expr.getName().c_str());
}

llvm::Value* IRGenerator::codegen(BinaryExpr& expr) {
    llvm::Value* left = expr.getLeft()->codegen(*this);
    llvm::Value* right = expr.getRight()->codegen(*this);
    
    if (!left || !right) {
        return nullptr;
    }
    
    return createBinaryOperation(expr.getOp(), left, right);
}

llvm::Value* IRGenerator::codegen(CallExpr& expr) {
    llvm::Function* callee = getFunction(expr.getCallee());
    if (!callee) {
        std::cerr << "Unknown function: " << expr.getCallee() << std::endl;
        return nullptr;
    }
    
    if (callee->arg_size() != expr.getArgs().size()) {
        std::cerr << "Incorrect number of arguments for function: " << expr.getCallee() << std::endl;
        return nullptr;
    }
    
    std::vector<llvm::Value*> args;
    for (auto& arg : expr.getArgs()) {
        llvm::Value* argValue = arg->codegen(*this);
        if (!argValue) return nullptr;
        args.push_back(argValue);
    }
    
    return builder_->CreateCall(callee, args, "calltmp");
}

llvm::Value* IRGenerator::codegen(ExprStmt& stmt) {
    return stmt.getExpr()->codegen(*this);
}

llvm::Value* IRGenerator::codegen(VarDeclStmt& stmt) {
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    llvm::AllocaInst* alloca = createEntryBlockAlloca(function, stmt.getName());
    
    if (stmt.getInit()) {
        llvm::Value* initValue = stmt.getInit()->codegen(*this);
        if (!initValue) return nullptr;
        builder_->CreateStore(initValue, alloca);
    } else {
        // Initialize with 0.0
        llvm::Value* zero = llvm::ConstantFP::get(*context_, llvm::APFloat(0.0));
        builder_->CreateStore(zero, alloca);
    }
    
    addSymbol(stmt.getName(), alloca);
    return alloca;
}

llvm::Value* IRGenerator::codegen(IfStmt& stmt) {
    llvm::Value* condition = stmt.getCondition()->codegen(*this);
    if (!condition) return nullptr;
    
    // Convert condition to boolean
    condition = builder_->CreateFCmpONE(condition, 
        llvm::ConstantFP::get(*context_, llvm::APFloat(0.0)), "ifcond");
    
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context_, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context_, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context_, "ifcont");
    
    builder_->CreateCondBr(condition, thenBB, elseBB);
    
    // Emit then block
    builder_->SetInsertPoint(thenBB);
    llvm::Value* thenValue = stmt.getThenStmt()->codegen(*this);
    if (!thenValue) return nullptr;
    builder_->CreateBr(mergeBB);
    
    // Emit else block
    elseBB->insertInto(function);
    builder_->SetInsertPoint(elseBB);
    llvm::Value* elseValue = nullptr;
    if (stmt.getElseStmt()) {
        elseValue = stmt.getElseStmt()->codegen(*this);
        if (!elseValue) return nullptr;
    }
    builder_->CreateBr(mergeBB);
    
    // Emit merge block
    mergeBB->insertInto(function);
    builder_->SetInsertPoint(mergeBB);
    
    return llvm::Constant::getNullValue(getDoubleType());
}

llvm::Value* IRGenerator::codegen(WhileStmt& stmt) {
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context_, "loop", function);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context_, "loopbody");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context_, "afterloop");
    
    builder_->CreateBr(loopBB);
    builder_->SetInsertPoint(loopBB);
    
    // Check condition
    llvm::Value* condition = stmt.getCondition()->codegen(*this);
    if (!condition) return nullptr;
    
    condition = builder_->CreateFCmpONE(condition, 
        llvm::ConstantFP::get(*context_, llvm::APFloat(0.0)), "loopcond");
    
    builder_->CreateCondBr(condition, bodyBB, afterBB);
    
    // Emit body
    bodyBB->insertInto(function);
    builder_->SetInsertPoint(bodyBB);
    llvm::Value* bodyValue = stmt.getBody()->codegen(*this);
    if (!bodyValue) return nullptr;
    builder_->CreateBr(loopBB);
    
    // Emit after block
    afterBB->insertInto(function);
    builder_->SetInsertPoint(afterBB);
    
    return llvm::Constant::getNullValue(getDoubleType());
}

llvm::Value* IRGenerator::codegen(ReturnStmt& stmt) {
    if (stmt.getExpr()) {
        llvm::Value* returnValue = stmt.getExpr()->codegen(*this);
        if (!returnValue) return nullptr;
        builder_->CreateRet(returnValue);
    } else {
        builder_->CreateRetVoid();
    }
    return llvm::Constant::getNullValue(getDoubleType());
}

llvm::Value* IRGenerator::codegen(BlockStmt& stmt) {
    enterScope();
    
    llvm::Value* lastValue = nullptr;
    for (auto& statement : stmt.getStatements()) {
        lastValue = statement->codegen(*this);
        if (!lastValue) {
            exitScope();
            return nullptr;
        }
    }
    
    exitScope();
    return lastValue;
}

llvm::Function* IRGenerator::codegen(FunctionDef& func) {
    if (!module_) {
        std::cerr << "No module created" << std::endl;
        return nullptr;
    }
    
    // Check if function already exists
    llvm::Function* existingFunction = module_->getFunction(func.getName());
    if (existingFunction) {
        std::cerr << "Function " << func.getName() << " already exists" << std::endl;
        return nullptr;
    }
    
    // Create function type
    std::vector<llvm::Type*> argTypes(func.getArgs().size(), getDoubleType());
    llvm::FunctionType* functionType = llvm::FunctionType::get(getDoubleType(), argTypes, false);
    
    // Create function
    llvm::Function* function = llvm::Function::Create(functionType, 
        llvm::Function::ExternalLinkage, func.getName(), module_.get());
    
    // Create basic block
    llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(basicBlock);
    
    // Add arguments to symbol table
    enterScope();
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        llvm::AllocaInst* alloca = createEntryBlockAlloca(function, func.getArgs()[idx]);
        builder_->CreateStore(&arg, alloca);
        addSymbol(func.getArgs()[idx], alloca);
        ++idx;
    }
    
    // Generate function body
    llvm::Value* bodyValue = func.getBody()->codegen(*this);
    if (!bodyValue) {
        function->eraseFromParent();
        exitScope();
        return nullptr;
    }
    
    // Add return statement if not present
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateRet(llvm::ConstantFP::get(*context_, llvm::APFloat(0.0)));
    }
    
    exitScope();
    
    // Verify function
    if (llvm::verifyFunction(*function)) {
        function->eraseFromParent();
        return nullptr;
    }
    
    return function;
}

llvm::Module* IRGenerator::codegen(Program& program) {
    if (!module_) {
        createModule("clike_module");
    }
    
    // Generate all functions
    for (auto& func : program.getFunctions()) {
        llvm::Function* function = codegen(*func);
        if (!function) {
            return nullptr;
        }
    }
    
    // Create main function if it doesn't exist
    if (!module_->getFunction("main")) {
        createMainFunction();
    }
    
    return module_.get();
}

llvm::Type* IRGenerator::getDoubleType() {
    return llvm::Type::getDoubleTy(*context_);
}

llvm::Function* IRGenerator::getFunction(const std::string& name) {
    if (!module_) return nullptr;
    return module_->getFunction(name);
}

llvm::AllocaInst* IRGenerator::createEntryBlockAlloca(llvm::Function* function, const std::string& varName) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(getDoubleType(), nullptr, varName.c_str());
}

void IRGenerator::enterScope() {
    symbolTable_.push_back({});
}

void IRGenerator::exitScope() {
    if (symbolTable_.size() > 1) {
        symbolTable_.pop_back();
    }
}

void IRGenerator::addSymbol(const std::string& name, llvm::Value* value) {
    symbolTable_.back()[name] = value;
}

llvm::Value* IRGenerator::getSymbol(const std::string& name) {
    for (auto it = symbolTable_.rbegin(); it != symbolTable_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return nullptr;
}

llvm::Value* IRGenerator::createBinaryOperation(char op, llvm::Value* left, llvm::Value* right) {
    switch (op) {
        case '+': return builder_->CreateFAdd(left, right, "addtmp");
        case '-': return builder_->CreateFSub(left, right, "subtmp");
        case '*': return builder_->CreateFMul(left, right, "multmp");
        case '/': return builder_->CreateFDiv(left, right, "divtmp");
        case '<': return builder_->CreateFCmpULT(left, right, "cmptmp");
        case '>': return builder_->CreateFCmpUGT(left, right, "cmptmp");
        case '=': return builder_->CreateFCmpUEQ(left, right, "cmptmp");
        default:
            std::cerr << "Invalid binary operator: " << op << std::endl;
            return nullptr;
    }
}

llvm::Function* IRGenerator::createMainFunction() {
    llvm::FunctionType* mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), false);
    llvm::Function* mainFunction = llvm::Function::Create(mainType, 
        llvm::Function::ExternalLinkage, "main", module_.get());
    
    llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(*context_, "entry", mainFunction);
    builder_->SetInsertPoint(basicBlock);
    
    // Call user-defined main function if it exists
    llvm::Function* userMain = module_->getFunction("main");
    if (userMain && userMain != mainFunction) {
        llvm::Value* result = builder_->CreateCall(userMain, {}, "mainresult");
        builder_->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0));
    } else {
        builder_->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0));
    }
    
    return mainFunction;
}

void IRGenerator::setupBuiltinFunctions() {
    if (!module_) return;
    
    // Create printf function
    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*context_)));
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context_), printfArgs, true);
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module_.get());
}

} // namespace clike
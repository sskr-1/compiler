#include "codegen.h"
#include <iostream>
#include <vector>

CodeGenerator::CodeGenerator() 
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>("clike_module", *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context)),
      current_function(nullptr) {
    initialize();
}

CodeGenerator::~CodeGenerator() = default;

void CodeGenerator::initialize() {
    // Initialize LLVM targets
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // Create a new scope
    enterScope();
}

void CodeGenerator::enterScope() {
    scope_stack.push(std::map<std::string, llvm::Value*>());
}

void CodeGenerator::exitScope() {
    if (!scope_stack.empty()) {
        scope_stack.pop();
    }
}

llvm::Value* CodeGenerator::getVariable(const std::string& name) {
    // Search from top of stack (most recent scope) to bottom
    std::stack<std::map<std::string, llvm::Value*>> temp_stack = scope_stack;
    while (!temp_stack.empty()) {
        auto& scope = temp_stack.top();
        auto it = scope.find(name);
        if (it != scope.end()) {
            return it->second;
        }
        temp_stack.pop();
    }
    return nullptr;
}

void CodeGenerator::setVariable(const std::string& name, llvm::Value* value) {
    if (!scope_stack.empty()) {
        scope_stack.top()[name] = value;
    }
}

llvm::Function* CodeGenerator::getFunction(const std::string& name) {
    return module->getFunction(name);
}

llvm::Type* CodeGenerator::getLLVMType(const std::string& type_name) {
    if (type_name == "int") {
        return llvm::Type::getInt32Ty(*context);
    } else if (type_name == "float") {
        return llvm::Type::getFloatTy(*context);
    } else if (type_name == "char") {
        return llvm::Type::getInt8Ty(*context);
    } else if (type_name == "void") {
        return llvm::Type::getVoidTy(*context);
    }
    return llvm::Type::getInt32Ty(*context); // Default to int
}

llvm::Value* CodeGenerator::createCast(llvm::Value* value, llvm::Type* target_type) {
    if (value->getType() == target_type) {
        return value;
    }
    
    // Handle int to float conversion
    if (value->getType()->isIntegerTy() && target_type->isFloatingPointTy()) {
        return builder->CreateSIToFP(value, target_type, "cast");
    }
    
    // Handle float to int conversion
    if (value->getType()->isFloatingPointTy() && target_type->isIntegerTy()) {
        return builder->CreateFPToSI(value, target_type, "cast");
    }
    
    // Handle int to int conversion (different sizes)
    if (value->getType()->isIntegerTy() && target_type->isIntegerTy()) {
        return builder->CreateIntCast(value, target_type, true, "cast");
    }
    
    return value; // No conversion needed or possible
}

// Function definition codegen
llvm::Value* CodeGenerator::codegenFunctionDefinition(FunctionDefinitionNode* node) {
    // Get function name and return type
    std::string func_name = node->getName();
    llvm::Type* return_type = node->getReturnType()->getLLVMType(*context);
    
    // Create parameter types
    std::vector<llvm::Type*> param_types;
    if (node->getParameters()) {
        for (auto* param : *node->getParameters()) {
            ParameterNode* param_node = static_cast<ParameterNode*>(param);
            llvm::Type* param_type = param_node->getType()->getLLVMType(*context);
            param_types.push_back(param_type);
        }
    }
    
    // Create function type
    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);
    
    // Create function
    llvm::Function* function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, func_name, module.get());
    
    // Set names for parameters
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        if (node->getParameters() && idx < node->getParameters()->size()) {
            ParameterNode* param_node = static_cast<ParameterNode*>((*node->getParameters())[idx]);
            arg.setName(param_node->getName());
        }
        idx++;
    }
    
    // Create basic block for function body
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);
    
    // Enter new scope for function parameters
    enterScope();
    
    // Store parameters in symbol table
    idx = 0;
    for (auto& arg : function->args()) {
        if (node->getParameters() && idx < node->getParameters()->size()) {
            ParameterNode* param_node = static_cast<ParameterNode*>((*node->getParameters())[idx]);
            setVariable(param_node->getName(), &arg);
        }
        idx++;
    }
    
    // Set current function
    current_function = function;
    
    // Generate function body
    if (node->getBody()) {
        llvm::Value* body_value = node->getBody()->codegen(*this);
        
        // If function has void return type and no explicit return, add one
        if (return_type->isVoidTy() && !builder->GetInsertBlock()->getTerminator()) {
            builder->CreateRetVoid();
        }
    }
    
    // Exit function scope
    exitScope();
    current_function = nullptr;
    
    return function;
}

// Variable declaration codegen
llvm::Value* CodeGenerator::codegenVariableDeclaration(VariableDeclarationNode* node) {
    std::string var_name = node->getName();
    llvm::Type* var_type = node->getType()->getLLVMType(*context);
    
    // Create alloca for variable
    llvm::AllocaInst* alloca = builder->CreateAlloca(var_type, nullptr, var_name);
    
    // Store in symbol table
    setVariable(var_name, alloca);
    
    // If there's an initializer, generate it and store the value
    if (node->getInitializer()) {
        llvm::Value* init_value = node->getInitializer()->codegen(*this);
        if (init_value) {
            // Cast if necessary
            llvm::Value* casted_value = createCast(init_value, var_type);
            builder->CreateStore(casted_value, alloca);
        }
    }
    
    return alloca;
}

// Compound statement codegen
llvm::Value* CodeGenerator::codegenCompoundStatement(CompoundStatementNode* node) {
    enterScope();
    
    llvm::Value* last_value = nullptr;
    if (node->getStatements()) {
        for (auto* stmt : *node->getStatements()) {
            last_value = stmt->codegen(*this);
        }
    }
    
    exitScope();
    return last_value;
}

// Expression statement codegen
llvm::Value* CodeGenerator::codegenExpressionStatement(ExpressionStatementNode* node) {
    if (node->getExpression()) {
        return node->getExpression()->codegen(*this);
    }
    return nullptr;
}

// If statement codegen
llvm::Value* CodeGenerator::codegenIfStatement(IfStatementNode* node) {
    llvm::Value* cond_value = node->getCondition()->codegen(*this);
    if (!cond_value) return nullptr;
    
    // Convert condition to boolean
    cond_value = builder->CreateICmpNE(cond_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), "ifcond");
    
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(*context, "then", function);
    llvm::BasicBlock* else_bb = llvm::BasicBlock::Create(*context, "else");
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(*context, "ifcont");
    
    // Create conditional branch
    builder->CreateCondBr(cond_value, then_bb, else_bb);
    
    // Generate then block
    builder->SetInsertPoint(then_bb);
    llvm::Value* then_value = node->getThenStatement()->codegen(*this);
    builder->CreateBr(merge_bb);
    
    // Generate else block
    function->getBasicBlockList().push_back(else_bb);
    builder->SetInsertPoint(else_bb);
    llvm::Value* else_value = nullptr;
    if (node->getElseStatement()) {
        else_value = node->getElseStatement()->codegen(*this);
    }
    builder->CreateBr(merge_bb);
    
    // Generate merge block
    function->getBasicBlockList().push_back(merge_bb);
    builder->SetInsertPoint(merge_bb);
    
    return nullptr; // If statements don't return values
}

// While statement codegen
llvm::Value* CodeGenerator::codegenWhileStatement(WhileStatementNode* node) {
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(*context, "whilecond", function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(*context, "whilebody");
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(*context, "whileafter");
    
    // Push break/continue targets
    break_stack.push(after_bb);
    continue_stack.push(cond_bb);
    
    // Branch to condition
    builder->CreateBr(cond_bb);
    
    // Generate condition block
    builder->SetInsertPoint(cond_bb);
    llvm::Value* cond_value = node->getCondition()->codegen(*this);
    cond_value = builder->CreateICmpNE(cond_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), "whilecond");
    builder->CreateCondBr(cond_value, body_bb, after_bb);
    
    // Generate body block
    function->getBasicBlockList().push_back(body_bb);
    builder->SetInsertPoint(body_bb);
    node->getBody()->codegen(*this);
    builder->CreateBr(cond_bb);
    
    // Generate after block
    function->getBasicBlockList().push_back(after_bb);
    builder->SetInsertPoint(after_bb);
    
    // Pop break/continue targets
    break_stack.pop();
    continue_stack.pop();
    
    return nullptr;
}

// For statement codegen
llvm::Value* CodeGenerator::codegenForStatement(ForStatementNode* node) {
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* init_bb = llvm::BasicBlock::Create(*context, "forinit", function);
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(*context, "forcond");
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(*context, "forbody");
    llvm::BasicBlock* inc_bb = llvm::BasicBlock::Create(*context, "forinc");
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(*context, "forafter");
    
    // Push break/continue targets
    break_stack.push(after_bb);
    continue_stack.push(inc_bb);
    
    // Branch to init
    builder->CreateBr(init_bb);
    
    // Generate init block
    builder->SetInsertPoint(init_bb);
    if (node->getInit()) {
        node->getInit()->codegen(*this);
    }
    builder->CreateBr(cond_bb);
    
    // Generate condition block
    function->getBasicBlockList().push_back(cond_bb);
    builder->SetInsertPoint(cond_bb);
    llvm::Value* cond_value = nullptr;
    if (node->getCondition()) {
        cond_value = node->getCondition()->codegen(*this);
        cond_value = builder->CreateICmpNE(cond_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), "forcond");
    } else {
        cond_value = llvm::ConstantInt::getTrue(*context);
    }
    builder->CreateCondBr(cond_value, body_bb, after_bb);
    
    // Generate body block
    function->getBasicBlockList().push_back(body_bb);
    builder->SetInsertPoint(body_bb);
    node->getBody()->codegen(*this);
    builder->CreateBr(inc_bb);
    
    // Generate increment block
    function->getBasicBlockList().push_back(inc_bb);
    builder->SetInsertPoint(inc_bb);
    if (node->getIncrement()) {
        node->getIncrement()->codegen(*this);
    }
    builder->CreateBr(cond_bb);
    
    // Generate after block
    function->getBasicBlockList().push_back(after_bb);
    builder->SetInsertPoint(after_bb);
    
    // Pop break/continue targets
    break_stack.pop();
    continue_stack.pop();
    
    return nullptr;
}

// Return statement codegen
llvm::Value* CodeGenerator::codegenReturnStatement(ReturnStatementNode* node) {
    if (node->getExpression()) {
        llvm::Value* ret_value = node->getExpression()->codegen(*this);
        llvm::Type* return_type = current_function->getReturnType();
        
        if (return_type->isVoidTy()) {
            builder->CreateRetVoid();
        } else {
            llvm::Value* casted_value = createCast(ret_value, return_type);
            builder->CreateRet(casted_value);
        }
    } else {
        builder->CreateRetVoid();
    }
    return nullptr;
}

// Break statement codegen
llvm::Value* CodeGenerator::codegenBreakStatement(BreakStatementNode* node) {
    if (!break_stack.empty()) {
        builder->CreateBr(break_stack.top());
    }
    return nullptr;
}

// Continue statement codegen
llvm::Value* CodeGenerator::codegenContinueStatement(ContinueStatementNode* node) {
    if (!continue_stack.empty()) {
        builder->CreateBr(continue_stack.top());
    }
    return nullptr;
}

// Assignment codegen
llvm::Value* CodeGenerator::codegenAssignment(AssignmentNode* node) {
    llvm::Value* rhs_value = node->getRHS()->codegen(*this);
    if (!rhs_value) return nullptr;
    
    // Handle different LHS types
    if (IdentifierNode* id_node = dynamic_cast<IdentifierNode*>(node->getLHS())) {
        llvm::Value* variable = getVariable(id_node->getName());
        if (!variable) {
            std::cerr << "Error: Undefined variable " << id_node->getName() << std::endl;
            return nullptr;
        }
        
        llvm::Type* var_type = variable->getType()->getPointerElementType();
        llvm::Value* casted_value = createCast(rhs_value, var_type);
        builder->CreateStore(casted_value, variable);
        return casted_value;
    }
    
    return nullptr;
}

// Binary expression codegen
llvm::Value* CodeGenerator::codegenBinaryExpression(BinaryExpressionNode* node) {
    llvm::Value* left = node->getLeft()->codegen(*this);
    llvm::Value* right = node->getRight()->codegen(*this);
    
    if (!left || !right) return nullptr;
    
    std::string op = node->getOperator();
    
    // Handle different operators
    if (op == "+") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFAdd(left, right, "addtmp");
        } else {
            return builder->CreateAdd(left, right, "addtmp");
        }
    } else if (op == "-") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFSub(left, right, "subtmp");
        } else {
            return builder->CreateSub(left, right, "subtmp");
        }
    } else if (op == "*") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFMul(left, right, "multmp");
        } else {
            return builder->CreateMul(left, right, "multmp");
        }
    } else if (op == "/") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFDiv(left, right, "divtmp");
        } else {
            return builder->CreateSDiv(left, right, "divtmp");
        }
    } else if (op == "%") {
        return builder->CreateSRem(left, right, "modtmp");
    } else if (op == "==") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOEQ(left, right, "eqtmp");
        } else {
            return builder->CreateICmpEQ(left, right, "eqtmp");
        }
    } else if (op == "!=") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpONE(left, right, "netmp");
        } else {
            return builder->CreateICmpNE(left, right, "netmp");
        }
    } else if (op == "<") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOLT(left, right, "lttmp");
        } else {
            return builder->CreateICmpSLT(left, right, "lttmp");
        }
    } else if (op == ">") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOGT(left, right, "gttmp");
        } else {
            return builder->CreateICmpSGT(left, right, "gttmp");
        }
    } else if (op == "<=") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOLE(left, right, "letmp");
        } else {
            return builder->CreateICmpSLE(left, right, "letmp");
        }
    } else if (op == ">=") {
        if (left->getType()->isFloatingPointTy() || right->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOGE(left, right, "getmp");
        } else {
            return builder->CreateICmpSGE(left, right, "getmp");
        }
    } else if (op == "&&") {
        return builder->CreateAnd(left, right, "andtmp");
    } else if (op == "||") {
        return builder->CreateOr(left, right, "ortmp");
    }
    
    std::cerr << "Error: Unknown binary operator " << op << std::endl;
    return nullptr;
}

// Unary expression codegen
llvm::Value* CodeGenerator::codegenUnaryExpression(UnaryExpressionNode* node) {
    llvm::Value* operand = node->getOperand()->codegen(*this);
    if (!operand) return nullptr;
    
    std::string op = node->getOperator();
    
    if (op == "!") {
        return builder->CreateNot(operand, "nottmp");
    } else if (op == "-") {
        if (operand->getType()->isFloatingPointTy()) {
            return builder->CreateFNeg(operand, "negtmp");
        } else {
            return builder->CreateNeg(operand, "negtmp");
        }
    } else if (op == "+") {
        return operand; // Unary plus is a no-op
    } else if (op == "++") {
        if (node->isPostfix()) {
            // Postfix increment
            llvm::Value* old_value = builder->CreateLoad(operand->getType()->getPointerElementType(), operand, "oldval");
            llvm::Value* new_value = builder->CreateAdd(old_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 1)), "newval");
            builder->CreateStore(new_value, operand);
            return old_value;
        } else {
            // Prefix increment
            llvm::Value* old_value = builder->CreateLoad(operand->getType()->getPointerElementType(), operand, "oldval");
            llvm::Value* new_value = builder->CreateAdd(old_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 1)), "newval");
            builder->CreateStore(new_value, operand);
            return new_value;
        }
    } else if (op == "--") {
        if (node->isPostfix()) {
            // Postfix decrement
            llvm::Value* old_value = builder->CreateLoad(operand->getType()->getPointerElementType(), operand, "oldval");
            llvm::Value* new_value = builder->CreateSub(old_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 1)), "newval");
            builder->CreateStore(new_value, operand);
            return old_value;
        } else {
            // Prefix decrement
            llvm::Value* old_value = builder->CreateLoad(operand->getType()->getPointerElementType(), operand, "oldval");
            llvm::Value* new_value = builder->CreateSub(old_value, llvm::ConstantInt::get(*context, llvm::APInt(32, 1)), "newval");
            builder->CreateStore(new_value, operand);
            return new_value;
        }
    }
    
    std::cerr << "Error: Unknown unary operator " << op << std::endl;
    return nullptr;
}

// Identifier codegen
llvm::Value* CodeGenerator::codegenIdentifier(IdentifierNode* node) {
    llvm::Value* variable = getVariable(node->getName());
    if (!variable) {
        std::cerr << "Error: Undefined variable " << node->getName() << std::endl;
        return nullptr;
    }
    
    return builder->CreateLoad(variable->getType()->getPointerElementType(), variable, node->getName());
}

// Integer literal codegen
llvm::Value* CodeGenerator::codegenIntegerLiteral(IntegerLiteralNode* node) {
    return llvm::ConstantInt::get(*context, llvm::APInt(32, node->getValue()));
}

// Float literal codegen
llvm::Value* CodeGenerator::codegenFloatLiteral(FloatLiteralNode* node) {
    return llvm::ConstantFP::get(*context, llvm::APFloat(node->getValue()));
}

// Char literal codegen
llvm::Value* CodeGenerator::codegenCharLiteral(CharLiteralNode* node) {
    return llvm::ConstantInt::get(*context, llvm::APInt(8, node->getValue()));
}

// String literal codegen
llvm::Value* CodeGenerator::codegenStringLiteral(StringLiteralNode* node) {
    return builder->CreateGlobalStringPtr(node->getValue(), "str");
}

// Boolean literal codegen
llvm::Value* CodeGenerator::codegenBooleanLiteral(BooleanLiteralNode* node) {
    return llvm::ConstantInt::get(*context, llvm::APInt(1, node->getValue()));
}

// Function call codegen
llvm::Value* CodeGenerator::codegenFunctionCall(FunctionCallNode* node) {
    llvm::Function* callee = getFunction(node->getName());
    if (!callee) {
        std::cerr << "Error: Unknown function " << node->getName() << std::endl;
        return nullptr;
    }
    
    std::vector<llvm::Value*> args;
    if (node->getArguments()) {
        for (auto* arg : *node->getArguments()) {
            llvm::Value* arg_value = arg->codegen(*this);
            if (!arg_value) return nullptr;
            args.push_back(arg_value);
        }
    }
    
    return builder->CreateCall(callee, args, "calltmp");
}

// Array access codegen
llvm::Value* CodeGenerator::codegenArrayAccess(ArrayAccessNode* node) {
    llvm::Value* array = getVariable(node->getName());
    if (!array) {
        std::cerr << "Error: Undefined array " << node->getName() << std::endl;
        return nullptr;
    }
    
    llvm::Value* index = node->getIndex()->codegen(*this);
    if (!index) return nullptr;
    
    // Create GEP (GetElementPtr) instruction
    std::vector<llvm::Value*> indices;
    indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, 0)));
    indices.push_back(index);
    
    llvm::Value* element_ptr = builder->CreateGEP(array->getType()->getPointerElementType(), array, indices, "arrayidx");
    return builder->CreateLoad(array->getType()->getPointerElementType()->getArrayElementType(), element_ptr, "arrayval");
}

// Utility methods
void CodeGenerator::printIR() {
    module->print(llvm::outs(), nullptr);
}

void CodeGenerator::optimizeIR() {
    llvm::legacy::FunctionPassManager fpm(module.get());
    
    // Add optimization passes
    fpm.add(llvm::createInstructionCombiningPass());
    fpm.add(llvm::createReassociatePass());
    fpm.add(llvm::createGVNPass());
    fpm.add(llvm::createCFGSimplificationPass());
    
    fpm.doInitialization();
    
    for (auto& function : *module) {
        fpm.run(function);
    }
}

bool CodeGenerator::verifyModule() {
    std::string error;
    llvm::raw_string_ostream error_stream(error);
    
    if (llvm::verifyModule(*module, &error_stream)) {
        std::cerr << "Module verification failed: " << error << std::endl;
        return false;
    }
    
    return true;
}
#include "ast.h"
#include "codegen.h"
#include <iostream>

// Utility function to indent output
void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

// Type implementation
llvm::Value* Type::codegen(CodeGenerator& cg) {
    // Types don't generate code directly
    return nullptr;
}

void Type::print(int indent) const {
    printIndent(indent);
    std::cout << "Type: " << type_name << std::endl;
}

llvm::Type* Type::getLLVMType(CodeGenerator& cg) {
    return cg.getLLVMType(this);
}

// Program implementation
llvm::Value* Program::codegen(CodeGenerator& cg) {
    llvm::Value* last = nullptr;
    for (auto decl : *declarations) {
        last = decl->codegen(cg);
    }
    return last;
}

void Program::print(int indent) const {
    printIndent(indent);
    std::cout << "Program:" << std::endl;
    for (auto decl : *declarations) {
        decl->print(indent + 1);
    }
}

// IntegerLiteral implementation
llvm::Value* IntegerLiteral::codegen(CodeGenerator& cg) {
    return llvm::ConstantInt::get(cg.getContext(), llvm::APInt(32, value, true));
}

void IntegerLiteral::print(int indent) const {
    printIndent(indent);
    std::cout << "IntegerLiteral: " << value << std::endl;
}

// FloatLiteral implementation
llvm::Value* FloatLiteral::codegen(CodeGenerator& cg) {
    return llvm::ConstantFP::get(cg.getContext(), llvm::APFloat((double)value));
}

void FloatLiteral::print(int indent) const {
    printIndent(indent);
    std::cout << "FloatLiteral: " << value << std::endl;
}

// StringLiteral implementation
llvm::Value* StringLiteral::codegen(CodeGenerator& cg) {
    // For simplicity, we'll treat strings as i8* pointers
    // In a full implementation, you'd want proper string handling
    return cg.logError("String literals not yet implemented");
}

void StringLiteral::print(int indent) const {
    printIndent(indent);
    std::cout << "StringLiteral: \"" << value << "\"" << std::endl;
}

// VariableExpression implementation
llvm::Value* VariableExpression::codegen(CodeGenerator& cg) {
    llvm::Value* value = cg.getNamedValue(name);
    if (!value) {
        return cg.logError("Unknown variable name: " + name);
    }
    return cg.getBuilder().CreateLoad(value, name);
}

void VariableExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "VariableExpression: " << name << std::endl;
}

// BinaryOpExpression implementation
llvm::Value* BinaryOpExpression::codegen(CodeGenerator& cg) {
    llvm::Value* left_val = left->codegen(cg);
    llvm::Value* right_val = right->codegen(cg);

    if (!left_val || !right_val) {
        return nullptr;
    }

    llvm::IRBuilder<>& builder = cg.getBuilder();

    if (op == "+") {
        return builder.CreateAdd(left_val, right_val, "addtmp");
    } else if (op == "-") {
        return builder.CreateSub(left_val, right_val, "subtmp");
    } else if (op == "*") {
        return builder.CreateMul(left_val, right_val, "multmp");
    } else if (op == "/") {
        return builder.CreateSDiv(left_val, right_val, "divtmp");
    } else if (op == "%") {
        return builder.CreateSRem(left_val, right_val, "remtmp");
    } else if (op == "==") {
        return builder.CreateICmpEQ(left_val, right_val, "eqtmp");
    } else if (op == "!=") {
        return builder.CreateICmpNE(left_val, right_val, "netmp");
    } else if (op == "<") {
        return builder.CreateICmpSLT(left_val, right_val, "lttmp");
    } else if (op == ">") {
        return builder.CreateICmpSGT(left_val, right_val, "gttmp");
    } else if (op == "<=") {
        return builder.CreateICmpSLE(left_val, right_val, "letmp");
    } else if (op == ">=") {
        return builder.CreateICmpSGE(left_val, right_val, "getmp");
    } else if (op == "&&") {
        return builder.CreateAnd(left_val, right_val, "andtmp");
    } else if (op == "||") {
        return builder.CreateOr(left_val, right_val, "ortmp");
    } else if (op == "&") {
        return builder.CreateAnd(left_val, right_val, "bitandtmp");
    } else if (op == "|") {
        return builder.CreateOr(left_val, right_val, "bitor_tmp");
    } else if (op == "^") {
        return builder.CreateXor(left_val, right_val, "xortmp");
    } else if (op == "<<") {
        return builder.CreateShl(left_val, right_val, "shltmp");
    } else if (op == ">>") {
        return builder.CreateAShr(left_val, right_val, "shrtmp");
    } else {
        return cg.logError("Unknown binary operator: " + op);
    }
}

void BinaryOpExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryOpExpression: " << op << std::endl;
    left->print(indent + 1);
    right->print(indent + 1);
}

// UnaryOpExpression implementation
llvm::Value* UnaryOpExpression::codegen(CodeGenerator& cg) {
    llvm::Value* operand_val = operand->codegen(cg);
    if (!operand_val) {
        return nullptr;
    }

    llvm::IRBuilder<>& builder = cg.getBuilder();

    if (op == "!") {
        return builder.CreateNot(operand_val, "nottmp");
    } else if (op == "~") {
        return builder.CreateNot(operand_val, "bitnottmp");
    } else if (op == "-") {
        return builder.CreateNeg(operand_val, "negtmp");
    } else {
        return cg.logError("Unknown unary operator: " + op);
    }
}

void UnaryOpExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryOpExpression: " << op << std::endl;
    operand->print(indent + 1);
}

// FunctionCallExpression implementation
llvm::Value* FunctionCallExpression::codegen(CodeGenerator& cg) {
    llvm::Function* function = cg.getFunction(function_name);
    if (!function) {
        return cg.logError("Unknown function referenced: " + function_name);
    }

    if (function->arg_size() != arguments->size()) {
        return cg.logError("Incorrect number of arguments passed to function " + function_name);
    }

    std::vector<llvm::Value*> args;
    for (auto arg : *arguments) {
        llvm::Value* arg_val = arg->codegen(cg);
        if (!arg_val) {
            return nullptr;
        }
        args.push_back(arg_val);
    }

    return cg.getBuilder().CreateCall(function, args, "calltmp");
}

void FunctionCallExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "FunctionCallExpression: " << function_name << std::endl;
    for (auto arg : *arguments) {
        arg->print(indent + 1);
    }
}

// AssignmentExpression implementation
llvm::Value* AssignmentExpression::codegen(CodeGenerator& cg) {
    // Special handling for variable assignments
    VariableExpression* var_expr = dynamic_cast<VariableExpression*>(target);
    if (!var_expr) {
        return cg.logError("Assignment target must be a variable");
    }

    llvm::Value* value_val = value->codegen(cg);
    if (!value_val) {
        return nullptr;
    }

    llvm::Value* var_ptr = cg.getNamedValue(var_expr->name);
    if (!var_ptr) {
        return cg.logError("Unknown variable name in assignment: " + var_expr->name);
    }

    return cg.getBuilder().CreateStore(value_val, var_ptr);
}

void AssignmentExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "AssignmentExpression:" << std::endl;
    target->print(indent + 1);
    value->print(indent + 1);
}

// VariableDeclaration implementation
llvm::Value* VariableDeclaration::codegen(CodeGenerator& cg) {
    llvm::Type* var_type = cg.getLLVMType(type);
    if (!var_type) {
        return cg.logError("Unknown type in variable declaration");
    }

    llvm::Function* current_func = cg.getCurrentFunction();
    llvm::IRBuilder<>& builder = cg.getBuilder();

    llvm::Value* var_alloca = builder.CreateAlloca(var_type, nullptr, name);
    cg.setNamedValue(name, var_alloca);

    if (initializer) {
        llvm::Value* init_val = initializer->codegen(cg);
        if (!init_val) {
            return nullptr;
        }
        builder.CreateStore(init_val, var_alloca);
    }

    return var_alloca;
}

void VariableDeclaration::print(int indent) const {
    printIndent(indent);
    std::cout << "VariableDeclaration: " << name << std::endl;
    type->print(indent + 1);
    if (initializer) {
        initializer->print(indent + 1);
    }
}

// FunctionDeclaration implementation
llvm::Value* FunctionDeclaration::codegen(CodeGenerator& cg) {
    // Function declarations don't generate code in the current context
    // They just register the function signature
    return nullptr;
}

void FunctionDeclaration::print(int indent) const {
    printIndent(indent);
    std::cout << "FunctionDeclaration: " << name << std::endl;
    return_type->print(indent + 1);
    for (auto param : *parameters) {
        printIndent(indent + 1);
        std::cout << "Parameter: " << *param << std::endl;
        delete param;
    }
}

// FunctionDefinition implementation
llvm::Value* FunctionDefinition::codegen(CodeGenerator& cg) {
    // Create function signature
    llvm::Type* return_type = cg.getLLVMType(return_type);
    if (!return_type) {
        return cg.logError("Unknown return type in function definition");
    }

    std::vector<llvm::Type*> param_types;
    for (auto param : *parameters) {
        // For simplicity, assume all parameters are int for now
        param_types.push_back(llvm::Type::getInt32Ty(cg.getContext()));
        delete param;
    }

    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);
    llvm::Function* function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, cg.getModule());

    cg.setFunction(name, function);

    // Create basic block for function body
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(cg.getContext(), "entry", function);
    cg.getBuilder().SetInsertPoint(entry);

    // Allocate parameters
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        llvm::Value* alloca = cg.getBuilder().CreateAlloca(llvm::Type::getInt32Ty(cg.getContext()), nullptr, arg.getName());
        cg.getBuilder().CreateStore(&arg, alloca);
        cg.setNamedValue(std::string(arg.getName()), alloca);
        idx++;
    }

    // Generate body
    llvm::Value* last = nullptr;
    for (auto stmt : *body) {
        last = stmt->codegen(cg);
    }

    // Add return statement if none exists and return type is not void
    if (!last && return_type != llvm::Type::getVoidTy(cg.getContext())) {
        if (return_type == llvm::Type::getInt32Ty(cg.getContext())) {
            cg.getBuilder().CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(cg.getContext()), 0));
        } else {
            cg.getBuilder().CreateRetVoid();
        }
    }

    // Verify function
    std::string error_str;
    llvm::raw_string_ostream error_stream(error_str);
    if (llvm::verifyFunction(*function, &error_stream)) {
        error_stream.flush();
        return cg.logError("Function verification failed: " + error_str);
    }

    return function;
}

void FunctionDefinition::print(int indent) const {
    printIndent(indent);
    std::cout << "FunctionDefinition: " << name << std::endl;
    return_type->print(indent + 1);
    for (auto param : *parameters) {
        printIndent(indent + 1);
        std::cout << "Parameter: " << *param << std::endl;
        delete param;
    }
    for (auto stmt : *body) {
        stmt->print(indent + 1);
    }
}

// ExternFunctionDeclaration implementation
llvm::Value* ExternFunctionDeclaration::codegen(CodeGenerator& cg) {
    llvm::Type* return_type = cg.getLLVMType(return_type);
    if (!return_type) {
        return cg.logError("Unknown return type in extern function declaration");
    }

    std::vector<llvm::Type*> param_types;
    for (auto param : *parameters) {
        param_types.push_back(llvm::Type::getInt32Ty(cg.getContext())); // Assume int for simplicity
        delete param;
    }

    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);
    llvm::Function* function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, cg.getModule());

    cg.setFunction(name, function);
    return function;
}

void ExternFunctionDeclaration::print(int indent) const {
    printIndent(indent);
    std::cout << "ExternFunctionDeclaration: " << name << std::endl;
    return_type->print(indent + 1);
    for (auto param : *parameters) {
        printIndent(indent + 1);
        std::cout << "Parameter: " << *param << std::endl;
        delete param;
    }
}

// ExpressionStatement implementation
llvm::Value* ExpressionStatement::codegen(CodeGenerator& cg) {
    return expression->codegen(cg);
}

void ExpressionStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ExpressionStatement:" << std::endl;
    expression->print(indent + 1);
}

// ReturnStatement implementation
llvm::Value* ReturnStatement::codegen(CodeGenerator& cg) {
    if (expression) {
        llvm::Value* ret_val = expression->codegen(cg);
        if (!ret_val) {
            return nullptr;
        }
        return cg.getBuilder().CreateRet(ret_val);
    } else {
        return cg.getBuilder().CreateRetVoid();
    }
}

void ReturnStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ReturnStatement:" << std::endl;
    if (expression) {
        expression->print(indent + 1);
    }
}

// IfStatement implementation
llvm::Value* IfStatement::codegen(CodeGenerator& cg) {
    llvm::Value* cond_val = condition->codegen(cg);
    if (!cond_val) {
        return nullptr;
    }

    llvm::Function* current_func = cg.getCurrentFunction();
    llvm::IRBuilder<>& builder = cg.getBuilder();

    // Convert condition to bool if needed
    cond_val = builder.CreateICmpNE(cond_val, llvm::ConstantInt::get(cond_val->getType(), 0), "ifcond");

    // Create basic blocks
    llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(cg.getContext(), "then", current_func);
    llvm::BasicBlock* else_bb = else_branch ? llvm::BasicBlock::Create(cg.getContext(), "else", current_func) : nullptr;
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(cg.getContext(), "ifcont", current_func);

    // Create conditional branch
    builder.CreateCondBr(cond_val, then_bb, else_branch ? else_bb : merge_bb);

    // Generate then block
    builder.SetInsertPoint(then_bb);
    llvm::Value* then_val = then_branch->codegen(cg);
    if (!then_val) {
        return nullptr;
    }
    builder.CreateBr(merge_bb);
    then_bb = builder.GetInsertBlock();

    llvm::BasicBlock* else_end_bb = nullptr;
    if (else_branch) {
        // Generate else block
        builder.SetInsertPoint(else_bb);
        llvm::Value* else_val = else_branch->codegen(cg);
        if (!else_val) {
            return nullptr;
        }
        builder.CreateBr(merge_bb);
        else_end_bb = builder.GetInsertBlock();
    }

    // Continue in merge block
    builder.SetInsertPoint(merge_bb);

    return cond_val;
}

void IfStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "IfStatement:" << std::endl;
    condition->print(indent + 1);
    then_branch->print(indent + 1);
    if (else_branch) {
        else_branch->print(indent + 1);
    }
}

// WhileStatement implementation
llvm::Value* WhileStatement::codegen(CodeGenerator& cg) {
    llvm::Function* current_func = cg.getCurrentFunction();
    llvm::IRBuilder<>& builder = cg.getBuilder();

    // Create basic blocks
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(cg.getContext(), "whilecond", current_func);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(cg.getContext(), "whilebody", current_func);
    llvm::BasicBlock* end_bb = llvm::BasicBlock::Create(cg.getContext(), "whileend", current_func);

    // Push loop context
    cg.pushLoopBlock(end_bb);

    // Jump to condition check
    builder.CreateBr(cond_bb);

    // Generate condition
    builder.SetInsertPoint(cond_bb);
    llvm::Value* cond_val = condition->codegen(cg);
    if (!cond_val) {
        return nullptr;
    }
    cond_val = builder.CreateICmpNE(cond_val, llvm::ConstantInt::get(cond_val->getType(), 0), "whilecond");
    builder.CreateCondBr(cond_val, body_bb, end_bb);

    // Generate body
    builder.SetInsertPoint(body_bb);
    llvm::Value* body_val = body->codegen(cg);
    if (!body_val) {
        return nullptr;
    }
    builder.CreateBr(cond_bb);

    // Pop loop context and set insert point
    cg.popLoopBlock();
    builder.SetInsertPoint(end_bb);

    return cond_val;
}

void WhileStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "WhileStatement:" << std::endl;
    condition->print(indent + 1);
    body->print(indent + 1);
}

// ForStatement implementation (simplified)
llvm::Value* ForStatement::codegen(CodeGenerator& cg) {
    llvm::Function* current_func = cg.getCurrentFunction();
    llvm::IRBuilder<>& builder = cg.getBuilder();

    // Generate init expression
    if (init) {
        init->codegen(cg);
    }

    // Create basic blocks
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(cg.getContext(), "forcond", current_func);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(cg.getContext(), "forbody", current_func);
    llvm::BasicBlock* inc_bb = llvm::BasicBlock::Create(cg.getContext(), "forinc", current_func);
    llvm::BasicBlock* end_bb = llvm::BasicBlock::Create(cg.getContext(), "forend", current_func);

    // Jump to condition check
    builder.CreateBr(cond_bb);

    // Generate condition
    builder.SetInsertPoint(cond_bb);
    llvm::Value* cond_val = nullptr;
    if (condition) {
        cond_val = condition->codegen(cg);
        if (!cond_val) {
            return nullptr;
        }
        cond_val = builder.CreateICmpNE(cond_val, llvm::ConstantInt::get(cond_val->getType(), 0), "forcond");
    } else {
        cond_val = llvm::ConstantInt::get(llvm::Type::getInt1Ty(cg.getContext()), 1); // true
    }
    builder.CreateCondBr(cond_val, body_bb, end_bb);

    // Generate body
    builder.SetInsertPoint(body_bb);
    llvm::Value* body_val = body->codegen(cg);
    if (!body_val) {
        return nullptr;
    }
    builder.CreateBr(inc_bb);

    // Generate increment
    builder.SetInsertPoint(inc_bb);
    if (increment) {
        increment->codegen(cg);
    }
    builder.CreateBr(cond_bb);

    // Set insert point to end
    builder.SetInsertPoint(end_bb);

    return cond_val;
}

void ForStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ForStatement:" << std::endl;
    if (init) init->print(indent + 1);
    if (condition) condition->print(indent + 1);
    if (increment) increment->print(indent + 1);
    body->print(indent + 1);
}

// BlockStatement implementation
llvm::Value* BlockStatement::codegen(CodeGenerator& cg) {
    llvm::Value* last = nullptr;
    for (auto stmt : *statements) {
        last = stmt->codegen(cg);
    }
    return last;
}

void BlockStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "BlockStatement:" << std::endl;
    for (auto stmt : *statements) {
        stmt->print(indent + 1);
    }
}
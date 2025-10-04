#include "ast.h"
#include "ir_generator.h"
#include <iostream>

namespace clike {

// Expression implementations
void NumberExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* NumberExpr::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void VariableExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* VariableExpr::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void BinaryExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* BinaryExpr::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void CallExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* CallExpr::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

// Statement implementations
void ExprStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* ExprStmt::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void VarDeclStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* VarDeclStmt::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void IfStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* IfStmt::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void WhileStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* WhileStmt::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void ReturnStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* ReturnStmt::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

void BlockStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Value* BlockStmt::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

// Function definition implementation
void FunctionDef::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Function* FunctionDef::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

// Program implementation
void Program::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

llvm::Module* Program::codegen(IRGenerator& generator) {
    return generator.codegen(*this);
}

} // namespace clike
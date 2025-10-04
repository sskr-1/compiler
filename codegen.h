#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <map>
#include <stack>
#include <memory>
#include "ast_nodes.h"

// LLVM includes
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

class CodeGenerator {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    
    // Symbol table for variables
    std::map<std::string, llvm::Value*> named_values;
    
    // Stack for nested scopes
    std::stack<std::map<std::string, llvm::Value*>> scope_stack;
    
    // Control flow stack for break/continue
    std::stack<llvm::BasicBlock*> break_stack;
    std::stack<llvm::BasicBlock*> continue_stack;
    
    // Current function being generated
    llvm::Function* current_function;
    
    // Helper methods
    void enterScope();
    void exitScope();
    llvm::Value* getVariable(const std::string& name);
    void setVariable(const std::string& name, llvm::Value* value);
    llvm::Function* getFunction(const std::string& name);
    llvm::Type* getLLVMType(const std::string& type_name);
    llvm::Value* createCast(llvm::Value* value, llvm::Type* target_type);

public:
    CodeGenerator();
    ~CodeGenerator();
    
    // Initialize LLVM
    void initialize();
    
    // Generate LLVM IR for different AST nodes
    llvm::Value* codegenFunctionDefinition(FunctionDefinitionNode* node);
    llvm::Value* codegenVariableDeclaration(VariableDeclarationNode* node);
    llvm::Value* codegenCompoundStatement(CompoundStatementNode* node);
    llvm::Value* codegenExpressionStatement(ExpressionStatementNode* node);
    llvm::Value* codegenIfStatement(IfStatementNode* node);
    llvm::Value* codegenWhileStatement(WhileStatementNode* node);
    llvm::Value* codegenForStatement(ForStatementNode* node);
    llvm::Value* codegenReturnStatement(ReturnStatementNode* node);
    llvm::Value* codegenBreakStatement(BreakStatementNode* node);
    llvm::Value* codegenContinueStatement(ContinueStatementNode* node);
    llvm::Value* codegenAssignment(AssignmentNode* node);
    llvm::Value* codegenBinaryExpression(BinaryExpressionNode* node);
    llvm::Value* codegenUnaryExpression(UnaryExpressionNode* node);
    llvm::Value* codegenIdentifier(IdentifierNode* node);
    llvm::Value* codegenIntegerLiteral(IntegerLiteralNode* node);
    llvm::Value* codegenFloatLiteral(FloatLiteralNode* node);
    llvm::Value* codegenCharLiteral(CharLiteralNode* node);
    llvm::Value* codegenStringLiteral(StringLiteralNode* node);
    llvm::Value* codegenBooleanLiteral(BooleanLiteralNode* node);
    llvm::Value* codegenFunctionCall(FunctionCallNode* node);
    llvm::Value* codegenArrayAccess(ArrayAccessNode* node);
    
    // Utility methods
    void printIR();
    void optimizeIR();
    bool verifyModule();
    
    // Getters
    llvm::Module* getModule() const { return module.get(); }
    llvm::LLVMContext* getContext() const { return context.get(); }
};

#endif // CODEGEN_H
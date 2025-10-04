#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

class CodeGenContext;

// Forward declare LLVM types we'll use as pointers
namespace llvm {
    class Value;
    class Function;
}

// Base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value* codegen(CodeGenContext& context) = 0;
};

// Expression base class
class ExprAST : public ASTNode {
public:
    virtual ~ExprAST() = default;
};

// Statement base class
class StmtAST : public ASTNode {
public:
    virtual ~StmtAST() = default;
};

// Number literal expression
class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(double Val) : Val(Val) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Variable reference expression
class VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string& Name) : Name(Name) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Binary operation expression
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Function call expression
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string& Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Function prototype
class PrototypeAST : public ASTNode {
    std::string Name;
    std::vector<std::string> Args;
    std::string ReturnType;
public:
    PrototypeAST(const std::string& Name, std::vector<std::string> Args, const std::string& ReturnType = "double")
        : Name(Name), Args(std::move(Args)), ReturnType(ReturnType) {}
    
    llvm::Function* codegenFunction(CodeGenContext& context);
    const std::string& getName() const { return Name; }
    const std::vector<std::string>& getArgs() const { return Args; }
    const std::string& getReturnType() const { return ReturnType; }
    
    // Override the base codegen method - return the function as a Value*
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Variable declaration statement
class VarDeclAST : public StmtAST {
    std::string Name;
    std::string Type;
    std::unique_ptr<ExprAST> InitVal;
public:
    VarDeclAST(const std::string& Name, const std::string& Type, std::unique_ptr<ExprAST> InitVal = nullptr)
        : Name(Name), Type(Type), InitVal(std::move(InitVal)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Assignment statement
class AssignmentAST : public StmtAST {
    std::string Name;
    std::unique_ptr<ExprAST> Value;
public:
    AssignmentAST(const std::string& Name, std::unique_ptr<ExprAST> Value)
        : Name(Name), Value(std::move(Value)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// If statement
class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<StmtAST> Then, Else;
public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtAST> Then,
              std::unique_ptr<StmtAST> Else = nullptr)
        : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Block statement (compound statement)
class BlockAST : public StmtAST {
    std::vector<std::unique_ptr<StmtAST>> Statements;
public:
    BlockAST(std::vector<std::unique_ptr<StmtAST>> Statements)
        : Statements(std::move(Statements)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Function definition
class FunctionAST : public ASTNode {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<StmtAST> Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<StmtAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
    llvm::Function* codegenFunction(CodeGenContext& context);
    
    // Override the base codegen method - return the function as a Value*
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Return statement
class ReturnAST : public StmtAST {
    std::unique_ptr<ExprAST> RetVal;
public:
    ReturnAST(std::unique_ptr<ExprAST> RetVal = nullptr) : RetVal(std::move(RetVal)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

// Expression statement (for expressions used as statements)
class ExpressionStmtAST : public StmtAST {
    std::unique_ptr<ExprAST> Expr;
public:
    ExpressionStmtAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};
#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>

// Forward declarations for LLVM types
namespace llvm {
    class Value;
    class Type;
    class Module;
    class LLVMContext;
    class IRBuilder;
    class Function;
}

class CodeGenerator;

// Base AST Node class
class ASTNode {
public:
    int line_number;
    ASTNode(int line) : line_number(line) {}
    virtual ~ASTNode() = default;

    // Pure virtual methods for code generation
    virtual llvm::Value* codegen(CodeGenerator& cg) = 0;
    virtual void print(int indent = 0) const = 0;
};

// Type representation
class Type : public ASTNode {
public:
    std::string type_name;
    Type(std::string name, int line) : ASTNode(line), type_name(name) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
    llvm::Type* getLLVMType(CodeGenerator& cg);
};

// Expression base class
class Expression : public ASTNode {
public:
    Expression(int line) : ASTNode(line) {}
};

// Statement base class
class Statement : public ASTNode {
public:
    Statement(int line) : ASTNode(line) {}
};

// Declaration base class
class Declaration : public ASTNode {
public:
    Declaration(int line) : ASTNode(line) {}
};

// Program class (top-level)
class Program : public ASTNode {
public:
    std::vector<Declaration*>* declarations;
    Program(std::vector<Declaration*>* decls) : ASTNode(0), declarations(decls) {}
    ~Program() { delete declarations; }

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Literal expressions
class IntegerLiteral : public Expression {
public:
    int value;
    IntegerLiteral(int val, int line) : Expression(line), value(val) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class FloatLiteral : public Expression {
public:
    float value;
    FloatLiteral(float val, int line) : Expression(line), value(val) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class StringLiteral : public Expression {
public:
    std::string value;
    StringLiteral(std::string val, int line) : Expression(line), value(val) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Variable expression
class VariableExpression : public Expression {
public:
    std::string name;
    VariableExpression(std::string var_name, int line) : Expression(line), name(var_name) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Binary operation expression
class BinaryOpExpression : public Expression {
public:
    Expression* left;
    std::string op;
    Expression* right;
    BinaryOpExpression(Expression* l, std::string oper, Expression* r, int line)
        : Expression(line), left(l), op(oper), right(r) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Unary operation expression
class UnaryOpExpression : public Expression {
public:
    std::string op;
    Expression* operand;
    UnaryOpExpression(std::string oper, Expression* expr, int line)
        : Expression(line), op(oper), operand(expr) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Function call expression
class FunctionCallExpression : public Expression {
public:
    std::string function_name;
    std::vector<Expression*>* arguments;
    FunctionCallExpression(std::string fname, std::vector<Expression*>* args, int line)
        : Expression(line), function_name(fname), arguments(args) {}
    ~FunctionCallExpression() { delete arguments; }

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Assignment expression
class AssignmentExpression : public Expression {
public:
    Expression* target;
    Expression* value;
    AssignmentExpression(Expression* tgt, Expression* val, int line)
        : Expression(line), target(tgt), value(val) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Variable declaration
class VariableDeclaration : public Declaration {
public:
    Type* type;
    std::string name;
    Expression* initializer;
    VariableDeclaration(Type* t, std::string var_name, int line)
        : Declaration(line), type(t), name(var_name), initializer(nullptr) {}
    VariableDeclaration(Type* t, std::string var_name, Expression* init, int line)
        : Declaration(line), type(t), name(var_name), initializer(init) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Function declaration
class FunctionDeclaration : public Declaration {
public:
    Type* return_type;
    std::string name;
    std::vector<std::string*>* parameters;
    FunctionDeclaration(Type* ret_type, std::string fname, std::vector<std::string*>* params, int line)
        : Declaration(line), return_type(ret_type), name(fname), parameters(params) {}
    ~FunctionDeclaration() {
        for (auto param : *parameters) delete param;
        delete parameters;
    }

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Function definition
class FunctionDefinition : public Declaration {
public:
    Type* return_type;
    std::string name;
    std::vector<std::string*>* parameters;
    std::vector<Statement*>* body;
    FunctionDefinition(Type* ret_type, std::string fname, std::vector<std::string*>* params,
                      std::vector<Statement*>* func_body, int line)
        : Declaration(line), return_type(ret_type), name(fname), parameters(params), body(func_body) {}
    ~FunctionDefinition() {
        for (auto param : *parameters) delete param;
        delete parameters;
        delete body;
    }

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// External function declaration
class ExternFunctionDeclaration : public Declaration {
public:
    Type* return_type;
    std::string name;
    std::vector<std::string*>* parameters;
    ExternFunctionDeclaration(Type* ret_type, std::string fname, std::vector<std::string*>* params, int line)
        : Declaration(line), return_type(ret_type), name(fname), parameters(params) {}
    ~ExternFunctionDeclaration() {
        for (auto param : *parameters) delete param;
        delete parameters;
    }

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

// Statement classes
class ExpressionStatement : public Statement {
public:
    Expression* expression;
    ExpressionStatement(Expression* expr, int line) : Statement(line), expression(expr) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class ReturnStatement : public Statement {
public:
    Expression* expression;
    ReturnStatement(Expression* expr, int line) : Statement(line), expression(expr) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class IfStatement : public Statement {
public:
    Expression* condition;
    Statement* then_branch;
    Statement* else_branch;
    IfStatement(Expression* cond, Statement* then_stmt, Statement* else_stmt, int line)
        : Statement(line), condition(cond), then_branch(then_stmt), else_branch(else_stmt) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class WhileStatement : public Statement {
public:
    Expression* condition;
    Statement* body;
    WhileStatement(Expression* cond, Statement* stmt, int line)
        : Statement(line), condition(cond), body(stmt) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class ForStatement : public Statement {
public:
    Expression* init;
    Expression* condition;
    Expression* increment;
    Statement* body;
    ForStatement(Expression* init_expr, Expression* cond_expr, Expression* inc_expr, Statement* stmt, int line)
        : Statement(line), init(init_expr), condition(cond_expr), increment(inc_expr), body(stmt) {}

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

class BlockStatement : public Statement {
public:
    std::vector<Statement*>* statements;
    BlockStatement(std::vector<Statement*>* stmts, int line) : Statement(line), statements(stmts) {}
    ~BlockStatement() { delete statements; }

    llvm::Value* codegen(CodeGenerator& cg) override;
    void print(int indent = 0) const override;
};

#endif // AST_H
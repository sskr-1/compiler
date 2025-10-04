#pragma once

#include <memory>
#include <vector>
#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace clike {

// Forward declarations
class ASTVisitor;
class IRGenerator;

// Base AST node class
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Expression nodes
class Expression : public ASTNode {
public:
    virtual llvm::Value* codegen(IRGenerator& generator) = 0;
};

class NumberExpr : public Expression {
public:
    NumberExpr(double value) : value_(value) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    double getValue() const { return value_; }

private:
    double value_;
};

class VariableExpr : public Expression {
public:
    VariableExpr(const std::string& name) : name_(name) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
};

class BinaryExpr : public Expression {
public:
    BinaryExpr(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : op_(op), left_(std::move(left)), right_(std::move(right)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    char getOp() const { return op_; }
    Expression* getLeft() const { return left_.get(); }
    Expression* getRight() const { return right_.get(); }

private:
    char op_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

class CallExpr : public Expression {
public:
    CallExpr(const std::string& callee, std::vector<std::unique_ptr<Expression>> args)
        : callee_(callee), args_(std::move(args)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    const std::string& getCallee() const { return callee_; }
    const std::vector<std::unique_ptr<Expression>>& getArgs() const { return args_; }

private:
    std::string callee_;
    std::vector<std::unique_ptr<Expression>> args_;
};

// Statement nodes
class Statement : public ASTNode {
public:
    virtual llvm::Value* codegen(IRGenerator& generator) = 0;
};

class ExprStmt : public Statement {
public:
    ExprStmt(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    Expression* getExpr() const { return expr_.get(); }

private:
    std::unique_ptr<Expression> expr_;
};

class VarDeclStmt : public Statement {
public:
    VarDeclStmt(const std::string& name, std::unique_ptr<Expression> init)
        : name_(name), init_(std::move(init)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    const std::string& getName() const { return name_; }
    Expression* getInit() const { return init_.get(); }

private:
    std::string name_;
    std::unique_ptr<Expression> init_;
};

class IfStmt : public Statement {
public:
    IfStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenStmt, 
           std::unique_ptr<Statement> elseStmt = nullptr)
        : condition_(std::move(condition)), thenStmt_(std::move(thenStmt)), 
          elseStmt_(std::move(elseStmt)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    Expression* getCondition() const { return condition_.get(); }
    Statement* getThenStmt() const { return thenStmt_.get(); }
    Statement* getElseStmt() const { return elseStmt_.get(); }

private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Statement> thenStmt_;
    std::unique_ptr<Statement> elseStmt_;
};

class WhileStmt : public Statement {
public:
    WhileStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : condition_(std::move(condition)), body_(std::move(body)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    Expression* getCondition() const { return condition_.get(); }
    Statement* getBody() const { return body_.get(); }

private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Statement> body_;
};

class ReturnStmt : public Statement {
public:
    ReturnStmt(std::unique_ptr<Expression> expr = nullptr) : expr_(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    Expression* getExpr() const { return expr_.get(); }

private:
    std::unique_ptr<Expression> expr_;
};

class BlockStmt : public Statement {
public:
    BlockStmt(std::vector<std::unique_ptr<Statement>> statements)
        : statements_(std::move(statements)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Value* codegen(IRGenerator& generator) override;
    
    const std::vector<std::unique_ptr<Statement>>& getStatements() const { return statements_; }

private:
    std::vector<std::unique_ptr<Statement>> statements_;
};

// Function definition
class FunctionDef : public ASTNode {
public:
    FunctionDef(const std::string& name, std::vector<std::string> args, 
                std::unique_ptr<Statement> body)
        : name_(name), args_(std::move(args)), body_(std::move(body)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Function* codegen(IRGenerator& generator);
    
    const std::string& getName() const { return name_; }
    const std::vector<std::string>& getArgs() const { return args_; }
    Statement* getBody() const { return body_.get(); }

private:
    std::string name_;
    std::vector<std::string> args_;
    std::unique_ptr<Statement> body_;
};

// Program (top-level)
class Program : public ASTNode {
public:
    Program(std::vector<std::unique_ptr<FunctionDef>> functions)
        : functions_(std::move(functions)) {}
    
    void accept(ASTVisitor& visitor) override;
    llvm::Module* codegen(IRGenerator& generator);
    
    const std::vector<std::unique_ptr<FunctionDef>>& getFunctions() const { return functions_; }

private:
    std::vector<std::unique_ptr<FunctionDef>> functions_;
};

// Visitor pattern for AST traversal
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual void visit(NumberExpr& node) = 0;
    virtual void visit(VariableExpr& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(VarDeclStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(BlockStmt& node) = 0;
    virtual void visit(FunctionDef& node) = 0;
    virtual void visit(Program& node) = 0;
};

} // namespace clike
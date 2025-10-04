#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <map>

// Forward declarations for LLVM
namespace llvm {
    class Value;
}

// Forward declarations
class CodeGenContext;

// Base class for all AST nodes
class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0;
};

// Base class for expressions
class ExprNode : public Node {
};

// Base class for statements
class StmtNode : public Node {
};

// Integer literal
class IntegerNode : public ExprNode {
public:
    long long value;
    IntegerNode(long long value) : value(value) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Float literal
class DoubleNode : public ExprNode {
public:
    double value;
    DoubleNode(double value) : value(value) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Identifier (variable reference)
class IdentifierNode : public ExprNode {
public:
    std::string name;
    IdentifierNode(const std::string& name) : name(name) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Binary operation
class BinaryOpNode : public ExprNode {
public:
    int op; // '+', '-', '*', '/', '<', '>', '==', '!=', etc.
    std::unique_ptr<ExprNode> lhs;
    std::unique_ptr<ExprNode> rhs;
    
    BinaryOpNode(std::unique_ptr<ExprNode> lhs, int op, std::unique_ptr<ExprNode> rhs)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Unary operation
class UnaryOpNode : public ExprNode {
public:
    int op; // '-', '!'
    std::unique_ptr<ExprNode> expr;
    
    UnaryOpNode(int op, std::unique_ptr<ExprNode> expr)
        : op(op), expr(std::move(expr)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Function call
class FunctionCallNode : public ExprNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ExprNode>> args;
    
    FunctionCallNode(const std::string& name) : name(name) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Variable declaration
class VariableDeclNode : public StmtNode {
public:
    std::string type;
    std::string name;
    std::unique_ptr<ExprNode> initExpr;
    
    VariableDeclNode(const std::string& type, const std::string& name, 
                     std::unique_ptr<ExprNode> initExpr = nullptr)
        : type(type), name(name), initExpr(std::move(initExpr)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Assignment
class AssignmentNode : public ExprNode {
public:
    std::string name;
    std::unique_ptr<ExprNode> expr;
    
    AssignmentNode(const std::string& name, std::unique_ptr<ExprNode> expr)
        : name(name), expr(std::move(expr)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Block (list of statements)
class BlockNode : public StmtNode {
public:
    std::vector<std::unique_ptr<StmtNode>> statements;
    
    BlockNode() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// If statement
class IfStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> thenBlock;
    std::unique_ptr<BlockNode> elseBlock;
    
    IfStmtNode(std::unique_ptr<ExprNode> condition,
               std::unique_ptr<BlockNode> thenBlock,
               std::unique_ptr<BlockNode> elseBlock = nullptr)
        : condition(std::move(condition)), 
          thenBlock(std::move(thenBlock)), 
          elseBlock(std::move(elseBlock)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// While statement
class WhileStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> body;
    
    WhileStmtNode(std::unique_ptr<ExprNode> condition,
                  std::unique_ptr<BlockNode> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Return statement
class ReturnStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expr;
    
    ReturnStmtNode(std::unique_ptr<ExprNode> expr = nullptr)
        : expr(std::move(expr)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Expression statement
class ExprStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expr;
    
    ExprStmtNode(std::unique_ptr<ExprNode> expr)
        : expr(std::move(expr)) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Function parameter
class ParamNode {
public:
    std::string type;
    std::string name;
    
    ParamNode(const std::string& type, const std::string& name)
        : type(type), name(name) {}
};

// Function declaration
class FunctionNode : public Node {
public:
    std::string returnType;
    std::string name;
    std::vector<std::unique_ptr<ParamNode>> params;
    std::unique_ptr<BlockNode> body;
    
    FunctionNode(const std::string& returnType, const std::string& name)
        : returnType(returnType), name(name) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Program (root node)
class ProgramNode : public Node {
public:
    std::vector<std::unique_ptr<FunctionNode>> functions;
    
    ProgramNode() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

#endif // AST_H

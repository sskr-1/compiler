#ifndef AST_NODES_H
#define AST_NODES_H

#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace llvm {
    class Value;
    class Type;
    class Function;
    class BasicBlock;
    class Module;
    class LLVMContext;
}

class CodeGenerator;

// Base AST Node class
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value* codegen(CodeGenerator& gen) = 0;
    virtual void print(int indent = 0) const = 0;
};

// Program node - root of AST
class ProgramNode : public ASTNode {
private:
    std::vector<ASTNode*>* declarations;

public:
    ProgramNode(std::vector<ASTNode*>* decls) : declarations(decls) {}
    ~ProgramNode() override {
        if (declarations) {
            for (auto* decl : *declarations) {
                delete decl;
            }
            delete declarations;
        }
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
};

// Type node
class TypeNode : public ASTNode {
private:
    std::string type_name;

public:
    TypeNode(const std::string& name) : type_name(name) {}
    
    const std::string& getName() const { return type_name; }
    llvm::Type* getLLVMType(llvm::LLVMContext& context) const;
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
};

// Function definition node
class FunctionDefinitionNode : public ASTNode {
private:
    TypeNode* return_type;
    std::string name;
    std::vector<ASTNode*>* parameters;
    ASTNode* body;

public:
    FunctionDefinitionNode(TypeNode* ret_type, const std::string& func_name, 
                          std::vector<ASTNode*>* params, ASTNode* func_body)
        : return_type(ret_type), name(func_name), parameters(params), body(func_body) {}
    
    ~FunctionDefinitionNode() override {
        delete return_type;
        if (parameters) {
            for (auto* param : *parameters) {
                delete param;
            }
            delete parameters;
        }
        delete body;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getName() const { return name; }
    TypeNode* getReturnType() const { return return_type; }
    std::vector<ASTNode*>* getParameters() const { return parameters; }
    ASTNode* getBody() const { return body; }
};

// Parameter node
class ParameterNode : public ASTNode {
private:
    TypeNode* type;
    std::string name;

public:
    ParameterNode(TypeNode* param_type, const std::string& param_name)
        : type(param_type), name(param_name) {}
    
    ~ParameterNode() override {
        delete type;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    TypeNode* getType() const { return type; }
    const std::string& getName() const { return name; }
};

// Parameter list node
class ParameterListNode : public ASTNode {
private:
    std::vector<ASTNode*>* parameters;

public:
    ParameterListNode(std::vector<ASTNode*>* params) : parameters(params) {}
    
    ~ParameterListNode() override {
        if (parameters) {
            for (auto* param : *parameters) {
                delete param;
            }
            delete parameters;
        }
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
};

// Variable declaration node
class VariableDeclarationNode : public ASTNode {
private:
    TypeNode* type;
    std::string name;
    ASTNode* initializer;

public:
    VariableDeclarationNode(TypeNode* var_type, const std::string& var_name, ASTNode* init)
        : type(var_type), name(var_name), initializer(init) {}
    
    ~VariableDeclarationNode() override {
        delete type;
        delete initializer;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    TypeNode* getType() const { return type; }
    const std::string& getName() const { return name; }
    ASTNode* getInitializer() const { return initializer; }
};

// Compound statement node
class CompoundStatementNode : public ASTNode {
private:
    std::vector<ASTNode*>* statements;

public:
    CompoundStatementNode(std::vector<ASTNode*>* stmts) : statements(stmts) {}
    
    ~CompoundStatementNode() override {
        if (statements) {
            for (auto* stmt : *statements) {
                delete stmt;
            }
            delete statements;
        }
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    std::vector<ASTNode*>* getStatements() const { return statements; }
};

// Expression statement node
class ExpressionStatementNode : public ASTNode {
private:
    ASTNode* expression;

public:
    ExpressionStatementNode(ASTNode* expr) : expression(expr) {}
    
    ~ExpressionStatementNode() override {
        delete expression;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    ASTNode* getExpression() const { return expression; }
};

// If statement node
class IfStatementNode : public ASTNode {
private:
    ASTNode* condition;
    ASTNode* then_stmt;
    ASTNode* else_stmt;

public:
    IfStatementNode(ASTNode* cond, ASTNode* then, ASTNode* else_st)
        : condition(cond), then_stmt(then), else_stmt(else_st) {}
    
    ~IfStatementNode() override {
        delete condition;
        delete then_stmt;
        delete else_stmt;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    ASTNode* getCondition() const { return condition; }
    ASTNode* getThenStatement() const { return then_stmt; }
    ASTNode* getElseStatement() const { return else_stmt; }
};

// While statement node
class WhileStatementNode : public ASTNode {
private:
    ASTNode* condition;
    ASTNode* body;

public:
    WhileStatementNode(ASTNode* cond, ASTNode* loop_body)
        : condition(cond), body(loop_body) {}
    
    ~WhileStatementNode() override {
        delete condition;
        delete body;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    ASTNode* getCondition() const { return condition; }
    ASTNode* getBody() const { return body; }
};

// For statement node
class ForStatementNode : public ASTNode {
private:
    ASTNode* init;
    ASTNode* condition;
    ASTNode* increment;
    ASTNode* body;

public:
    ForStatementNode(ASTNode* init_stmt, ASTNode* cond, ASTNode* inc, ASTNode* loop_body)
        : init(init_stmt), condition(cond), increment(inc), body(loop_body) {}
    
    ~ForStatementNode() override {
        delete init;
        delete condition;
        delete increment;
        delete body;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    ASTNode* getInit() const { return init; }
    ASTNode* getCondition() const { return condition; }
    ASTNode* getIncrement() const { return increment; }
    ASTNode* getBody() const { return body; }
};

// Return statement node
class ReturnStatementNode : public ASTNode {
private:
    ASTNode* expression;

public:
    ReturnStatementNode(ASTNode* expr) : expression(expr) {}
    
    ~ReturnStatementNode() override {
        delete expression;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
};

// Break statement node
class BreakStatementNode : public ASTNode {
public:
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
};

// Continue statement node
class ContinueStatementNode : public ASTNode {
public:
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    ASTNode* getExpression() const { return expression; }
};

// Assignment node
class AssignmentNode : public ASTNode {
private:
    ASTNode* lhs;
    ASTNode* rhs;

public:
    AssignmentNode(ASTNode* left, ASTNode* right) : lhs(left), rhs(right) {}
    
    ~AssignmentNode() override {
        delete lhs;
        delete rhs;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    ASTNode* getLHS() const { return lhs; }
    ASTNode* getRHS() const { return rhs; }
};

// Binary expression node
class BinaryExpressionNode : public ASTNode {
private:
    std::string op;
    ASTNode* left;
    ASTNode* right;

public:
    BinaryExpressionNode(const std::string& operation, ASTNode* l, ASTNode* r)
        : op(operation), left(l), right(r) {}
    
    ~BinaryExpressionNode() override {
        delete left;
        delete right;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getOperator() const { return op; }
    ASTNode* getLeft() const { return left; }
    ASTNode* getRight() const { return right; }
};

// Unary expression node
class UnaryExpressionNode : public ASTNode {
private:
    std::string op;
    ASTNode* operand;
    bool postfix;

public:
    UnaryExpressionNode(const std::string& operation, ASTNode* op_node, bool is_postfix = false)
        : op(operation), operand(op_node), postfix(is_postfix) {}
    
    ~UnaryExpressionNode() override {
        delete operand;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getOperator() const { return op; }
    ASTNode* getOperand() const { return operand; }
    bool isPostfix() const { return postfix; }
};

// Identifier node
class IdentifierNode : public ASTNode {
private:
    std::string name;

public:
    IdentifierNode(const std::string& var_name) : name(var_name) {}
    
    const std::string& getName() const { return name; }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getName() const { return name; }
};

// Integer literal node
class IntegerLiteralNode : public ASTNode {
private:
    int value;

public:
    IntegerLiteralNode(int val) : value(val) {}
    
    int getValue() const { return value; }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    int getValue() const { return value; }
};

// Float literal node
class FloatLiteralNode : public ASTNode {
private:
    float value;

public:
    FloatLiteralNode(float val) : value(val) {}
    
    float getValue() const { return value; }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    float getValue() const { return value; }
};

// Char literal node
class CharLiteralNode : public ASTNode {
private:
    char value;

public:
    CharLiteralNode(char val) : value(val) {}
    
    char getValue() const { return value; }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    char getValue() const { return value; }
};

// String literal node
class StringLiteralNode : public ASTNode {
private:
    std::string value;

public:
    StringLiteralNode(const std::string& val) : value(val) {}
    
    const std::string& getValue() const { return value; }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getValue() const { return value; }
};

// Boolean literal node
class BooleanLiteralNode : public ASTNode {
private:
    bool value;

public:
    BooleanLiteralNode(bool val) : value(val) {}
    
    bool getValue() const { return value; }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    bool getValue() const { return value; }
};

// Function call node
class FunctionCallNode : public ASTNode {
private:
    std::string name;
    std::vector<ASTNode*>* arguments;

public:
    FunctionCallNode(const std::string& func_name, std::vector<ASTNode*>* args)
        : name(func_name), arguments(args) {}
    
    ~FunctionCallNode() override {
        if (arguments) {
            for (auto* arg : *arguments) {
                delete arg;
            }
            delete arguments;
        }
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getName() const { return name; }
    std::vector<ASTNode*>* getArguments() const { return arguments; }
};

// Array access node
class ArrayAccessNode : public ASTNode {
private:
    std::string name;
    ASTNode* index;

public:
    ArrayAccessNode(const std::string& array_name, ASTNode* idx)
        : name(array_name), index(idx) {}
    
    ~ArrayAccessNode() override {
        delete index;
    }
    
    llvm::Value* codegen(CodeGenerator& gen) override;
    void print(int indent = 0) const override;
    
    // Getters
    const std::string& getName() const { return name; }
    ASTNode* getIndex() const { return index; }
};

#endif // AST_NODES_H
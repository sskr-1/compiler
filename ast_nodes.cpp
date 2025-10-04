#include "ast_nodes.h"
#include "codegen.h"
#include <iostream>
#include <iomanip>

// ProgramNode implementation
llvm::Value* ProgramNode::codegen(CodeGenerator& gen) {
    llvm::Value* result = nullptr;
    if (declarations) {
        for (auto* decl : *declarations) {
            result = decl->codegen(gen);
        }
    }
    return result;
}

void ProgramNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "Program\n";
    if (declarations) {
        for (auto* decl : *declarations) {
            decl->print(indent + 2);
        }
    }
}

// TypeNode implementation
llvm::Type* TypeNode::getLLVMType(llvm::LLVMContext& context) const {
    if (type_name == "int") {
        return llvm::Type::getInt32Ty(context);
    } else if (type_name == "float") {
        return llvm::Type::getFloatTy(context);
    } else if (type_name == "char") {
        return llvm::Type::getInt8Ty(context);
    } else if (type_name == "void") {
        return llvm::Type::getVoidTy(context);
    }
    return llvm::Type::getInt32Ty(context); // Default to int
}

llvm::Value* TypeNode::codegen(CodeGenerator& gen) {
    return nullptr; // Types don't generate values directly
}

void TypeNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "Type: " << type_name << "\n";
}

// FunctionDefinitionNode implementation
llvm::Value* FunctionDefinitionNode::codegen(CodeGenerator& gen) {
    return gen.codegenFunctionDefinition(this);
}

void FunctionDefinitionNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "FunctionDefinition: " << name << "\n";
    return_type->print(indent + 2);
    if (parameters) {
        std::cout << std::string(indent + 2, ' ') << "Parameters:\n";
        for (auto* param : *parameters) {
            param->print(indent + 4);
        }
    }
    if (body) {
        std::cout << std::string(indent + 2, ' ') << "Body:\n";
        body->print(indent + 4);
    }
}

// ParameterNode implementation
llvm::Value* ParameterNode::codegen(CodeGenerator& gen) {
    return nullptr; // Parameters are handled in function definition
}

void ParameterNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "Parameter: " << name << "\n";
    type->print(indent + 2);
}

// ParameterListNode implementation
llvm::Value* ParameterListNode::codegen(CodeGenerator& gen) {
    return nullptr; // Parameter lists are handled in function definition
}

void ParameterListNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "ParameterList\n";
    if (parameters) {
        for (auto* param : *parameters) {
            param->print(indent + 2);
        }
    }
}

// VariableDeclarationNode implementation
llvm::Value* VariableDeclarationNode::codegen(CodeGenerator& gen) {
    return gen.codegenVariableDeclaration(this);
}

void VariableDeclarationNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "VariableDeclaration: " << name << "\n";
    type->print(indent + 2);
    if (initializer) {
        std::cout << std::string(indent + 2, ' ') << "Initializer:\n";
        initializer->print(indent + 4);
    }
}

// CompoundStatementNode implementation
llvm::Value* CompoundStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenCompoundStatement(this);
}

void CompoundStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "CompoundStatement\n";
    if (statements) {
        for (auto* stmt : *statements) {
            stmt->print(indent + 2);
        }
    }
}

// ExpressionStatementNode implementation
llvm::Value* ExpressionStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenExpressionStatement(this);
}

void ExpressionStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "ExpressionStatement\n";
    if (expression) {
        expression->print(indent + 2);
    }
}

// IfStatementNode implementation
llvm::Value* IfStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenIfStatement(this);
}

void IfStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "IfStatement\n";
    std::cout << std::string(indent + 2, ' ') << "Condition:\n";
    condition->print(indent + 4);
    std::cout << std::string(indent + 2, ' ') << "Then:\n";
    then_stmt->print(indent + 4);
    if (else_stmt) {
        std::cout << std::string(indent + 2, ' ') << "Else:\n";
        else_stmt->print(indent + 4);
    }
}

// WhileStatementNode implementation
llvm::Value* WhileStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenWhileStatement(this);
}

void WhileStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "WhileStatement\n";
    std::cout << std::string(indent + 2, ' ') << "Condition:\n";
    condition->print(indent + 4);
    std::cout << std::string(indent + 2, ' ') << "Body:\n";
    body->print(indent + 4);
}

// ForStatementNode implementation
llvm::Value* ForStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenForStatement(this);
}

void ForStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "ForStatement\n";
    if (init) {
        std::cout << std::string(indent + 2, ' ') << "Init:\n";
        init->print(indent + 4);
    }
    if (condition) {
        std::cout << std::string(indent + 2, ' ') << "Condition:\n";
        condition->print(indent + 4);
    }
    if (increment) {
        std::cout << std::string(indent + 2, ' ') << "Increment:\n";
        increment->print(indent + 4);
    }
    std::cout << std::string(indent + 2, ' ') << "Body:\n";
    body->print(indent + 4);
}

// ReturnStatementNode implementation
llvm::Value* ReturnStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenReturnStatement(this);
}

void ReturnStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "ReturnStatement\n";
    if (expression) {
        expression->print(indent + 2);
    }
}

// BreakStatementNode implementation
llvm::Value* BreakStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenBreakStatement(this);
}

void BreakStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BreakStatement\n";
}

// ContinueStatementNode implementation
llvm::Value* ContinueStatementNode::codegen(CodeGenerator& gen) {
    return gen.codegenContinueStatement(this);
}

void ContinueStatementNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "ContinueStatement\n";
}

// AssignmentNode implementation
llvm::Value* AssignmentNode::codegen(CodeGenerator& gen) {
    return gen.codegenAssignment(this);
}

void AssignmentNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "Assignment\n";
    lhs->print(indent + 2);
    rhs->print(indent + 2);
}

// BinaryExpressionNode implementation
llvm::Value* BinaryExpressionNode::codegen(CodeGenerator& gen) {
    return gen.codegenBinaryExpression(this);
}

void BinaryExpressionNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BinaryExpression: " << op << "\n";
    left->print(indent + 2);
    right->print(indent + 2);
}

// UnaryExpressionNode implementation
llvm::Value* UnaryExpressionNode::codegen(CodeGenerator& gen) {
    return gen.codegenUnaryExpression(this);
}

void UnaryExpressionNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "UnaryExpression: " << op;
    if (postfix) std::cout << " (postfix)";
    std::cout << "\n";
    operand->print(indent + 2);
}

// IdentifierNode implementation
llvm::Value* IdentifierNode::codegen(CodeGenerator& gen) {
    return gen.codegenIdentifier(this);
}

void IdentifierNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "Identifier: " << name << "\n";
}

// IntegerLiteralNode implementation
llvm::Value* IntegerLiteralNode::codegen(CodeGenerator& gen) {
    return gen.codegenIntegerLiteral(this);
}

void IntegerLiteralNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "IntegerLiteral: " << value << "\n";
}

// FloatLiteralNode implementation
llvm::Value* FloatLiteralNode::codegen(CodeGenerator& gen) {
    return gen.codegenFloatLiteral(this);
}

void FloatLiteralNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "FloatLiteral: " << value << "\n";
}

// CharLiteralNode implementation
llvm::Value* CharLiteralNode::codegen(CodeGenerator& gen) {
    return gen.codegenCharLiteral(this);
}

void CharLiteralNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "CharLiteral: '" << value << "'\n";
}

// StringLiteralNode implementation
llvm::Value* StringLiteralNode::codegen(CodeGenerator& gen) {
    return gen.codegenStringLiteral(this);
}

void StringLiteralNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "StringLiteral: \"" << value << "\"\n";
}

// BooleanLiteralNode implementation
llvm::Value* BooleanLiteralNode::codegen(CodeGenerator& gen) {
    return gen.codegenBooleanLiteral(this);
}

void BooleanLiteralNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BooleanLiteral: " << (value ? "true" : "false") << "\n";
}

// FunctionCallNode implementation
llvm::Value* FunctionCallNode::codegen(CodeGenerator& gen) {
    return gen.codegenFunctionCall(this);
}

void FunctionCallNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "FunctionCall: " << name << "\n";
    if (arguments) {
        for (auto* arg : *arguments) {
            arg->print(indent + 2);
        }
    }
}

// ArrayAccessNode implementation
llvm::Value* ArrayAccessNode::codegen(CodeGenerator& gen) {
    return gen.codegenArrayAccess(this);
}

void ArrayAccessNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "ArrayAccess: " << name << "\n";
    index->print(indent + 2);
}
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <memory>
#include <vector>

class Parser {
private:
    Lexer& lexer;
    int curTok;
    
    int getNextToken() {
        return curTok = lexer.gettok();
    }
    
public:
    Parser(Lexer& lexer) : lexer(lexer) {
        getNextToken(); // prime the first token
    }
    
    // Get precedence of binary operators
    int getTokPrecedence() {
        switch (curTok) {
            case '=': return 2;
            case tok_eq:
            case tok_ne: return 10;
            case '<':
            case '>':
            case tok_le:
            case tok_ge: return 20;
            case '+':
            case '-': return 30;
            case '*':
            case '/': return 40;
            default: return -1;
        }
    }
    
    // Parse primary expression
    std::unique_ptr<ExprNode> parsePrimary() {
        switch (curTok) {
            case tok_identifier: {
                std::string idName = lexer.identifierStr;
                getNextToken();
                
                if (curTok == '(') {
                    // Function call
                    getNextToken(); // eat '('
                    auto call = std::make_unique<FunctionCallNode>(idName);
                    
                    if (curTok != ')') {
                        while (true) {
                            auto arg = parseExpression();
                            if (!arg) return nullptr;
                            call->args.push_back(std::move(arg));
                            
                            if (curTok == ')') break;
                            
                            if (curTok != ',') {
                                std::cerr << "Expected ')' or ',' in function call" << std::endl;
                                return nullptr;
                            }
                            getNextToken(); // eat ','
                        }
                    }
                    
                    getNextToken(); // eat ')'
                    return call;
                } else if (curTok == '=') {
                    // Assignment
                    getNextToken(); // eat '='
                    auto expr = parseExpression();
                    if (!expr) return nullptr;
                    return std::make_unique<AssignmentNode>(idName, std::move(expr));
                } else {
                    // Variable reference
                    return std::make_unique<IdentifierNode>(idName);
                }
            }
            case tok_number: {
                auto result = std::make_unique<IntegerNode>(lexer.numVal);
                getNextToken();
                return result;
            }
            case tok_float_literal: {
                auto result = std::make_unique<DoubleNode>(lexer.floatVal);
                getNextToken();
                return result;
            }
            case '(': {
                getNextToken(); // eat '('
                auto expr = parseExpression();
                if (!expr) return nullptr;
                
                if (curTok != ')') {
                    std::cerr << "Expected ')'" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ')'
                return expr;
            }
            default:
                std::cerr << "Unknown token when expecting an expression: " << curTok << std::endl;
                return nullptr;
        }
    }
    
    // Parse unary expression
    std::unique_ptr<ExprNode> parseUnary() {
        if (curTok != '-' && curTok != '!') {
            return parsePrimary();
        }
        
        int op = curTok;
        getNextToken();
        
        auto operand = parseUnary();
        if (!operand) return nullptr;
        
        return std::make_unique<UnaryOpNode>(op, std::move(operand));
    }
    
    // Parse binary operator right-hand side
    std::unique_ptr<ExprNode> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprNode> lhs) {
        while (true) {
            int tokPrec = getTokPrecedence();
            
            if (tokPrec < exprPrec) {
                return lhs;
            }
            
            int binOp = curTok;
            getNextToken(); // eat operator
            
            auto rhs = parseUnary();
            if (!rhs) return nullptr;
            
            int nextPrec = getTokPrecedence();
            if (tokPrec < nextPrec) {
                rhs = parseBinOpRHS(tokPrec + 1, std::move(rhs));
                if (!rhs) return nullptr;
            }
            
            // Map token to operator code
            int opCode = binOp;
            if (binOp == tok_eq) opCode = 256;
            else if (binOp == tok_ne) opCode = 257;
            
            lhs = std::make_unique<BinaryOpNode>(std::move(lhs), opCode, std::move(rhs));
        }
    }
    
    // Parse expression
    std::unique_ptr<ExprNode> parseExpression() {
        auto lhs = parseUnary();
        if (!lhs) return nullptr;
        
        return parseBinOpRHS(0, std::move(lhs));
    }
    
    // Parse statement
    std::unique_ptr<StmtNode> parseStatement() {
        switch (curTok) {
            case tok_int:
            case tok_float:
            case tok_double: {
                // Variable declaration
                std::string type;
                if (curTok == tok_int) type = "int";
                else if (curTok == tok_float) type = "float";
                else type = "double";
                
                getNextToken();
                
                if (curTok != tok_identifier) {
                    std::cerr << "Expected identifier after type" << std::endl;
                    return nullptr;
                }
                
                std::string name = lexer.identifierStr;
                getNextToken();
                
                std::unique_ptr<ExprNode> initExpr = nullptr;
                if (curTok == '=') {
                    getNextToken(); // eat '='
                    initExpr = parseExpression();
                    if (!initExpr) return nullptr;
                }
                
                if (curTok != ';') {
                    std::cerr << "Expected ';' after variable declaration" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ';'
                
                return std::make_unique<VariableDeclNode>(type, name, std::move(initExpr));
            }
            case tok_if: {
                getNextToken(); // eat 'if'
                
                if (curTok != '(') {
                    std::cerr << "Expected '(' after 'if'" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat '('
                
                auto condition = parseExpression();
                if (!condition) return nullptr;
                
                if (curTok != ')') {
                    std::cerr << "Expected ')' after if condition" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ')'
                
                auto thenBlock = parseBlock();
                if (!thenBlock) return nullptr;
                
                std::unique_ptr<BlockNode> elseBlock = nullptr;
                if (curTok == tok_else) {
                    getNextToken(); // eat 'else'
                    elseBlock = parseBlock();
                    if (!elseBlock) return nullptr;
                }
                
                return std::make_unique<IfStmtNode>(std::move(condition), 
                                                    std::move(thenBlock), 
                                                    std::move(elseBlock));
            }
            case tok_while: {
                getNextToken(); // eat 'while'
                
                if (curTok != '(') {
                    std::cerr << "Expected '(' after 'while'" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat '('
                
                auto condition = parseExpression();
                if (!condition) return nullptr;
                
                if (curTok != ')') {
                    std::cerr << "Expected ')' after while condition" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ')'
                
                auto body = parseBlock();
                if (!body) return nullptr;
                
                return std::make_unique<WhileStmtNode>(std::move(condition), std::move(body));
            }
            case tok_return: {
                getNextToken(); // eat 'return'
                
                std::unique_ptr<ExprNode> expr = nullptr;
                if (curTok != ';') {
                    expr = parseExpression();
                    if (!expr) return nullptr;
                }
                
                if (curTok != ';') {
                    std::cerr << "Expected ';' after return" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ';'
                
                return std::make_unique<ReturnStmtNode>(std::move(expr));
            }
            default: {
                // Expression statement
                auto expr = parseExpression();
                if (!expr) return nullptr;
                
                if (curTok != ';') {
                    std::cerr << "Expected ';' after expression" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ';'
                
                return std::make_unique<ExprStmtNode>(std::move(expr));
            }
        }
    }
    
    // Parse block
    std::unique_ptr<BlockNode> parseBlock() {
        if (curTok != '{') {
            std::cerr << "Expected '{'" << std::endl;
            return nullptr;
        }
        getNextToken(); // eat '{'
        
        auto block = std::make_unique<BlockNode>();
        
        while (curTok != '}' && curTok != tok_eof) {
            auto stmt = parseStatement();
            if (!stmt) return nullptr;
            block->statements.push_back(std::move(stmt));
        }
        
        if (curTok != '}') {
            std::cerr << "Expected '}'" << std::endl;
            return nullptr;
        }
        getNextToken(); // eat '}'
        
        return block;
    }
    
    // Parse function
    std::unique_ptr<FunctionNode> parseFunction() {
        std::string returnType;
        if (curTok == tok_int) returnType = "int";
        else if (curTok == tok_float) returnType = "float";
        else if (curTok == tok_double) returnType = "double";
        else if (curTok == tok_void) returnType = "void";
        else {
            std::cerr << "Expected return type" << std::endl;
            return nullptr;
        }
        
        getNextToken(); // eat return type
        
        if (curTok != tok_identifier) {
            std::cerr << "Expected function name" << std::endl;
            return nullptr;
        }
        
        std::string name = lexer.identifierStr;
        getNextToken();
        
        if (curTok != '(') {
            std::cerr << "Expected '(' in function declaration" << std::endl;
            return nullptr;
        }
        getNextToken(); // eat '('
        
        auto func = std::make_unique<FunctionNode>(returnType, name);
        
        // Parse parameters
        if (curTok != ')') {
            while (true) {
                std::string paramType;
                if (curTok == tok_int) paramType = "int";
                else if (curTok == tok_float) paramType = "float";
                else if (curTok == tok_double) paramType = "double";
                else {
                    std::cerr << "Expected parameter type" << std::endl;
                    return nullptr;
                }
                getNextToken();
                
                if (curTok != tok_identifier) {
                    std::cerr << "Expected parameter name" << std::endl;
                    return nullptr;
                }
                
                std::string paramName = lexer.identifierStr;
                getNextToken();
                
                func->params.push_back(std::make_unique<ParamNode>(paramType, paramName));
                
                if (curTok == ')') break;
                
                if (curTok != ',') {
                    std::cerr << "Expected ')' or ',' in parameter list" << std::endl;
                    return nullptr;
                }
                getNextToken(); // eat ','
            }
        }
        
        getNextToken(); // eat ')'
        
        // Parse function body
        func->body = parseBlock();
        if (!func->body) return nullptr;
        
        return func;
    }
    
    // Parse program
    std::unique_ptr<ProgramNode> parseProgram() {
        auto program = std::make_unique<ProgramNode>();
        
        while (curTok != tok_eof) {
            auto func = parseFunction();
            if (!func) return nullptr;
            program->functions.push_back(std::move(func));
        }
        
        return program;
    }
};

#endif // PARSER_H

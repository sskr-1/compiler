#pragma once

#include "ast.h"
#include "lexer.h"
#include <memory>
#include <vector>
#include <map>

class Parser {
private:
    std::map<char, int> BinopPrecedence;
    
    void initBinopPrecedence();
    int getTokPrecedence();
    
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST> parseIdentifierExpr();
    std::unique_ptr<ExprAST> parseParenExpr();
    std::unique_ptr<ExprAST> parseNumberExpr();
    std::unique_ptr<ExprAST> parseIfExpr();
    
    std::unique_ptr<StmtAST> parseStatement();
    std::unique_ptr<StmtAST> parseBlock();
    std::unique_ptr<VarDeclAST> parseVarDeclaration();
    std::unique_ptr<AssignmentAST> parseAssignment();
    std::unique_ptr<ReturnAST> parseReturn();
    std::unique_ptr<ExpressionStmtAST> parseExpressionStatement();
    
    std::unique_ptr<PrototypeAST> parsePrototype();
    std::unique_ptr<FunctionAST> parseDefinition();
    std::unique_ptr<PrototypeAST> parseExtern();
    std::unique_ptr<ExprAST> parseTopLevelExpr();
    
    void logError(const std::string& str);
    
public:
    Parser();
    
    // Main parsing entry points
    std::unique_ptr<FunctionAST> parseTopLevelFunction();
    std::unique_ptr<PrototypeAST> parseExternDeclaration();
    std::unique_ptr<FunctionAST> parseFunctionDefinition();
    
    // Parse a complete program
    std::vector<std::unique_ptr<ASTNode>> parseProgram();
};
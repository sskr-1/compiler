#pragma once

#include "lexer.h"
#include "ast.h"
#include <memory>
#include <string>

namespace clike {

class Parser {
public:
    Parser(Lexer& lexer);
    
    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<FunctionDef> parseFunction();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignment();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseAddition();
    std::unique_ptr<Expression> parseMultiplication();
    std::unique_ptr<Expression> parsePrimary();

private:
    Lexer& lexer_;
    Token currentToken_;
    
    void advance();
    void expect(TokenType expected);
    bool match(TokenType type);
    bool matchAny(const std::vector<TokenType>& types);
    
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<IfStmt> parseIfStatement();
    std::unique_ptr<WhileStmt> parseWhileStatement();
    std::unique_ptr<ReturnStmt> parseReturnStatement();
    std::unique_ptr<VarDeclStmt> parseVarDeclaration();
    std::unique_ptr<CallExpr> parseCall(const std::string& callee);
    
    void reportError(const std::string& message);
    void synchronize();
};

} // namespace clike
#include "parser.h"
#include <iostream>

namespace clike {

Parser::Parser(Lexer& lexer) : lexer_(lexer), currentToken_(END_OF_FILE, "") {
    advance();
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<std::unique_ptr<FunctionDef>> functions;
    
    while (currentToken_.type != END_OF_FILE) {
        auto function = parseFunction();
        if (!function) {
            return nullptr;
        }
        functions.push_back(std::move(function));
    }
    
    return std::make_unique<Program>(std::move(functions));
}

std::unique_ptr<FunctionDef> Parser::parseFunction() {
    // Function definition: function name(args) { body }
    if (currentToken_.type != IDENTIFIER) {
        reportError("Expected function name");
        return nullptr;
    }
    
    std::string functionName = currentToken_.value;
    advance();
    
    // Parse arguments
    expect(LEFT_PAREN);
    std::vector<std::string> args;
    
    if (currentToken_.type != RIGHT_PAREN) {
        do {
            if (currentToken_.type != IDENTIFIER) {
                reportError("Expected parameter name");
                return nullptr;
            }
            args.push_back(currentToken_.value);
            advance();
            if (currentToken_.type == COMMA) {
                advance();
            }
        } while (currentToken_.type != RIGHT_PAREN);
    }
    
    expect(RIGHT_PAREN);
    expect(LEFT_BRACE);
    
    auto body = parseBlock();
    if (!body) {
        return nullptr;
    }
    
    expect(RIGHT_BRACE);
    
    return std::make_unique<FunctionDef>(functionName, std::move(args), std::move(body));
}

std::unique_ptr<Statement> Parser::parseStatement() {
    switch (currentToken_.type) {
        case IF:
            return parseIfStatement();
        case WHILE:
            return parseWhileStatement();
        case RETURN:
            return parseReturnStatement();
        case VAR:
            return parseVarDeclaration();
        case LEFT_BRACE:
            return parseBlock();
        default:
            return std::make_unique<ExprStmt>(parseExpression());
    }
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expression> Parser::parseAssignment() {
    auto left = parseEquality();
    
    if (match(ASSIGN)) {
        advance();
        auto right = parseAssignment();
        if (!right) {
            return nullptr;
        }
        
        // For assignment, we need to handle variable assignment differently
        // For now, treat as equality comparison
        return std::make_unique<BinaryExpr>('=', std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto left = parseComparison();
    
    while (matchAny({EQUAL, NOT_EQUAL})) {
        char op = (currentToken_.type == EQUAL) ? '=' : '!';
        advance();
        auto right = parseComparison();
        if (!right) {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto left = parseAddition();
    
    while (matchAny({LESS, GREATER, LESS_EQUAL, GREATER_EQUAL})) {
        char op;
        switch (currentToken_.type) {
            case LESS: op = '<'; break;
            case GREATER: op = '>'; break;
            case LESS_EQUAL: op = '<'; break; // We'll handle <= as < for now
            case GREATER_EQUAL: op = '>'; break; // We'll handle >= as > for now
            default: op = '<'; break;
        }
        advance();
        auto right = parseAddition();
        if (!right) {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseAddition() {
    auto left = parseMultiplication();
    
    while (matchAny({PLUS, MINUS})) {
        char op = (currentToken_.type == PLUS) ? '+' : '-';
        advance();
        auto right = parseMultiplication();
        if (!right) {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseMultiplication() {
    auto left = parsePrimary();
    
    while (matchAny({MULTIPLY, DIVIDE})) {
        char op = (currentToken_.type == MULTIPLY) ? '*' : '/';
        advance();
        auto right = parsePrimary();
        if (!right) {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    switch (currentToken_.type) {
        case NUMBER: {
            double value = std::stod(currentToken_.value);
            advance();
            return std::make_unique<NumberExpr>(value);
        }
        
        case IDENTIFIER: {
            std::string name = currentToken_.value;
            
            if (lexer_.peekToken().type == LEFT_PAREN) {
                advance();
                return parseCall(name);
            }
            
            advance();
            return std::make_unique<VariableExpr>(name);
        }
        
        case LEFT_PAREN: {
            advance();
            auto expr = parseExpression();
            if (!expr) {
                return nullptr;
            }
            expect(RIGHT_PAREN);
            return expr;
        }
        
        default:
            reportError("Unexpected token: " + currentToken_.value);
            return nullptr;
    }
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    expect(LEFT_BRACE);
    
    std::vector<std::unique_ptr<Statement>> statements;
    
    while (currentToken_.type != RIGHT_BRACE && currentToken_.type != END_OF_FILE) {
        auto statement = parseStatement();
        if (!statement) {
            return nullptr;
        }
        statements.push_back(std::move(statement));
        
        if (currentToken_.type == SEMICOLON) {
            advance();
        }
    }
    
    expect(RIGHT_BRACE);
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<IfStmt> Parser::parseIfStatement() {
    expect(IF);
    expect(LEFT_PAREN);
    
    auto condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    
    expect(RIGHT_PAREN);
    
    auto thenStmt = parseStatement();
    if (!thenStmt) {
        return nullptr;
    }
    
    std::unique_ptr<Statement> elseStmt = nullptr;
    if (match(ELSE)) {
        advance();
        elseStmt = parseStatement();
        if (!elseStmt) {
            return nullptr;
        }
    }
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenStmt), std::move(elseStmt));
}

std::unique_ptr<WhileStmt> Parser::parseWhileStatement() {
    expect(WHILE);
    expect(LEFT_PAREN);
    
    auto condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    
    expect(RIGHT_PAREN);
    
    auto body = parseStatement();
    if (!body) {
        return nullptr;
    }
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<ReturnStmt> Parser::parseReturnStatement() {
    expect(RETURN);
    
    std::unique_ptr<Expression> expr = nullptr;
    if (currentToken_.type != SEMICOLON && currentToken_.type != RIGHT_BRACE) {
        expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
    }
    
    return std::make_unique<ReturnStmt>(std::move(expr));
}

std::unique_ptr<VarDeclStmt> Parser::parseVarDeclaration() {
    expect(VAR);
    
    if (currentToken_.type != IDENTIFIER) {
        reportError("Expected variable name");
        return nullptr;
    }
    
    std::string name = currentToken_.value;
    advance();
    
    std::unique_ptr<Expression> init = nullptr;
    if (match(ASSIGN)) {
        advance();
        init = parseExpression();
        if (!init) {
            return nullptr;
        }
    }
    
    return std::make_unique<VarDeclStmt>(name, std::move(init));
}

std::unique_ptr<CallExpr> Parser::parseCall(const std::string& callee) {
    expect(LEFT_PAREN);
    
    std::vector<std::unique_ptr<Expression>> args;
    if (currentToken_.type != RIGHT_PAREN) {
        do {
            auto arg = parseExpression();
            if (!arg) {
                return nullptr;
            }
            args.push_back(std::move(arg));
        } while (match(COMMA));
    }
    
    expect(RIGHT_PAREN);
    return std::make_unique<CallExpr>(callee, std::move(args));
}

void Parser::advance() {
    currentToken_ = lexer_.getNextToken();
}

void Parser::expect(TokenType expected) {
    if (currentToken_.type != expected) {
        reportError("Expected " + std::to_string(expected) + " but got " + std::to_string(currentToken_.type));
        return;
    }
    advance();
}

bool Parser::match(TokenType type) {
    return currentToken_.type == type;
}

bool Parser::matchAny(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (currentToken_.type == type) {
            return true;
        }
    }
    return false;
}

void Parser::reportError(const std::string& message) {
    std::cerr << "Parse error at line " << currentToken_.line 
              << ", column " << currentToken_.column << ": " << message << std::endl;
}

void Parser::synchronize() {
    advance();
    
    while (currentToken_.type != END_OF_FILE) {
        if (currentToken_.type == SEMICOLON) {
            advance();
            return;
        }
        
        switch (currentToken_.type) {
            case IF:
            case WHILE:
            case RETURN:
            case VAR:
            case LEFT_BRACE:
                return;
            default:
                advance();
                break;
        }
    }
}

} // namespace clike
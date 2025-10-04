#include "parser.h"
#include <iostream>

Parser::Parser() {
    initBinopPrecedence();
}

void Parser::initBinopPrecedence() {
    // 1 is lowest precedence
    BinopPrecedence['<'] = 10;
    BinopPrecedence['>'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;
    BinopPrecedence['/'] = 40;
}

int Parser::getTokPrecedence() {
    if (!isascii(Lexer::CurTok))
        return -1;
    
    int tokPrec = BinopPrecedence[Lexer::CurTok];
    if (tokPrec <= 0)
        return -1;
    return tokPrec;
}

void Parser::logError(const std::string& str) {
    std::cerr << "Error: " << str << std::endl;
}

std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(Lexer::NumVal);
    Lexer::getNextTok(); // consume the number
    return std::move(result);
}

std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    Lexer::getNextTok(); // eat '('
    auto V = parseExpression();
    if (!V)
        return nullptr;
    
    if (Lexer::CurTok != ')')
        return nullptr; // logError("expected ')'");
    Lexer::getNextTok(); // eat ')'
    return V;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = Lexer::IdentifierStr;
    
    Lexer::getNextTok(); // eat identifier
    
    if (Lexer::CurTok != tok_lparen) // Simple variable ref
        return std::make_unique<VariableExprAST>(idName);
    
    // Call
    Lexer::getNextTok(); // eat '('
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (Lexer::CurTok != tok_rparen) {
        while (true) {
            if (auto Arg = parseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;
            
            if (Lexer::CurTok == tok_rparen)
                break;
            
            if (Lexer::CurTok != tok_comma)
                return nullptr; // logError("Expected ')' or ',' in argument list");
            Lexer::getNextTok();
        }
    }
    
    Lexer::getNextTok(); // eat ')'
    
    return std::make_unique<CallExprAST>(idName, std::move(Args));
}

std::unique_ptr<ExprAST> Parser::parseIfExpr() {
    Lexer::getNextTok(); // eat the if
    
    // condition
    auto Cond = parseExpression();
    if (!Cond)
        return nullptr;
    
    if (Lexer::CurTok != tok_then)
        return nullptr; // logError("expected then");
    Lexer::getNextTok(); // eat the then
    
    auto Then = parseStatement();
    if (!Then)
        return nullptr;
    
    std::unique_ptr<StmtAST> Else = nullptr;
    if (Lexer::CurTok == tok_else) {
        Lexer::getNextTok();
        Else = parseStatement();
        if (!Else)
            return nullptr;
    }
    
    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then), std::move(Else));
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (Lexer::CurTok) {
        default:
            return nullptr; // logError("unknown token when expecting an expression");
        case tok_identifier:
            return parseIdentifierExpr();
        case tok_number:
            return parseNumberExpr();
        case tok_lparen:
            return parseParenExpr();
        case tok_if:
            return parseIfExpr();
    }
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence
    while (true) {
        int TokPrec = getTokPrecedence();
        
        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done
        if (TokPrec < ExprPrec)
            return LHS;
        
        // Okay, we know this is a binop
        int BinOp = Lexer::CurTok;
        Lexer::getNextTok(); // eat binop
        
        // Parse the primary expression after the binary operator
        auto RHS = parsePrimary();
        if (!RHS)
            return nullptr;
        
        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS
        int NextPrec = getTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }
        
        // Merge LHS/RHS
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    auto LHS = parsePrimary();
    if (!LHS)
        return nullptr;
    
    return parseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype() {
    if (Lexer::CurTok != tok_identifier)
        return nullptr; // logError("Expected function name in prototype");
    
    std::string FnName = Lexer::IdentifierStr;
    Lexer::getNextTok();
    
    if (Lexer::CurTok != tok_lparen)
        return nullptr; // logError("Expected '(' in prototype");
    
    std::vector<std::string> ArgNames;
    while (Lexer::getNextTok() == tok_identifier)
        ArgNames.push_back(Lexer::IdentifierStr);
    
    if (Lexer::CurTok != tok_rparen)
        return nullptr; // logError("Expected ')' in prototype");
    
    // success
    Lexer::getNextTok(); // eat ')'
    
    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<VarDeclAST> Parser::parseVarDeclaration() {
    // Parse type
    std::string type;
    switch (Lexer::CurTok) {
        case tok_int: type = "int"; break;
        case tok_double: type = "double"; break;
        case tok_bool: type = "bool"; break;
        default:
            return nullptr; // logError("Expected type in variable declaration");
    }
    Lexer::getNextTok(); // consume type
    
    if (Lexer::CurTok != tok_identifier)
        return nullptr; // logError("Expected variable name");
    
    std::string varName = Lexer::IdentifierStr;
    Lexer::getNextTok(); // consume identifier
    
    std::unique_ptr<ExprAST> initVal = nullptr;
    if (Lexer::CurTok == tok_assign) {
        Lexer::getNextTok(); // consume '='
        initVal = parseExpression();
        if (!initVal)
            return nullptr;
    }
    
    return std::make_unique<VarDeclAST>(varName, type, std::move(initVal));
}

std::unique_ptr<AssignmentAST> Parser::parseAssignment() {
    std::string varName = Lexer::IdentifierStr;
    Lexer::getNextTok(); // consume identifier
    
    if (Lexer::CurTok != tok_assign)
        return nullptr; // logError("Expected '=' in assignment");
    
    Lexer::getNextTok(); // consume '='
    
    auto value = parseExpression();
    if (!value)
        return nullptr;
    
    return std::make_unique<AssignmentAST>(varName, std::move(value));
}

std::unique_ptr<ReturnAST> Parser::parseReturn() {
    Lexer::getNextTok(); // consume 'return'
    
    std::unique_ptr<ExprAST> retVal = nullptr;
    if (Lexer::CurTok != tok_semicolon) {
        retVal = parseExpression();
        if (!retVal)
            return nullptr;
    }
    
    return std::make_unique<ReturnAST>(std::move(retVal));
}

std::unique_ptr<ExpressionStmtAST> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    if (!expr)
        return nullptr;
    
    return std::make_unique<ExpressionStmtAST>(std::move(expr));
}

std::unique_ptr<StmtAST> Parser::parseBlock() {
    if (Lexer::CurTok != tok_lbrace)
        return nullptr; // logError("Expected '{'");
    
    Lexer::getNextTok(); // consume '{'
    
    std::vector<std::unique_ptr<StmtAST>> statements;
    
    while (Lexer::CurTok != tok_rbrace && Lexer::CurTok != tok_eof) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        } else {
            return nullptr;
        }
    }
    
    if (Lexer::CurTok != tok_rbrace)
        return nullptr; // logError("Expected '}'");
    
    Lexer::getNextTok(); // consume '}'
    
    return std::make_unique<BlockAST>(std::move(statements));
}

std::unique_ptr<StmtAST> Parser::parseStatement() {
    switch (Lexer::CurTok) {
        case tok_int:
        case tok_double:
        case tok_bool: {
            auto varDecl = parseVarDeclaration();
            if (Lexer::CurTok == tok_semicolon)
                Lexer::getNextTok(); // consume ';'
            return std::move(varDecl);
        }
        case tok_return: {
            auto ret = parseReturn();
            if (Lexer::CurTok == tok_semicolon)
                Lexer::getNextTok(); // consume ';'
            return std::move(ret);
        }
        case tok_lbrace:
            return parseBlock();
        case tok_if:
            return std::unique_ptr<StmtAST>(dynamic_cast<StmtAST*>(parseIfExpr().release()));
        case tok_identifier: {
            // Look ahead to see if it's an assignment
            std::string saved_identifier = Lexer::IdentifierStr;
            
            // Look ahead to determine if this is an assignment or expression
            // We'll use the existing token to make this determination
            
            Lexer::getNextTok(); // consume identifier
            
            if (Lexer::CurTok == tok_assign) {
                // Restore identifier and parse as assignment
                Lexer::IdentifierStr = saved_identifier;
                auto assignment = parseAssignment();
                if (Lexer::CurTok == tok_semicolon)
                    Lexer::getNextTok(); // consume ';'
                return std::move(assignment);
            } else {
                // Not an assignment, parse as expression statement
                // We need to backtrack - restore the identifier token
                Lexer::IdentifierStr = saved_identifier;
                Lexer::CurTok = tok_identifier;
                
                auto exprStmt = parseExpressionStatement();
                if (Lexer::CurTok == tok_semicolon)
                    Lexer::getNextTok(); // consume ';'
                return std::move(exprStmt);
            }
        }
        default: {
            auto exprStmt = parseExpressionStatement();
            if (Lexer::CurTok == tok_semicolon)
                Lexer::getNextTok(); // consume ';'
            return std::move(exprStmt);
        }
    }
}

std::unique_ptr<FunctionAST> Parser::parseDefinition() {
    Lexer::getNextTok(); // eat def
    auto Proto = parsePrototype();
    if (!Proto)
        return nullptr;
    
    auto Body = parseStatement();
    if (!Body)
        return nullptr;
    
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(Body));
}

std::unique_ptr<PrototypeAST> Parser::parseExtern() {
    Lexer::getNextTok(); // eat extern
    return parsePrototype();
}

std::unique_ptr<ExprAST> Parser::parseTopLevelExpr() {
    if (auto E = parseExpression()) {
        // Make an anonymous proto
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return E;
    }
    return nullptr;
}

std::unique_ptr<FunctionAST> Parser::parseTopLevelFunction() {
    if (auto E = parseTopLevelExpr()) {
        // Make an anonymous proto
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        auto exprStmt = std::make_unique<ExpressionStmtAST>(std::move(E));
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(exprStmt));
    }
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::parseExternDeclaration() {
    return parseExtern();
}

std::unique_ptr<FunctionAST> Parser::parseFunctionDefinition() {
    return parseDefinition();
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseProgram() {
    std::vector<std::unique_ptr<ASTNode>> nodes;
    
    // Prime the first token
    Lexer::getNextTok();
    
    while (Lexer::CurTok != tok_eof) {
        switch (Lexer::CurTok) {
            case tok_def: {
                if (auto FnAST = parseFunctionDefinition()) {
                    nodes.push_back(std::move(FnAST));
                } else {
                    Lexer::getNextTok(); // skip token for error recovery
                }
                break;
            }
            case tok_extern: {
                if (auto ProtoAST = parseExternDeclaration()) {
                    nodes.push_back(std::move(ProtoAST));
                } else {
                    Lexer::getNextTok(); // skip token for error recovery
                }
                break;
            }
            case tok_semicolon:
                Lexer::getNextTok(); // ignore top-level semicolons
                break;
            default: {
                if (auto FnAST = parseTopLevelFunction()) {
                    nodes.push_back(std::move(FnAST));
                } else {
                    Lexer::getNextTok(); // skip token for error recovery
                }
                break;
            }
        }
    }
    
    return nodes;
}
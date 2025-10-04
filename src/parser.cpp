#include "parser.hpp"
#include <stdexcept>

Parser::Parser(Lexer lexer) : lexer(std::move(lexer)) {}

const Token &Parser::peek() {
  if (!hasLookahead) { lookahead = this->lexer.peek(); hasLookahead = true; }
  return lookahead;
}

Token Parser::consume() {
  const Token &t = peek();
  hasLookahead = false;
  return this->lexer.next();
}

bool Parser::match(TokenKind kind) {
  if (peek().kind == kind) { consume(); return true; }
  return false;
}

void Parser::expect(TokenKind kind, const char *what) {
  if (!match(kind)) throw std::runtime_error(std::string("Expected ") + what);
}

std::unique_ptr<TranslationUnit> Parser::parseTranslationUnit() {
  auto tu = std::make_unique<TranslationUnit>();
  while (true) {
    if (peek().kind == TokenKind::Eof) break;
    if (peek().kind == TokenKind::KwExtern) {
      tu->externs.push_back(parseExtern());
    } else {
      tu->functions.push_back(parseFunction());
    }
  }
  return tu;
}

std::unique_ptr<ExternDecl> Parser::parseExtern() {
  expect(TokenKind::KwExtern, "extern");
  expect(TokenKind::KwInt, "int");
  Token nameTok = consume();
  if (nameTok.kind != TokenKind::Identifier) throw std::runtime_error("Expected function name");
  expect(TokenKind::LParen, "(");
  auto params = parseParamList();
  expect(TokenKind::RParen, ")");
  expect(TokenKind::Semicolon, ";");
  auto e = std::make_unique<ExternDecl>();
  e->name = nameTok.lexeme; e->params = std::move(params);
  return e;
}

std::unique_ptr<Function> Parser::parseFunction() {
  expect(TokenKind::KwInt, "int");
  Token nameTok = consume();
  if (nameTok.kind != TokenKind::Identifier) throw std::runtime_error("Expected function name");
  expect(TokenKind::LParen, "(");
  auto params = parseParamList();
  expect(TokenKind::RParen, ")");
  expect(TokenKind::LBrace, "{");
  auto body = parseBlock();
  expect(TokenKind::RBrace, "}");
  auto f = std::make_unique<Function>();
  f->name = nameTok.lexeme; f->params = std::move(params); f->body = std::move(body);
  return f;
}

std::vector<StmtPtr> Parser::parseBlock() {
  std::vector<StmtPtr> stmts;
  while (peek().kind != TokenKind::RBrace) {
    stmts.push_back(parseStatement());
  }
  return stmts;
}

StmtPtr Parser::parseStatement() {
  switch (peek().kind) {
    case TokenKind::KwReturn: return parseReturn();
    case TokenKind::KwIf: return parseIf();
    case TokenKind::KwWhile: return parseWhile();
    case TokenKind::KwInt: return parseVarDecl();
    default: return parseExprStmt();
  }
}

StmtPtr Parser::parseReturn() {
  expect(TokenKind::KwReturn, "return");
  auto v = parseExpression();
  expect(TokenKind::Semicolon, ";");
  return std::make_unique<ReturnStmt>(std::move(v));
}

StmtPtr Parser::parseIf() {
  expect(TokenKind::KwIf, "if");
  expect(TokenKind::LParen, "(");
  auto cond = parseExpression();
  expect(TokenKind::RParen, ")");
  expect(TokenKind::LBrace, "{");
  auto thenStmts = parseBlock();
  expect(TokenKind::RBrace, "}");
  std::vector<StmtPtr> elseStmts;
  if (match(TokenKind::KwElse)) {
    expect(TokenKind::LBrace, "{");
    elseStmts = parseBlock();
    expect(TokenKind::RBrace, "}");
  }
  auto s = std::make_unique<IfStmt>();
  s->cond = std::move(cond); s->thenStmts = std::move(thenStmts); s->elseStmts = std::move(elseStmts);
  return s;
}

StmtPtr Parser::parseWhile() {
  expect(TokenKind::KwWhile, "while");
  expect(TokenKind::LParen, "(");
  auto cond = parseExpression();
  expect(TokenKind::RParen, ")");
  expect(TokenKind::LBrace, "{");
  auto body = parseBlock();
  expect(TokenKind::RBrace, "}");
  auto s = std::make_unique<WhileStmt>();
  s->cond = std::move(cond); s->body = std::move(body);
  return s;
}

StmtPtr Parser::parseVarDecl() {
  expect(TokenKind::KwInt, "int");
  Token nameTok = consume();
  if (nameTok.kind != TokenKind::Identifier) throw std::runtime_error("Expected variable name");
  ExprPtr init;
  if (match(TokenKind::Assign)) {
    init = parseExpression();
  }
  expect(TokenKind::Semicolon, ";");
  return std::make_unique<VarDeclStmt>(nameTok.lexeme, std::move(init));
}

StmtPtr Parser::parseExprStmt() {
  auto e = parseExpression();
  expect(TokenKind::Semicolon, ";");
  return std::make_unique<ExprStmt>(std::move(e));
}

ExprPtr Parser::parseExpression() { return parseAssignment(); }

ExprPtr Parser::parseAssignment() {
  auto lhs = parseLogicalOr();
  if (peek().kind == TokenKind::Assign) {
    consume();
    if (auto v = dynamic_cast<VariableExpr*>(lhs.get())) {
      auto rhs = parseAssignment();
      return std::make_unique<AssignExpr>(v->name, std::move(rhs));
    } else {
      throw std::runtime_error("Invalid assignment target");
    }
  }
  return lhs;
}

static ExprPtr makeBinary(const std::string &op, ExprPtr lhs, ExprPtr rhs) {
  return std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
}

ExprPtr Parser::parseLogicalOr() {
  auto e = parseLogicalAnd();
  while (peek().kind == TokenKind::OrOr) { consume(); e = makeBinary("||", std::move(e), parseLogicalAnd()); }
  return e;
}

ExprPtr Parser::parseLogicalAnd() {
  auto e = parseEquality();
  while (peek().kind == TokenKind::AndAnd) { consume(); e = makeBinary("&&", std::move(e), parseEquality()); }
  return e;
}

ExprPtr Parser::parseEquality() {
  auto e = parseRelational();
  while (peek().kind == TokenKind::Eq || peek().kind == TokenKind::Ne) {
    Token op = consume();
    e = makeBinary(op.lexeme, std::move(e), parseRelational());
  }
  return e;
}

ExprPtr Parser::parseRelational() {
  auto e = parseAdditive();
  while (peek().kind == TokenKind::Lt || peek().kind == TokenKind::Le || peek().kind == TokenKind::Gt || peek().kind == TokenKind::Ge) {
    Token op = consume();
    e = makeBinary(op.lexeme, std::move(e), parseAdditive());
  }
  return e;
}

ExprPtr Parser::parseAdditive() {
  auto e = parseMultiplicative();
  while (peek().kind == TokenKind::Plus || peek().kind == TokenKind::Minus) {
    Token op = consume();
    e = makeBinary(op.lexeme, std::move(e), parseMultiplicative());
  }
  return e;
}

ExprPtr Parser::parseMultiplicative() {
  auto e = parseUnary();
  while (peek().kind == TokenKind::Star || peek().kind == TokenKind::Slash || peek().kind == TokenKind::Percent) {
    Token op = consume();
    e = makeBinary(op.lexeme, std::move(e), parseUnary());
  }
  return e;
}

ExprPtr Parser::parseUnary() {
  if (peek().kind == TokenKind::Minus || peek().kind == TokenKind::Not || peek().kind == TokenKind::Plus) {
    Token op = consume();
    return std::make_unique<UnaryExpr>(op.lexeme, parseUnary());
  }
  return parsePrimary();
}

ExprPtr Parser::parsePrimary() {
  Token t = consume();
  switch (t.kind) {
    case TokenKind::Number: return std::make_unique<NumberExpr>(t.intValue);
    case TokenKind::Identifier: {
      if (match(TokenKind::LParen)) {
        auto args = parseArgList();
        expect(TokenKind::RParen, ")");
        return std::make_unique<CallExpr>(t.lexeme, std::move(args));
      }
      return std::make_unique<VariableExpr>(t.lexeme);
    }
    case TokenKind::LParen: {
      auto e = parseExpression();
      expect(TokenKind::RParen, ")");
      return e;
    }
    default: throw std::runtime_error("Unexpected token in primary");
  }
}

std::vector<FunctionParam> Parser::parseParamList() {
  std::vector<FunctionParam> params;
  if (peek().kind == TokenKind::RParen) return params;
  while (true) {
    expect(TokenKind::KwInt, "int");
    Token nameTok = consume();
    if (nameTok.kind != TokenKind::Identifier) throw std::runtime_error("Expected parameter name");
    params.push_back(FunctionParam{nameTok.lexeme});
    if (!match(TokenKind::Comma)) break;
  }
  return params;
}

std::vector<ExprPtr> Parser::parseArgList() {
  std::vector<ExprPtr> args;
  if (peek().kind == TokenKind::RParen) return args;
  while (true) {
    args.push_back(parseExpression());
    if (!match(TokenKind::Comma)) break;
  }
  return args;
}

#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include <optional>

class Parser {
public:
  explicit Parser(Lexer lexer);

  std::unique_ptr<TranslationUnit> parseTranslationUnit();

private:
  Lexer lexer;
  Token lookahead;
  bool hasLookahead{false};

  const Token &peek();
  Token consume();
  bool match(TokenKind kind);
  void expect(TokenKind kind, const char *what);

  // Grammar helpers
  std::unique_ptr<ExternDecl> parseExtern();
  std::unique_ptr<Function> parseFunction();
  std::vector<StmtPtr> parseBlock();

  StmtPtr parseStatement();
  StmtPtr parseReturn();
  StmtPtr parseIf();
  StmtPtr parseWhile();
  StmtPtr parseVarDecl();
  StmtPtr parseExprStmt();

  // Expressions with precedence
  ExprPtr parseExpression();
  ExprPtr parseAssignment();
  ExprPtr parseLogicalOr();
  ExprPtr parseLogicalAnd();
  ExprPtr parseEquality();
  ExprPtr parseRelational();
  ExprPtr parseAdditive();
  ExprPtr parseMultiplicative();
  ExprPtr parseUnary();
  ExprPtr parsePrimary();

  std::vector<FunctionParam> parseParamList();
  std::vector<ExprPtr> parseArgList();
};

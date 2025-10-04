#pragma once

#include "token.hpp"
#include <string>

class Lexer {
public:
  explicit Lexer(const std::string &source);

  // Look at current token without consuming
  const Token &peek();

  // Consume and return current token
  Token next();

  bool atEnd() const;

private:
  std::string source;
  std::size_t index{0};
  std::size_t line{1};
  std::size_t column{1};

  Token current{};
  bool hasCurrent{false};

  void skipWhitespaceAndComments();
  Token readToken();
  Token makeToken(TokenKind kind, const std::string &lexeme, std::int64_t intValue = 0);
};

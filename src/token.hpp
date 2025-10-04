#pragma once

#include <cstdint>
#include <string>

enum class TokenKind {
  Eof,
  Identifier,
  Number,
  KwInt,
  KwReturn,
  KwIf,
  KwElse,
  KwWhile,
  KwExtern,
  LParen,
  RParen,
  LBrace,
  RBrace,
  Comma,
  Semicolon,
  Plus,
  Minus,
  Star,
  Slash,
  Percent,
  Assign,
  Eq,
  Ne,
  Lt,
  Gt,
  Le,
  Ge,
  AndAnd,
  OrOr,
  Not
};

struct Token {
  TokenKind kind{TokenKind::Eof};
  std::string lexeme{};
  std::int64_t intValue{0};
  std::size_t line{1};
  std::size_t column{1};
};

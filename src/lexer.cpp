#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string &source) : source(source) {}

const Token &Lexer::peek() {
  if (!hasCurrent) {
    current = readToken();
    hasCurrent = true;
  }
  return current;
}

Token Lexer::next() {
  const Token &t = peek();
  hasCurrent = false;
  return t;
}

bool Lexer::atEnd() const { return index >= source.size(); }

void Lexer::skipWhitespaceAndComments() {
  while (!atEnd()) {
    char c = source[index];
    if (c == ' ' || c == '\t' || c == '\r') { index++; column++; continue; }
    if (c == '\n') { index++; line++; column = 1; continue; }
    // // comment
    if (c == '/' && index + 1 < source.size() && source[index+1] == '/') {
      index += 2; column += 2;
      while (!atEnd() && source[index] != '\n') { index++; column++; }
      continue;
    }
    // /* block comment */
    if (c == '/' && index + 1 < source.size() && source[index+1] == '*') {
      index += 2; column += 2;
      while (!atEnd()) {
        if (source[index] == '*' && index + 1 < source.size() && source[index+1] == '/') {
          index += 2; column += 2; break;
        }
        if (source[index] == '\n') { line++; column = 1; index++; continue; }
        index++; column++;
      }
      continue;
    }
    break;
  }
}

Token Lexer::makeToken(TokenKind kind, const std::string &lexeme, std::int64_t intValue) {
  Token t; t.kind = kind; t.lexeme = lexeme; t.intValue = intValue; t.line = line; t.column = column; return t;
}

Token Lexer::readToken() {
  skipWhitespaceAndComments();
  if (atEnd()) return makeToken(TokenKind::Eof, "");

  char c = source[index];

  // Identifiers/keywords
  if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
    std::size_t start = index; std::size_t startCol = column;
    index++; column++;
    while (!atEnd()) {
      char d = source[index];
      if (std::isalnum(static_cast<unsigned char>(d)) || d == '_') { index++; column++; }
      else break;
    }
    std::string text = source.substr(start, index - start);
    if (text == "int") return Token{TokenKind::KwInt, text, 0, line, startCol};
    if (text == "return") return Token{TokenKind::KwReturn, text, 0, line, startCol};
    if (text == "if") return Token{TokenKind::KwIf, text, 0, line, startCol};
    if (text == "else") return Token{TokenKind::KwElse, text, 0, line, startCol};
    if (text == "while") return Token{TokenKind::KwWhile, text, 0, line, startCol};
    if (text == "extern") return Token{TokenKind::KwExtern, text, 0, line, startCol};
    return Token{TokenKind::Identifier, text, 0, line, startCol};
  }

  // Numbers (decimal integers only)
  if (std::isdigit(static_cast<unsigned char>(c))) {
    std::size_t start = index; std::size_t startCol = column;
    while (!atEnd() && std::isdigit(static_cast<unsigned char>(source[index]))) { index++; column++; }
    std::string text = source.substr(start, index - start);
    long long value = std::stoll(text);
    return Token{TokenKind::Number, text, value, line, startCol};
  }

  auto two = [&](char a, char b, TokenKind kind){
    if (!atEnd() && index + 1 < source.size() && source[index] == a && source[index+1] == b) {
      std::size_t startCol = column; index += 2; column += 2; return Token{kind, std::string({a,b}), 0, line, startCol};
    }
    return Token{TokenKind::Eof, "", 0, line, column};
  };

  // Two-char operators
  if (c == '=' && index + 1 < source.size() && source[index+1] == '=') { std::size_t sc = column; index += 2; column += 2; return Token{TokenKind::Eq, "==", 0, line, sc}; }
  if (c == '!' && index + 1 < source.size() && source[index+1] == '=') { std::size_t sc = column; index += 2; column += 2; return Token{TokenKind::Ne, "!=", 0, line, sc}; }
  if (c == '<' && index + 1 < source.size() && source[index+1] == '=') { std::size_t sc = column; index += 2; column += 2; return Token{TokenKind::Le, "<=", 0, line, sc}; }
  if (c == '>' && index + 1 < source.size() && source[index+1] == '=') { std::size_t sc = column; index += 2; column += 2; return Token{TokenKind::Ge, ">=", 0, line, sc}; }
  if (c == '&' && index + 1 < source.size() && source[index+1] == '&') { std::size_t sc = column; index += 2; column += 2; return Token{TokenKind::AndAnd, "&&", 0, line, sc}; }
  if (c == '|' && index + 1 < source.size() && source[index+1] == '|') { std::size_t sc = column; index += 2; column += 2; return Token{TokenKind::OrOr, "||", 0, line, sc}; }

  // Single-char tokens
  index++; column++;
  switch (c) {
    case '(': return makeToken(TokenKind::LParen, "(");
    case ')': return makeToken(TokenKind::RParen, ")");
    case '{': return makeToken(TokenKind::LBrace, "{");
    case '}': return makeToken(TokenKind::RBrace, "}");
    case ',': return makeToken(TokenKind::Comma, ",");
    case ';': return makeToken(TokenKind::Semicolon, ";");
    case '+': return makeToken(TokenKind::Plus, "+");
    case '-': return makeToken(TokenKind::Minus, "-");
    case '*': return makeToken(TokenKind::Star, "*");
    case '/': return makeToken(TokenKind::Slash, "/");
    case '%': return makeToken(TokenKind::Percent, "%");
    case '=': return makeToken(TokenKind::Assign, "=");
    case '<': return makeToken(TokenKind::Lt, "<");
    case '>': return makeToken(TokenKind::Gt, ">");
    case '!': return makeToken(TokenKind::Not, "!");
  }

  throw std::runtime_error("Unexpected character: " + std::string(1, c));
}

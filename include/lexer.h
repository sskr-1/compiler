#pragma once

#include <string>
#include <vector>

namespace clike {

enum TokenType {
    // Literals
    NUMBER,
    IDENTIFIER,
    
    // Keywords
    IF,
    ELSE,
    WHILE,
    RETURN,
    VAR,
    
    // Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    ASSIGN,         // =
    EQUAL,          // ==
    LESS,           // <
    GREATER,        // >
    LESS_EQUAL,     // <=
    GREATER_EQUAL,  // >=
    NOT_EQUAL,      // !=
    
    // Delimiters
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    SEMICOLON,      // ;
    COMMA,          // ,
    
    // Special
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, const std::string& v, int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
public:
    Lexer(const std::string& input);
    
    Token getNextToken();
    Token peekToken();
    void reset();
    
    bool hasMoreTokens() const;
    int getCurrentLine() const { return currentLine_; }
    int getCurrentColumn() const { return currentColumn_; }

private:
    std::string input_;
    size_t position_;
    int currentLine_;
    int currentColumn_;
    
    void skipWhitespace();
    void skipComment();
    Token readNumber();
    Token readIdentifier();
    Token readOperator();
    char peek() const;
    char advance();
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isOperator(char c) const;
    
    static TokenType getKeywordType(const std::string& keyword);
};

} // namespace clike
#include "lexer.h"
#include <cctype>
#include <unordered_map>

namespace clike {

Lexer::Lexer(const std::string& input) 
    : input_(input), position_(0), currentLine_(1), currentColumn_(1) {}

Token Lexer::getNextToken() {
    skipWhitespace();
    
    if (position_ >= input_.length()) {
        return Token(END_OF_FILE, "", currentLine_, currentColumn_);
    }
    
    char current = peek();
    
    if (isDigit(current)) {
        return readNumber();
    } else if (isAlpha(current)) {
        return readIdentifier();
    } else if (isOperator(current)) {
        return readOperator();
    } else {
        // Handle single character tokens
        char c = advance();
        TokenType type = UNKNOWN;
        
        switch (c) {
            case '(': type = LEFT_PAREN; break;
            case ')': type = RIGHT_PAREN; break;
            case '{': type = LEFT_BRACE; break;
            case '}': type = RIGHT_BRACE; break;
            case ';': type = SEMICOLON; break;
            case ',': type = COMMA; break;
            default:
                return Token(UNKNOWN, std::string(1, c), currentLine_, currentColumn_);
        }
        
        return Token(type, std::string(1, c), currentLine_, currentColumn_);
    }
}

Token Lexer::peekToken() {
    size_t savedPosition = position_;
    int savedLine = currentLine_;
    int savedColumn = currentColumn_;
    
    Token token = getNextToken();
    
    position_ = savedPosition;
    currentLine_ = savedLine;
    currentColumn_ = savedColumn;
    
    return token;
}

void Lexer::reset() {
    position_ = 0;
    currentLine_ = 1;
    currentColumn_ = 1;
}

bool Lexer::hasMoreTokens() const {
    return position_ < input_.length();
}

void Lexer::skipWhitespace() {
    while (position_ < input_.length()) {
        char c = peek();
        if (c == ' ' || c == '\t') {
            advance();
        } else if (c == '\n') {
            advance();
            currentLine_++;
            currentColumn_ = 1;
        } else if (c == '\r') {
            advance();
            if (peek() == '\n') {
                advance();
            }
            currentLine_++;
            currentColumn_ = 1;
        } else if (c == '/' && position_ + 1 < input_.length() && input_[position_ + 1] == '/') {
            skipComment();
        } else {
            break;
        }
    }
}

void Lexer::skipComment() {
    while (position_ < input_.length() && peek() != '\n') {
        advance();
    }
}

Token Lexer::readNumber() {
    std::string value;
    int startLine = currentLine_;
    int startColumn = currentColumn_;
    
    while (position_ < input_.length() && isDigit(peek())) {
        value += advance();
    }
    
    // Handle decimal point
    if (position_ < input_.length() && peek() == '.') {
        value += advance();
        while (position_ < input_.length() && isDigit(peek())) {
            value += advance();
        }
    }
    
    return Token(NUMBER, value, startLine, startColumn);
}

Token Lexer::readIdentifier() {
    std::string value;
    int startLine = currentLine_;
    int startColumn = currentColumn_;
    
    while (position_ < input_.length() && isAlphaNumeric(peek())) {
        value += advance();
    }
    
    TokenType type = getKeywordType(value);
    if (type != UNKNOWN) {
        return Token(type, value, startLine, startColumn);
    }
    
    return Token(IDENTIFIER, value, startLine, startColumn);
}

Token Lexer::readOperator() {
    std::string value;
    int startLine = currentLine_;
    int startColumn = currentColumn_;
    
    char first = advance();
    value += first;
    
    // Check for two-character operators
    if (position_ < input_.length()) {
        char second = peek();
        std::string twoChar = std::string(1, first) + std::string(1, second);
        
        if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" || twoChar == ">=") {
            advance();
            value += second;
        }
    }
    
    TokenType type = UNKNOWN;
    if (value == "+") type = PLUS;
    else if (value == "-") type = MINUS;
    else if (value == "*") type = MULTIPLY;
    else if (value == "/") type = DIVIDE;
    else if (value == "=") type = ASSIGN;
    else if (value == "==") type = EQUAL;
    else if (value == "<") type = LESS;
    else if (value == ">") type = GREATER;
    else if (value == "<=") type = LESS_EQUAL;
    else if (value == ">=") type = GREATER_EQUAL;
    else if (value == "!=") type = NOT_EQUAL;
    
    return Token(type, value, startLine, startColumn);
}

char Lexer::peek() const {
    if (position_ >= input_.length()) {
        return '\0';
    }
    return input_[position_];
}

char Lexer::advance() {
    if (position_ >= input_.length()) {
        return '\0';
    }
    char c = input_[position_++];
    currentColumn_++;
    return c;
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(c);
}

bool Lexer::isAlpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isOperator(char c) const {
    return c == '+' || c == '-' || c == '*' || c == '/' || 
           c == '=' || c == '<' || c == '>' || c == '!';
}

TokenType Lexer::getKeywordType(const std::string& keyword) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"if", IF},
        {"else", ELSE},
        {"while", WHILE},
        {"return", RETURN},
        {"var", VAR}
    };
    
    auto it = keywords.find(keyword);
    return (it != keywords.end()) ? it->second : UNKNOWN;
}

} // namespace clike
#pragma once

#include <string>
#include <iostream>

enum Token {
    tok_eof = -1,
    
    // Commands
    tok_def = -2,
    tok_extern = -3,
    
    // Primary
    tok_identifier = -4,
    tok_number = -5,
    
    // Keywords
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,
    tok_for = -9,
    tok_while = -10,
    tok_return = -11,
    tok_int = -12,
    tok_double = -13,
    tok_void = -14,
    tok_bool = -15,
    
    // Operators
    tok_assign = -16,  // =
    tok_eq = -17,      // ==
    tok_ne = -18,      // !=
    tok_lt = -19,      // <
    tok_le = -20,      // <=
    tok_gt = -21,      // >
    tok_ge = -22,      // >=
    
    // Delimiters
    tok_semicolon = -23,
    tok_comma = -24,
    tok_lparen = -25,
    tok_rparen = -26,
    tok_lbrace = -27,
    tok_rbrace = -28
};

class Lexer {
private:
    std::string input;
    size_t position;
    char currentChar;
    
    void advance();
    void skipWhitespace();
    void skipComment();
    std::string readNumber();
    std::string readIdentifier();
    
public:
    Lexer(const std::string& source);
    int getNextToken();
    
    // Static members for current token state
    static std::string IdentifierStr;
    static double NumVal;
    static int CurTok;
    
    static int getNextTok() {
        return CurTok = getInstance().getNextToken();
    }
    
    static Lexer& getInstance() {
        static Lexer instance("");
        return instance;
    }
    
    void setInput(const std::string& source);
    
    // Make position and currentChar accessible for parser lookahead
    friend class Parser;
};
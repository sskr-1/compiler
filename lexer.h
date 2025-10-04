#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <cctype>
#include <iostream>

enum Token {
    tok_eof = -1,
    
    // keywords
    tok_int = -2,
    tok_float = -3,
    tok_double = -4,
    tok_void = -5,
    tok_if = -6,
    tok_else = -7,
    tok_while = -8,
    tok_return = -9,
    
    // primary
    tok_identifier = -10,
    tok_number = -11,
    tok_float_literal = -12,
    
    // operators
    tok_eq = -13,  // ==
    tok_ne = -14,  // !=
    tok_le = -15,  // <=
    tok_ge = -16,  // >=
};

class Lexer {
private:
    std::string input;
    size_t pos;
    int lastChar;
    
public:
    std::string identifierStr;
    long long numVal;
    double floatVal;
    
    Lexer(const std::string& input) : input(input), pos(0), lastChar(' ') {}
    
    int getChar() {
        if (pos >= input.length()) {
            return EOF;
        }
        return input[pos++];
    }
    
    int gettok() {
        // Skip whitespace
        while (isspace(lastChar)) {
            lastChar = getChar();
        }
        
        // Identifier: [a-zA-Z][a-zA-Z0-9_]*
        if (isalpha(lastChar)) {
            identifierStr = lastChar;
            while (isalnum(lastChar = getChar()) || lastChar == '_') {
                identifierStr += lastChar;
            }
            
            if (identifierStr == "int") return tok_int;
            if (identifierStr == "float") return tok_float;
            if (identifierStr == "double") return tok_double;
            if (identifierStr == "void") return tok_void;
            if (identifierStr == "if") return tok_if;
            if (identifierStr == "else") return tok_else;
            if (identifierStr == "while") return tok_while;
            if (identifierStr == "return") return tok_return;
            
            return tok_identifier;
        }
        
        // Number: [0-9.]+
        if (isdigit(lastChar) || lastChar == '.') {
            std::string numStr;
            bool hasDecimal = false;
            
            do {
                if (lastChar == '.') {
                    if (hasDecimal) break;
                    hasDecimal = true;
                }
                numStr += lastChar;
                lastChar = getChar();
            } while (isdigit(lastChar) || lastChar == '.');
            
            if (hasDecimal) {
                floatVal = strtod(numStr.c_str(), nullptr);
                return tok_float_literal;
            } else {
                numVal = strtoll(numStr.c_str(), nullptr, 10);
                return tok_number;
            }
        }
        
        // Comment
        if (lastChar == '/') {
            lastChar = getChar();
            if (lastChar == '/') {
                // Line comment
                do {
                    lastChar = getChar();
                } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');
                
                if (lastChar != EOF) {
                    return gettok();
                }
            } else {
                return '/';
            }
        }
        
        // Two-character operators
        if (lastChar == '=') {
            lastChar = getChar();
            if (lastChar == '=') {
                lastChar = getChar();
                return tok_eq;
            }
            return '=';
        }
        
        if (lastChar == '!') {
            lastChar = getChar();
            if (lastChar == '=') {
                lastChar = getChar();
                return tok_ne;
            }
            return '!';
        }
        
        if (lastChar == '<') {
            lastChar = getChar();
            if (lastChar == '=') {
                lastChar = getChar();
                return tok_le;
            }
            return '<';
        }
        
        if (lastChar == '>') {
            lastChar = getChar();
            if (lastChar == '=') {
                lastChar = getChar();
                return tok_ge;
            }
            return '>';
        }
        
        // EOF
        if (lastChar == EOF) {
            return tok_eof;
        }
        
        // Otherwise, return the character
        int thisChar = lastChar;
        lastChar = getChar();
        return thisChar;
    }
};

#endif // LEXER_H

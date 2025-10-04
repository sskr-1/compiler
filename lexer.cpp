#include "lexer.h"
#include <cctype>
#include <map>

// Static member definitions
std::string Lexer::IdentifierStr;
double Lexer::NumVal;
int Lexer::CurTok;

Lexer::Lexer(const std::string& source) : input(source), position(0) {
    if (!input.empty()) {
        currentChar = input[0];
    } else {
        currentChar = '\0';
    }
}

void Lexer::setInput(const std::string& source) {
    input = source;
    position = 0;
    if (!input.empty()) {
        currentChar = input[0];
    } else {
        currentChar = '\0';
    }
}

void Lexer::advance() {
    position++;
    if (position >= input.length()) {
        currentChar = '\0';
    } else {
        currentChar = input[position];
    }
}

void Lexer::skipWhitespace() {
    while (isspace(currentChar)) {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar == '/' && position + 1 < input.length()) {
        if (input[position + 1] == '/') {
            // Single line comment
            while (currentChar != '\0' && currentChar != '\n') {
                advance();
            }
        } else if (input[position + 1] == '*') {
            // Multi-line comment
            advance(); // skip first /
            advance(); // skip *
            
            while (currentChar != '\0') {
                if (currentChar == '*' && position + 1 < input.length() && input[position + 1] == '/') {
                    advance(); // skip *
                    advance(); // skip /
                    break;
                }
                advance();
            }
        }
    }
}

std::string Lexer::readNumber() {
    std::string numStr;
    
    while (isdigit(currentChar) || currentChar == '.') {
        numStr += currentChar;
        advance();
    }
    
    return numStr;
}

std::string Lexer::readIdentifier() {
    std::string identifier;
    
    while (isalnum(currentChar) || currentChar == '_') {
        identifier += currentChar;
        advance();
    }
    
    return identifier;
}

int Lexer::getNextToken() {
    static std::map<std::string, int> keywords = {
        {"def", tok_def},
        {"extern", tok_extern},
        {"if", tok_if},
        {"then", tok_then},
        {"else", tok_else},
        {"for", tok_for},
        {"while", tok_while},
        {"return", tok_return},
        {"int", tok_int},
        {"double", tok_double},
        {"void", tok_void},
        {"bool", tok_bool}
    };
    
    // Skip whitespace
    skipWhitespace();
    
    // Check for comments
    if (currentChar == '/' && position + 1 < input.length() && 
        (input[position + 1] == '/' || input[position + 1] == '*')) {
        skipComment();
        return getNextToken(); // Recursively get next token after comment
    }
    
    // End of file
    if (currentChar == '\0') {
        return tok_eof;
    }
    
    // Numbers
    if (isdigit(currentChar)) {
        std::string numStr = readNumber();
        NumVal = std::stod(numStr);
        return tok_number;
    }
    
    // Identifiers and keywords
    if (isalpha(currentChar) || currentChar == '_') {
        IdentifierStr = readIdentifier();
        
        // Check if it's a keyword
        auto it = keywords.find(IdentifierStr);
        if (it != keywords.end()) {
            return it->second;
        }
        
        return tok_identifier;
    }
    
    // Two-character operators
    if (currentChar == '=' && position + 1 < input.length() && input[position + 1] == '=') {
        advance();
        advance();
        return tok_eq;
    }
    
    if (currentChar == '!' && position + 1 < input.length() && input[position + 1] == '=') {
        advance();
        advance();
        return tok_ne;
    }
    
    if (currentChar == '<' && position + 1 < input.length() && input[position + 1] == '=') {
        advance();
        advance();
        return tok_le;
    }
    
    if (currentChar == '>' && position + 1 < input.length() && input[position + 1] == '=') {
        advance();
        advance();
        return tok_ge;
    }
    
    // Single character tokens
    char thisChar = currentChar;
    advance();
    
    switch (thisChar) {
        case '=': return tok_assign;
        case '<': return tok_lt;
        case '>': return tok_gt;
        case ';': return tok_semicolon;
        case ',': return tok_comma;
        case '(': return tok_lparen;
        case ')': return tok_rparen;
        case '{': return tok_lbrace;
        case '}': return tok_rbrace;
        case '+': case '-': case '*': case '/':
            return thisChar; // Return the character itself for basic operators
        default:
            // Unknown character, skip it
            return getNextToken();
    }
}
#include "lexer.h"
#include <iostream>

int main() {
    std::string input = "main() { return 42; }";
    clike::Lexer lexer(input);
    
    // Simulate parser state
    clike::Token currentToken(clike::END_OF_FILE, "");
    currentToken = lexer.getNextToken();
    
    std::cout << "Parsing function..." << std::endl;
    
    // Parse function name
    if (currentToken.type == clike::IDENTIFIER) {
        std::cout << "Function name: " << currentToken.value << std::endl;
        currentToken = lexer.getNextToken();
    }
    
    // Parse opening parenthesis
    if (currentToken.type == clike::LEFT_PAREN) {
        std::cout << "Found opening parenthesis" << std::endl;
        currentToken = lexer.getNextToken();
    }
    
    // Parse closing parenthesis
    if (currentToken.type == clike::RIGHT_PAREN) {
        std::cout << "Found closing parenthesis" << std::endl;
        currentToken = lexer.getNextToken();
    }
    
    // Parse opening brace
    if (currentToken.type == clike::LEFT_BRACE) {
        std::cout << "Found opening brace" << std::endl;
        currentToken = lexer.getNextToken();
    }
    
    // Parse statements
    while (currentToken.type != clike::RIGHT_BRACE && currentToken.type != clike::END_OF_FILE) {
        std::cout << "Parsing statement, current token: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
        
        if (currentToken.type == clike::RETURN) {
            std::cout << "Found return statement" << std::endl;
            currentToken = lexer.getNextToken();
            std::cout << "After return: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
            
            if (currentToken.type == clike::NUMBER) {
                std::cout << "Found return value: " << currentToken.value << std::endl;
                currentToken = lexer.getNextToken();
            }
            
            if (currentToken.type == clike::SEMICOLON) {
                std::cout << "Found semicolon" << std::endl;
                currentToken = lexer.getNextToken();
            }
        } else {
            std::cout << "Unknown statement type: " << currentToken.type << std::endl;
            break;
        }
    }
    
    if (currentToken.type == clike::RIGHT_BRACE) {
        std::cout << "Found closing brace" << std::endl;
    }
    
    return 0;
}
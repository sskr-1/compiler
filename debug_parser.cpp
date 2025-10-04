#include "lexer.h"
#include <iostream>

int main() {
    std::string input = "add(a, b) { return a + b; }";
    clike::Lexer lexer(input);
    
    // Simulate parser initialization
    clike::Token currentToken(clike::END_OF_FILE, "");
    currentToken = lexer.getNextToken();
    
    std::cout << "Initial token: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
    
    // Simulate parseFunction logic
    if (currentToken.type == clike::IDENTIFIER) {
        std::cout << "Found function name: " << currentToken.value << std::endl;
        currentToken = lexer.getNextToken();
        std::cout << "After advance: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
        
        if (currentToken.type == clike::LEFT_PAREN) {
            std::cout << "Found opening parenthesis" << std::endl;
            currentToken = lexer.getNextToken();
            std::cout << "After advance: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
            
            if (currentToken.type != clike::RIGHT_PAREN) {
                std::cout << "Parsing parameters..." << std::endl;
                do {
                    if (currentToken.type != clike::IDENTIFIER) {
                        std::cout << "ERROR: Expected parameter name, got: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
                        break;
                    }
                    std::cout << "Found parameter: " << currentToken.value << std::endl;
                    currentToken = lexer.getNextToken();
                    std::cout << "After advance: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
                } while (currentToken.type == clike::COMMA);
            }
        } else {
            std::cout << "ERROR: Expected LEFT_PAREN, got: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
        }
    } else {
        std::cout << "ERROR: Expected IDENTIFIER, got: " << currentToken.type << " Value: '" << currentToken.value << "'" << std::endl;
    }
    
    return 0;
}
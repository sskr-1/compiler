#include "lexer.h"
#include <iostream>

int main() {
    std::string input = "add(a, b) { return a + b; }";
    clike::Lexer lexer(input);
    
    // Test the parser logic manually
    clike::Token token = lexer.getNextToken();
    std::cout << "First token: " << token.type << " Value: '" << token.value << "'" << std::endl;
    
    if (token.type == clike::IDENTIFIER) {
        std::cout << "Function name: " << token.value << std::endl;
        token = lexer.getNextToken();
        std::cout << "Next token: " << token.type << " Value: '" << token.value << "'" << std::endl;
        
        if (token.type == clike::LEFT_PAREN) {
            std::cout << "Found opening parenthesis" << std::endl;
            token = lexer.getNextToken();
            std::cout << "Next token: " << token.type << " Value: '" << token.value << "'" << std::endl;
            
            if (token.type == clike::IDENTIFIER) {
                std::cout << "First parameter: " << token.value << std::endl;
            }
        }
    }
    
    return 0;
}
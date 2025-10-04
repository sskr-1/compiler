#include "lexer.h"
#include <iostream>

int main() {
    std::string input = "add(a, b) { return a + b; }";
    clike::Lexer lexer(input);
    
    clike::Token token = lexer.getNextToken();
    while (token.type != clike::END_OF_FILE) {
        std::cout << "Token: " << token.type << " Value: '" << token.value << "'" << std::endl;
        token = lexer.getNextToken();
    }
    
    return 0;
}
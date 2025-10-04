#include "lexer.h"
#include <iostream>

int main() {
    std::string input = "main() { return 42; }";
    clike::Lexer lexer(input);
    
    clike::Token token = lexer.getNextToken();
    while (token.type != clike::END_OF_FILE) {
        std::cout << "Token: " << token.type << " Value: '" << token.value << "'" << std::endl;
        token = lexer.getNextToken();
    }
    
    return 0;
}
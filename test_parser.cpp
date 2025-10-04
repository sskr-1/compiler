#include "lexer.h"
#include "parser.h"
#include <iostream>

int main() {
    std::string input = "add(a, b) { return a + b; }";
    clike::Lexer lexer(input);
    clike::Parser parser(lexer);
    
    auto program = parser.parseProgram();
    if (program) {
        std::cout << "Parse successful!" << std::endl;
    } else {
        std::cout << "Parse failed!" << std::endl;
    }
    
    return 0;
}
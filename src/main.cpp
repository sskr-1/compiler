#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "usage: ccl <file.c>\n";
    return 1;
  }

  std::ifstream in(argv[1]);
  if (!in) { std::cerr << "could not open file\n"; return 1; }
  std::stringstream buffer; buffer << in.rdbuf();
  std::string source = buffer.str();

  try {
    Lexer lex(source);
    Parser parser(lex);
    auto tu = parser.parseTranslationUnit();

    CodeGen cg("module");
    std::string ir = cg.emitIR(*tu);
    std::cout << ir << std::endl;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}

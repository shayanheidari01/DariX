#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: darix <file.dax>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Lexical analysis
    darix::Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    // Parsing
    darix::Parser parser(tokens);
    auto statements = parser.parse();

    // Interpretation
    darix::Interpreter interpreter;
    interpreter.interpret(statements);

    return 0;
}

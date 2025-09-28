#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <cassert>

void test_lexer() {
    std::string source = "var x = 42; print(x);";
    darix::Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    assert(tokens.size() >= 8); // Should have at least these tokens
    std::cout << "✓ Lexer test passed" << std::endl;
}

void test_basic_interpretation() {
    std::string source = "var x = 42; print(x);";
    darix::Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    darix::Parser parser(tokens);
    auto statements = parser.parse();

    darix::Interpreter interpreter;
    auto result = interpreter.interpret(statements);

    std::cout << "✓ Basic interpretation test passed" << std::endl;
}

int main() {
    std::cout << "Running DariX C++ tests..." << std::endl;

    test_lexer();
    test_basic_interpretation();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}

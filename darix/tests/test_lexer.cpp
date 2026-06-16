#include "lexer/lexer.hpp"
#include <iostream>
#include <cassert>

using namespace darix;

void testSimpleExpression() {
    std::cout << "Testing simple expression... ";
    Lexer lexer("1 + 2");
    auto tokens = lexer.tokenize();
    
    assert(!lexer.hasErrors());
    assert(tokens[0].type == TokenType::INTEGER);
    assert(tokens[1].type == TokenType::PLUS);
    assert(tokens[2].type == TokenType::INTEGER);
    assert(tokens[3].type == TokenType::EOF_TOKEN);
    
    std::cout << "PASSED\n";
}

void testStringLiteral() {
    std::cout << "Testing string literal... ";
    Lexer lexer("\"Hello, World!\"");
    auto tokens = lexer.tokenize();
    
    assert(!lexer.hasErrors());
    assert(tokens[0].type == TokenType::STRING);
    assert(std::get<std::string>(tokens[0].value.value) == "Hello, World!");
    
    std::cout << "PASSED\n";
}

void testKeywords() {
    std::cout << "Testing keywords... ";
    Lexer lexer("var x = 10\nfn test() { return x }");
    auto tokens = lexer.tokenize();
    
    assert(!lexer.hasErrors());
    assert(tokens[0].type == TokenType::VAR);
    assert(tokens[2].type == TokenType::ASSIGN);
    assert(tokens[4].type == TokenType::FN);
    
    std::cout << "PASSED\n";
}

void testOperators() {
    std::cout << "Testing operators... ";
    Lexer lexer("a == b && c != d || e ?? f");
    auto tokens = lexer.tokenize();
    
    assert(!lexer.hasErrors());
    assert(tokens[1].type == TokenType::EQ);
    assert(tokens[3].type == TokenType::AND);
    assert(tokens[5].type == TokenType::NEQ);
    assert(tokens[7].type == TokenType::OR);
    assert(tokens[9].type == TokenType::QUESTION_EQ);
    
    std::cout << "PASSED\n";
}

void testNullAware() {
    std::cout << "Testing null-aware operators... ";
    Lexer lexer("obj?.prop ?? defaultValue");
    auto tokens = lexer.tokenize();
    
    assert(!lexer.hasErrors());
    assert(tokens[1].type == TokenType::QUESTION_DOT);
    assert(tokens[3].type == TokenType::QUESTION_EQ);
    
    std::cout << "PASSED\n";
}

void testComments() {
    std::cout << "Testing comments... ";
    Lexer lexer("# This is a comment\nvar x = 1");
    auto tokens = lexer.tokenize();
    
    assert(!lexer.hasErrors());
    // Comment should be skipped
    assert(tokens[0].type == TokenType::VAR);
    
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "\n=== DariX Lexer Tests ===\n\n";
    
    testSimpleExpression();
    testStringLiteral();
    testKeywords();
    testOperators();
    testNullAware();
    testComments();
    
    std::cout << "\nAll lexer tests passed!\n\n";
    return 0;
}

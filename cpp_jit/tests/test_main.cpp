#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <cassert>

namespace darix {

void test_lexer_basic() {
    std::string source = "var x = 42; print(x);";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    // Should have: var, x, =, 42, ;, print, (, x, ), ;, EOF
    assert(tokens.size() >= 10);
    assert(tokens[0].type == TokenType::VAR);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[2].type == TokenType::EQUAL);
    assert(tokens[3].type == TokenType::NUMBER);
    assert(tokens[4].type == TokenType::SEMICOLON);
    
    std::cout << "✓ Lexer basic test passed" << std::endl;
}

void test_lexer_keywords() {
    std::string source = "class func var if else while for return try catch finally true false null";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    assert(tokens[0].type == TokenType::CLASS);
    assert(tokens[1].type == TokenType::FUNC);
    assert(tokens[2].type == TokenType::VAR);
    assert(tokens[3].type == TokenType::IF);
    assert(tokens[4].type == TokenType::ELSE);
    assert(tokens[5].type == TokenType::WHILE);
    assert(tokens[6].type == TokenType::FOR);
    assert(tokens[7].type == TokenType::RETURN);
    assert(tokens[8].type == TokenType::TRY);
    assert(tokens[9].type == TokenType::CATCH);
    assert(tokens[10].type == TokenType::FINALLY);
    assert(tokens[11].type == TokenType::TRUE);
    assert(tokens[12].type == TokenType::FALSE);
    assert(tokens[13].type == TokenType::NULL_);
    
    std::cout << "✓ Lexer keywords test passed" << std::endl;
}

void test_lexer_operators() {
    std::string source = "+ - * / % == != < <= > >= && || !";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    assert(tokens[0].type == TokenType::PLUS);
    assert(tokens[1].type == TokenType::MINUS);
    assert(tokens[2].type == TokenType::MULTIPLY);
    assert(tokens[3].type == TokenType::DIVIDE);
    assert(tokens[4].type == TokenType::MODULO);
    assert(tokens[5].type == TokenType::EQUAL_EQUAL);
    assert(tokens[6].type == TokenType::BANG_EQUAL);
    assert(tokens[7].type == TokenType::LESS);
    assert(tokens[8].type == TokenType::LESS_EQUAL);
    assert(tokens[9].type == TokenType::GREATER);
    assert(tokens[10].type == TokenType::GREATER_EQUAL);
    assert(tokens[11].type == TokenType::AND);
    assert(tokens[12].type == TokenType::OR);
    assert(tokens[13].type == TokenType::BANG);
    
    std::cout << "✓ Lexer operators test passed" << std::endl;
}

void test_parser_basic() {
    std::string source = "var x = 42;";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    Parser parser(tokens);
    auto statements = parser.parse();
    
    assert(!parser.hasErrors());
    assert(statements.size() == 1);
    assert(dynamic_cast<VarDecl*>(statements[0].get()) != nullptr);
    
    std::cout << "✓ Parser basic test passed" << std::endl;
}

void test_parser_expression() {
    std::string source = "var y = 10 + 20 * 3;";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    Parser parser(tokens);
    auto statements = parser.parse();
    
    assert(!parser.hasErrors());
    assert(statements.size() == 1);
    
    std::cout << "✓ Parser expression test passed" << std::endl;
}

void test_parser_function() {
    std::string source = "func add(a, b) { return a + b; }";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    Parser parser(tokens);
    auto statements = parser.parse();
    
    assert(!parser.hasErrors());
    assert(statements.size() == 1);
    assert(dynamic_cast<FuncDecl*>(statements[0].get()) != nullptr);
    
    std::cout << "✓ Parser function test passed" << std::endl;
}

} // namespace darix

int main() {
    std::cout << "Running DariX JIT v2.0 tests..." << std::endl;
    std::cout << "================================" << std::endl;
    
    darix::test_lexer_basic();
    darix::test_lexer_keywords();
    darix::test_lexer_operators();
    darix::test_parser_basic();
    darix::test_parser_expression();
    darix::test_parser_function();
    
    std::cout << "================================" << std::endl;
    std::cout << "All tests passed!" << std::endl;
    return 0;
}

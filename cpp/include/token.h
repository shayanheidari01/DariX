#pragma once

#include <string>
#include <unordered_map>

namespace darix {

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    PERCENT,
    
    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    
    // Literals
    IDENTIFIER, STRING, NUMBER,
    
    // Keywords
    AND, CLASS, ELSE, FALSE, FOR, FUNC, IF, NULL_LITERAL,
    OR, PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    BREAK, CONTINUE,
    
    // End of file
    EOF_TOKEN
};

class Token {
public:
    TokenType type;
    std::string lexeme;
    std::any literal;
    int line;
    
    Token(TokenType type, const std::string& lexeme, std::any literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}
    
    std::string toString() const {
        std::string result = tokenTypeToString(type);
        if (!lexeme.empty()) {
            result += " " + lexeme;
        }
        return result;
    }
    
private:
    static std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::LEFT_PAREN: return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
            case TokenType::LEFT_BRACE: return "LEFT_BRACE";
            case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
            case TokenType::COMMA: return "COMMA";
            case TokenType::DOT: return "DOT";
            case TokenType::MINUS: return "MINUS";
            case TokenType::PLUS: return "PLUS";
            case TokenType::SEMICOLON: return "SEMICOLON";
            case TokenType::SLASH: return "SLASH";
            case TokenType::STAR: return "STAR";
            case TokenType::PERCENT: return "PERCENT";
            case TokenType::BANG: return "BANG";
            case TokenType::BANG_EQUAL: return "BANG_EQUAL";
            case TokenType::EQUAL: return "EQUAL";
            case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
            case TokenType::GREATER: return "GREATER";
            case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
            case TokenType::LESS: return "LESS";
            case TokenType::LESS_EQUAL: return "LESS_EQUAL";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::STRING: return "STRING";
            case TokenType::NUMBER: return "NUMBER";
            case TokenType::AND: return "AND";
            case TokenType::CLASS: return "CLASS";
            case TokenType::ELSE: return "ELSE";
            case TokenType::FALSE: return "FALSE";
            case TokenType::FOR: return "FOR";
            case TokenType::FUNC: return "FUNC";
            case TokenType::IF: return "IF";
            case TokenType::NULL_LITERAL: return "NULL";
            case TokenType::OR: return "OR";
            case TokenType::PRINT: return "PRINT";
            case TokenType::RETURN: return "RETURN";
            case TokenType::SUPER: return "SUPER";
            case TokenType::THIS: return "THIS";
            case TokenType::TRUE: return "TRUE";
            case TokenType::VAR: return "VAR";
            case TokenType::WHILE: return "WHILE";
            case TokenType::BREAK: return "BREAK";
            case TokenType::CONTINUE: return "CONTINUE";
            case TokenType::EOF_TOKEN: return "EOF";
            default: return "UNKNOWN";
        }
    }
};

} // namespace darix

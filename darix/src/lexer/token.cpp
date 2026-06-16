#include "lexer/token.hpp"
#include <sstream>

namespace darix {

std::string Token::typeName(TokenType type) {
    switch (type) {
        // Literals
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::BOOLEAN: return "BOOLEAN";
        case TokenType::NULL_LITERAL: return "NULL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        
        // Keywords
        case TokenType::VAR: return "var";
        case TokenType::LET: return "let";
        case TokenType::CONST: return "const";
        case TokenType::FN: return "fn";
        case TokenType::ASYNC: return "async";
        case TokenType::AWAIT: return "await";
        case TokenType::RETURN: return "return";
        case TokenType::IF: return "if";
        case TokenType::ELSE: return "else";
        case TokenType::ELIF: return "elif";
        case TokenType::FOR: return "for";
        case TokenType::WHILE: return "while";
        case TokenType::BREAK: return "break";
        case TokenType::CONTINUE: return "continue";
        case TokenType::CLASS: return "class";
        case TokenType::EXTENDS: return "extends";
        case TokenType::MIXIN: return "mixin";
        case TokenType::WITH: return "with";
        case TokenType::SUPER: return "super";
        case TokenType::THIS: return "this";
        case TokenType::TRY: return "try";
        case TokenType::CATCH: return "catch";
        case TokenType::FINALLY: return "finally";
        case TokenType::THROW: return "throw";
        case TokenType::IMPORT: return "import";
        case TokenType::FROM: return "from";
        case TokenType::EXPORT: return "export";
        case TokenType::TYPEDEF: return "typedef";
        case TokenType::ENUM: return "enum";
        case TokenType::SWITCH: return "switch";
        case TokenType::CASE: return "case";
        case TokenType::DEFAULT: return "default";
        case TokenType::MATCH: return "match";
        case TokenType::IN: return "in";
        case TokenType::IS: return "is";
        case TokenType::NOT: return "not";
        case TokenType::AND: return "and";
        case TokenType::OR: return "or";
        case TokenType::TRUE: return "true";
        case TokenType::FALSE: return "false";
        
        // Operators
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::STAR: return "*";
        case TokenType::SLASH: return "/";
        case TokenType::PERCENT: return "%";
        case TokenType::DOUBLE_SLASH: return "//";
        case TokenType::POWER: return "**";
        case TokenType::AMPERSAND: return "&";
        case TokenType::PIPE: return "|";
        case TokenType::CARET: return "^";
        case TokenType::TILDE: return "~";
        case TokenType::BANG: return "!";
        case TokenType::QUESTION: return "?";
        case TokenType::DOT: return ".";
        case TokenType::DOUBLE_DOT: return "..";
        case TokenType::ARROW: return "->";
        case TokenType::FAT_ARROW: return "=>";
        case TokenType::PIPE_OP: return "|>";
        
        // Comparison
        case TokenType::EQ: return "==";
        case TokenType::NEQ: return "!=";
        case TokenType::LT: return "<";
        case TokenType::GT: return ">";
        case TokenType::LTE: return "<=";
        case TokenType::GTE: return ">=";
        case TokenType::SPACESHIP: return "<=>";
        
        // Assignment
        case TokenType::ASSIGN: return "=";
        case TokenType::PLUS_ASSIGN: return "+=";
        case TokenType::MINUS_ASSIGN: return "-=";
        case TokenType::STAR_ASSIGN: return "*=";
        case TokenType::SLASH_ASSIGN: return "/=";
        
        // Null-aware
        case TokenType::QUESTION_DOT: return "?.";
        case TokenType::QUESTION_EQ: return "??";
        case TokenType::BANG_EQ: return "!=";
        
        // Delimiters
        case TokenType::LPAREN: return "(";
        case TokenType::RPAREN: return ")";
        case TokenType::LBRACE: return "{";
        case TokenType::RBRACE: return "}";
        case TokenType::LBRACKET: return "[";
        case TokenType::RBRACKET: return "]";
        case TokenType::COMMA: return ",";
        case TokenType::COLON: return ":";
        case TokenType::SEMICOLON: return ";";
        
        // Special
        case TokenType::AT: return "@";
        case TokenType::HASH: return "#";
        case TokenType::DOLLAR: return "$";
        case TokenType::ELLIPSIS: return "...";
        
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::ERROR: return "ERROR";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
    }
    return "UNKNOWN";
}

std::string Token::getTypeName() const {
    return typeName(type);
}

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token(" << getTypeName();
    
    if (std::holds_alternative<int64_t>(value.value)) {
        oss << ", " << std::get<int64_t>(value.value);
    } else if (std::holds_alternative<double>(value.value)) {
        oss << ", " << std::get<double>(value.value);
    } else if (std::holds_alternative<std::string>(value.value)) {
        oss << ", \"" << std::get<std::string>(value.value) << "\"";
    } else if (std::holds_alternative<bool>(value.value)) {
        oss << ", " << (std::get<bool>(value.value) ? "true" : "false");
    }
    
    oss << ", line " << line << ", col " << column << ")";
    return oss.str();
}

} // namespace darix

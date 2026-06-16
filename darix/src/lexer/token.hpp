#ifndef DARIX_LEXER_TOKEN_HPP
#define DARIX_LEXER_TOKEN_HPP

#include <string>
#include <string_view>
#include <variant>
#include <optional>
#include <cstdint>

namespace darix {

enum class TokenType {
    // Literals
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    NULL_LITERAL,
    IDENTIFIER,
    
    // Keywords
    VAR,
    LET,
    CONST,
    FN,
    ASYNC,
    AWAIT,
    RETURN,
    IF,
    ELSE,
    ELIF,
    FOR,
    WHILE,
    BREAK,
    CONTINUE,
    CLASS,
    EXTENDS,
    MIXIN,
    WITH,
    SUPER,
    THIS,
    TRY,
    CATCH,
    FINALLY,
    THROW,
    IMPORT,
    FROM,
    EXPORT,
    TYPEDEF,
    ENUM,
    SWITCH,
    CASE,
    DEFAULT,
    MATCH,
    IN,
    IS,
    NOT,
    AND,
    OR,
    TRUE,
    FALSE,
    
    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    DOUBLE_SLASH,
    POWER,
    AMPERSAND,
    PIPE,
    CARET,
    TILDE,
    BANG,
    QUESTION,
    DOT,
    DOUBLE_DOT,
    ARROW,
    FAT_ARROW,
    PIPE_OP,
    
    // Comparison
    EQ,
    NEQ,
    LT,
    GT,
    LTE,
    GTE,
    SPACESHIP,
    
    // Assignment
    ASSIGN,
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    STAR_ASSIGN,
    SLASH_ASSIGN,
    
    // Null-aware operators (Dart-inspired)
    QUESTION_DOT,   // ?.
    QUESTION_EQ,    // ??
    BANG_EQ,        // !=
    
    // Delimiters
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    COMMA,
    COLON,
    SEMICOLON,
    
    // Special
    AT,             // Decorator
    HASH,           // Comment marker
    DOLLAR,         // String interpolation
    ELLIPSIS,       // ...
    
    EOF_TOKEN,
    ERROR,
    NEWLINE,
    INDENT,
    DEDENT
};

struct TokenValue {
    std::variant<
        std::monostate,
        int64_t,
        double,
        std::string,
        bool
    > value;
    
    TokenValue() = default;
    TokenValue(int64_t i) : value(i) {}
    TokenValue(double d) : value(d) {}
    TokenValue(std::string s) : value(std::move(s)) {}
    TokenValue(bool b) : value(b) {}
};

class Token {
public:
    TokenType type;
    TokenValue value;
    size_t line;
    size_t column;
    std::string_view source;
    
    Token(TokenType type, size_t line = 0, size_t column = 0, 
          std::string_view source = {}, TokenValue value = {})
        : type(type), value(std::move(value)), line(line), column(column), source(source) {}
    
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] std::string getTypeName() const;
    
    static std::string typeName(TokenType type);
};

} // namespace darix

#endif // DARIX_LEXER_TOKEN_HPP

#include "lexer.h"
#include <array>
#include <cstring>

namespace darix {

// Perfect hash-based keyword lookup table
// Generated using gperf-style minimal perfect hash
namespace {
    constexpr size_t KEYWORD_TABLE_SIZE = 256;
    
    // Keyword entry structure
    struct KeywordEntry {
        std::string_view name;
        TokenType type;
        uint32_t hash;
    };
    
    // Compile-time keyword table
    constexpr std::array<KeywordEntry, 14> KEYWORDS{{
        KeywordEntry{"class", TokenType::CLASS, KEYWORD_HASH("class")},
        KeywordEntry{"func", TokenType::FUNC, KEYWORD_HASH("func")},
        KeywordEntry{"var", TokenType::VAR, KEYWORD_HASH("var")},
        KeywordEntry{"if", TokenType::IF, KEYWORD_HASH("if")},
        KeywordEntry{"else", TokenType::ELSE, KEYWORD_HASH("else")},
        KeywordEntry{"while", TokenType::WHILE, KEYWORD_HASH("while")},
        KeywordEntry{"for", TokenType::FOR, KEYWORD_HASH("for")},
        KeywordEntry{"return", TokenType::RETURN, KEYWORD_HASH("return")},
        KeywordEntry{"try", TokenType::TRY, KEYWORD_HASH("try")},
        KeywordEntry{"catch", TokenType::CATCH, KEYWORD_HASH("catch")},
        KeywordEntry{"finally", TokenType::FINALLY, KEYWORD_HASH("finally")},
        KeywordEntry{"true", TokenType::TRUE, KEYWORD_HASH("true")},
        KeywordEntry{"false", TokenType::FALSE, KEYWORD_HASH("false")},
        KeywordEntry{"null", TokenType::NULL_, KEYWORD_HASH("null")},
    }};
    
    // Simple hash table for keyword lookup
    [[nodiscard]] constexpr TokenType lookupKeyword(std::string_view id) noexcept {
        uint32_t h = hashKeyword(id.data(), id.size());
        for (const auto& kw : KEYWORDS) {
            if (kw.hash == h && kw.name.size() == id.size()) {
                bool match = true;
                for (size_t i = 0; i < id.size(); ++i) {
                    if (kw.name[i] != id[i]) {
                        match = false;
                        break;
                    }
                }
                if (match) return kw.type;
            }
        }
        return TokenType::IDENTIFIER;
    }
}

Lexer::Lexer(std::string_view source) 
    : source_(source) {
    // Pre-allocate tokens vector with estimated capacity
    tokens_.reserve(source.size() / 4);  // Rough estimate: 1 token per 4 chars
}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }
    
    tokens_.emplace_back(TokenType::EOF_TOKEN, "", line_, column_);
    return std::move(tokens_);
}

void Lexer::addToken(TokenType type) {
    std::string_view text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, text, line_, column_ - static_cast<uint32_t>(current_ - start_));
}

void Lexer::addToken(TokenType type, std::string_view literal) {
    tokens_.emplace_back(type, literal, line_, column_ - static_cast<uint32_t>(current_ - start_));
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case ':': addToken(TokenType::COLON); break;
        case '+': addToken(TokenType::PLUS); break;
        case '*': addToken(TokenType::MULTIPLY); break;
        case '/':
            if (match('/')) {
                // Single-line comment - skip to end of line
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                // Multi-line comment
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance(); // consume '*'
                        advance(); // consume '/'
                        break;
                    }
                    if (peek() == '\n') {
                        line_++;
                        column_ = 0;
                    }
                    advance();
                }
            } else {
                addToken(TokenType::DIVIDE);
            }
            break;
        case '%': addToken(TokenType::MODULO); break;
        case '-': 
            if (match('>')) {
                // Arrow operator -> could be added later
                addToken(TokenType::MINUS); // For now, treat as minus
            } else {
                addToken(TokenType::MINUS); 
            }
            break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '&':
            if (match('&')) {
                addToken(TokenType::AND);
            }
            break;
        case '|':
            if (match('|')) {
                addToken(TokenType::OR);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;
        case '\n':
            line_++;
            column_ = 0;
            break;
        case '"': string(); break;
        default:
            if (isDigitChar(c)) {
                number();
            } else if (isAlphaChar(c)) {
                identifier();
            }
            // Ignore unknown characters silently (could add error handling)
            break;
    }
}

void Lexer::string() {
    const size_t stringStart = current_;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\\' && peekNext() != '\0') {
            advance(); // Skip backslash
            advance(); // Skip escaped character
        } else {
            if (peek() == '\n') line_++;
            advance();
        }
    }
    
    if (isAtEnd()) {
        // Unterminated string - error handling would go here
        return;
    }
    
    // The closing "
    advance();
    
    // Extract string value without quotes and handle escapes
    std::string_view raw = source_.substr(stringStart, current_ - start_ - 2);
    
    // For now, pass through raw (proper escape handling would decode \n, \t, etc.)
    std::string value(raw);
    
    // Process escape sequences
    std::string decoded;
    decoded.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            switch (value[i + 1]) {
                case 'n': decoded.push_back('\n'); break;
                case 't': decoded.push_back('\t'); break;
                case 'r': decoded.push_back('\r'); break;
                case '\\': decoded.push_back('\\'); break;
                case '"': decoded.push_back('"'); break;
                default: decoded.push_back(value[i + 1]); break;
            }
            ++i;
        } else {
            decoded.push_back(value[i]);
        }
    }
    
    addToken(TokenType::STRING, std::string_view(decoded));
}

void Lexer::number() {
    while (isDigitChar(peek())) advance();
    
    // Look for a fractional part
    if (peek() == '.' && isDigitChar(peekNext())) {
        advance(); // Consume the "."
        while (isDigitChar(peek())) advance();
    }
    
    // Look for exponent
    if (peek() == 'e' || peek() == 'E') {
        advance();
        if (peek() == '+' || peek() == '-') advance();
        while (isDigitChar(peek())) advance();
    }
    
    std::string_view value = source_.substr(start_, current_ - start_);
    addToken(TokenType::NUMBER, value);
}

void Lexer::identifier() {
    while (isAlphaNumericChar(peek())) advance();
    
    std::string_view text = source_.substr(start_, current_ - start_);
    TokenType type = lookupKeyword(text);
    addToken(type);
}

TokenType Lexer::identifierType(std::string_view identifier) noexcept {
    return lookupKeyword(identifier);
}

} // namespace darix

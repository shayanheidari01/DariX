#include "lexer/lexer.hpp"
#include <cctype>
#include <cmath>
#include <charconv>

namespace darix {

// Initialize keywords map
const std::unordered_map<std::string_view, TokenType> Lexer::keywords_ = {
    {"var", TokenType::VAR},
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"fn", TokenType::FN},
    {"async", TokenType::ASYNC},
    {"await", TokenType::AWAIT},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"elif", TokenType::ELIF},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"class", TokenType::CLASS},
    {"extends", TokenType::EXTENDS},
    {"mixin", TokenType::MIXIN},
    {"with", TokenType::WITH},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"throw", TokenType::THROW},
    {"import", TokenType::IMPORT},
    {"from", TokenType::FROM},
    {"export", TokenType::EXPORT},
    {"typedef", TokenType::TYPEDEF},
    {"enum", TokenType::ENUM},
    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT},
    {"match", TokenType::MATCH},
    {"in", TokenType::IN},
    {"is", TokenType::IS},
    {"not", TokenType::NOT},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_LITERAL}
};

Lexer::Lexer(std::string_view source) : source_(source) {
    indentStack_.push(0);  // Initial indentation level
}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }
    
    // Add pending dedents
    while (indentStack_.size() > 1) {
        indentStack_.pop();
        tokens_.emplace_back(TokenType::DEDENT, line_, column_);
    }
    
    tokens_.emplace_back(TokenType::EOF_TOKEN, line_, column_);
    return tokens_;
}

void Lexer::scanToken() {
    char c = advance();
    
    switch (c) {
        case '(': addToken(TokenType::LPAREN); break;
        case ')': addToken(TokenType::RPAREN); break;
        case '{': addToken(TokenType::LBRACE); break;
        case '}': 
            // Check for pending dedents before closing brace
            while (!pendingDedents_.empty()) {
                tokens_.emplace_back(TokenType::DEDENT, line_, column_);
                pendingDedents_.pop_back();
                indentStack_.pop();
            }
            addToken(TokenType::RBRACE); 
            break;
        case '[': addToken(TokenType::LBRACKET); break;
        case ']': addToken(TokenType::RBRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.':
            if (match('.')) {
                if (match('.')) {
                    addToken(TokenType::ELLIPSIS);
                } else {
                    addToken(TokenType::DOUBLE_DOT);
                }
            } else if (isdigit(peek())) {
                scanNumber();
            } else {
                addToken(TokenType::DOT);
            }
            break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case ':': addToken(TokenType::COLON); break;
        
        case '+':
            addToken(match('=') ? TokenType::PLUS_ASSIGN : TokenType::PLUS);
            break;
        case '-':
            if (match('>')) {
                addToken(TokenType::ARROW);
            } else if (match('=')) {
                addToken(TokenType::MINUS_ASSIGN);
            } else {
                addToken(TokenType::MINUS);
            }
            break;
        case '*':
            if (match('*')) {
                addToken(TokenType::POWER);
            } else if (match('=')) {
                addToken(TokenType::STAR_ASSIGN);
            } else {
                addToken(TokenType::STAR);
            }
            break;
        case '/':
            if (match('/')) {
                addToken(TokenType::DOUBLE_SLASH);
            } else if (match('=')) {
                addToken(TokenType::SLASH_ASSIGN);
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case '%': addToken(TokenType::PERCENT); break;
        case '&': addToken(TokenType::AMPERSAND); break;
        case '|':
            if (match('>')) {
                addToken(TokenType::PIPE_OP);
            } else {
                addToken(TokenType::PIPE);
            }
            break;
        case '^': addToken(TokenType::CARET); break;
        case '~': addToken(TokenType::TILDE); break;
        case '@': addToken(TokenType::AT); break;
        case '$': addToken(TokenType::DOLLAR); break;
        case '#': skipComment(); break;
        
        case '!':
            if (match('=')) {
                addToken(TokenType::NEQ);
            } else {
                addToken(TokenType::BANG);
            }
            break;
        case '?':
            if (match('.')) {
                addToken(TokenType::QUESTION_DOT);
            } else if (match('?')) {
                addToken(TokenType::QUESTION_EQ);
            } else {
                addToken(TokenType::QUESTION);
            }
            break;
        case '=':
            if (match('=')) {
                addToken(TokenType::EQ);
            } else if (match('>')) {
                addToken(TokenType::FAT_ARROW);
            } else {
                addToken(TokenType::ASSIGN);
            }
            break;
        case '<':
            if (match('=')) {
                addToken(match('>') ? TokenType::SPACESHIP : TokenType::LTE);
            } else if (match('>')) {
                addToken(TokenType::SPACESHIP);
            } else {
                addToken(TokenType::LT);
            }
            break;
        case '>':
            addToken(match('=') ? TokenType::GTE : TokenType::GT);
            break;
        
        case '\n':
        case '\r':
            handleNewline();
            break;
            
        case ' ':
        case '\t':
        case '\f':
        case '\v':
            skipWhitespace();
            break;
            
        case '"':
        case '\'':
            scanString();
            break;
            
        default:
            if (isdigit(c)) {
                scanNumber();
            } else if (isalpha(c) || c == '_') {
                scanIdentifier();
            } else {
                errors_.emplace_back(line_, column_, 
                    std::string("Unexpected character: ") + c);
                addToken(TokenType::ERROR);
            }
            break;
    }
}

char Lexer::advance() {
    char c = source_[current_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

void Lexer::addToken(TokenType type, TokenValue value) {
    tokens_.emplace_back(type, line_, column_, getLexeme(), std::move(value));
}

void Lexer::addToken(TokenType type, std::string_view lexeme) {
    TokenValue value;
    if (type == TokenType::IDENTIFIER || type == TokenType::STRING) {
        value.value = std::string(lexeme);
    }
    tokens_.emplace_back(type, line_, column_, lexeme, std::move(value));
}

void Lexer::scanNumber() {
    bool isFloat = false;
    
    // Handle leading dot (e.g., .5)
    if (peek() == '.' && isdigit(peekNext())) {
        advance();  // consume '.'
        isFloat = true;
    }
    
    // Consume integer part
    while (isdigit(peek())) {
        advance();
    }
    
    // Check for decimal part
    if (peek() == '.' && isdigit(peekNext())) {
        advance();  // consume '.'
        isFloat = true;
        while (isdigit(peek())) {
            advance();
        }
    }
    
    // Check for exponent
    char next = peek();
    if (next == 'e' || next == 'E') {
        isFloat = true;
        advance();
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        while (isdigit(peek())) {
            advance();
        }
    }
    
    auto lexeme = getLexeme();
    TokenValue value;
    
    if (isFloat) {
        double d;
        auto result = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), d);
        if (result.ec == std::errc()) {
            value.value = d;
        } else {
            errors_.emplace_back(line_, column_, "Invalid float literal");
            value.value = 0.0;
        }
        addToken(TokenType::FLOAT, std::move(value));
    } else {
        int64_t i;
        auto result = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), i);
        if (result.ec == std::errc()) {
            value.value = i;
        } else {
            errors_.emplace_back(line_, column_, "Invalid integer literal");
            value.value = int64_t(0);
        }
        addToken(TokenType::INTEGER, std::move(value));
    }
}

void Lexer::scanString() {
    char quote = source_[start_];
    bool unterminated = true;
    
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') {
            errors_.emplace_back(line_, column_, "Unterminated string");
            return;
        }
        
        if (peek() == '\\') {
            advance();  // consume backslash
            if (!isAtEnd()) {
                advance();  // consume escaped char
            }
        } else {
            advance();
        }
    }
    
    if (isAtEnd()) {
        errors_.emplace_back(line_, column_, "Unterminated string");
        return;
    }
    
    advance();  // consume closing quote
    
    // Extract string content (without quotes)
    auto lexeme = getLexeme();
    std::string content = std::string(lexeme.substr(1, lexeme.size() - 2));
    
    // Process escape sequences
    std::string processed;
    processed.reserve(content.size());
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\\' && i + 1 < content.size()) {
            char next = content[++i];
            switch (next) {
                case 'n': processed += '\n'; break;
                case 'r': processed += '\r'; break;
                case 't': processed += '\t'; break;
                case '\\': processed += '\\'; break;
                case quote: processed += quote; break;
                case '0': processed += '\0'; break;
                default: processed += next; break;
            }
        } else {
            processed += content[i];
        }
    }
    
    TokenValue value(processed);
    addToken(TokenType::STRING, std::move(value));
}

void Lexer::scanIdentifier() {
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }
    
    auto lexeme = getLexeme();
    
    // Check if it's a keyword
    auto it = keywords_.find(lexeme);
    if (it != keywords_.end()) {
        addToken(it->second);
    } else {
        TokenValue value(std::string(lexeme));
        addToken(TokenType::IDENTIFIER, std::move(value));
    }
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c != ' ' && c != '\t' && c != '\f' && c != '\v') {
            break;
        }
        advance();
    }
}

void Lexer::skipComment() {
    // Single-line comment
    while (peek() != '\n' && !isAtEnd()) {
        advance();
    }
}

void Lexer::handleNewline() {
    // Skip newlines in whitespace
    while (peek() == '\n' || peek() == '\r') {
        advance();
    }
    
    // Calculate indentation
    int indentLevel = 0;
    size_t tempPos = current_;
    while (tempPos < source_.size() && 
           (source_[tempPos] == ' ' || source_[tempPos] == '\t')) {
        if (source_[tempPos] == ' ') {
            indentLevel++;
        } else {
            indentLevel = (indentLevel / 4 + 1) * 4;  // Tab = 4 spaces
        }
        tempPos++;
    }
    
    // Skip if at end or only whitespace remains
    if (tempPos >= source_.size() || source_[tempPos] == '\n' || source_[tempPos] == '#') {
        return;
    }
    
    // Compare with current indent stack
    int currentIndent = indentStack_.top();
    
    if (indentLevel > currentIndent) {
        indentStack_.push(indentLevel);
        tokens_.emplace_back(TokenType::INDENT, line_, column_);
    } else if (indentLevel < currentIndent) {
        while (indentStack_.size() > 1 && indentStack_.top() > indentLevel) {
            indentStack_.pop();
            pendingDedents_.push_back(indentStack_.size());
        }
        
        if (indentStack_.top() != indentLevel) {
            errors_.emplace_back(line_, column_, "Inconsistent indentation");
        }
        
        // Add dedents
        while (!pendingDedents_.empty()) {
            tokens_.emplace_back(TokenType::DEDENT, line_, column_);
            pendingDedents_.pop_back();
        }
    }
}

} // namespace darix

#include "darix/token.hpp"

namespace darix {

const char* TokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::ILLEGAL: return "ILLEGAL";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::IDENT: return "IDENT";
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::ASSIGN: return "=";
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::BANG: return "!";
        case TokenType::OR: return "||";
        case TokenType::AND: return "&&";
        case TokenType::ASTERISK: return "*";
        case TokenType::SLASH: return "/";
        case TokenType::MODULO: return "%";
        case TokenType::LT: return "<";
        case TokenType::GT: return ">";
        case TokenType::LE: return "<=";
        case TokenType::GE: return ">=";
        case TokenType::EQ: return "==";
        case TokenType::NOT_EQ: return "!=";
        case TokenType::COMMA: return ",";
        case TokenType::SEMICOLON: return ";";
        case TokenType::COLON: return ":";
        case TokenType::DOT: return ".";
        case TokenType::AT: return "@";
        case TokenType::LPAREN: return "(";
        case TokenType::RPAREN: return ")";
        case TokenType::LBRACE: return "{";
        case TokenType::RBRACE: return "}";
        case TokenType::LBRACKET: return "[";
        case TokenType::RBRACKET: return "]";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::CLASS: return "CLASS";
        case TokenType::VAR: return "VAR";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NULL_TOKEN: return "NULL";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::ELIF: return "ELIF";
        case TokenType::RETURN: return "RETURN";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::FROM: return "FROM";
        case TokenType::AS: return "AS";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::FINALLY: return "FINALLY";
        case TokenType::THROW: return "THROW";
        case TokenType::RAISE: return "RAISE";
        case TokenType::DEL: return "DEL";
        case TokenType::ASSERT: return "ASSERT";
        case TokenType::PASS: return "PASS";
        case TokenType::AND_KW: return "AND";
        case TokenType::OR_KW: return "OR";
        case TokenType::NOT_KW: return "NOT";
        case TokenType::IN: return "IN";
        case TokenType::IS: return "IS";
        case TokenType::WITH: return "WITH";
        case TokenType::YIELD: return "YIELD";
        case TokenType::GLOBAL: return "GLOBAL";
        case TokenType::NONLOCAL: return "NONLOCAL";
        case TokenType::LAMBDA: return "LAMBDA";
    }
    return "UNKNOWN";
}

struct KeywordEntry {
    std::string literal;
    TokenType type;
};

static const KeywordEntry keywordEntries[] = {
    {"func",    TokenType::FUNCTION},
    {"class",   TokenType::CLASS},
    {"var",     TokenType::VAR},
    {"true",    TokenType::TRUE},
    {"false",   TokenType::FALSE},
    {"if",      TokenType::IF},
    {"else",    TokenType::ELSE},
    {"elif",    TokenType::ELIF},
    {"null",    TokenType::NULL_TOKEN},
    {"return",  TokenType::RETURN},
    {"while",   TokenType::WHILE},
    {"for",     TokenType::FOR},
    {"break",   TokenType::BREAK},
    {"continue",TokenType::CONTINUE},
    {"import",  TokenType::IMPORT},
    {"from",    TokenType::FROM},
    {"as",      TokenType::AS},
    {"try",     TokenType::TRY},
    {"catch",   TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"throw",   TokenType::THROW},
    {"raise",   TokenType::RAISE},
    {"del",     TokenType::DEL},
    {"assert",  TokenType::ASSERT},
    {"pass",    TokenType::PASS},
    {"and",     TokenType::AND_KW},
    {"or",      TokenType::OR_KW},
    {"not",     TokenType::NOT_KW},
    {"in",      TokenType::IN},
    {"is",      TokenType::IS},
    {"with",    TokenType::WITH},
    {"yield",   TokenType::YIELD},
    {"global",  TokenType::GLOBAL},
    {"nonlocal",TokenType::NONLOCAL},
    {"lambda",  TokenType::LAMBDA},
};

static std::unordered_map<std::string, TokenType> keywords;

static void InitKeywords() {
    static bool initialized = false;
    if (!initialized) {
        for (const auto& entry : keywordEntries) {
            keywords[entry.literal] = entry.type;
        }
        initialized = true;
    }
}

TokenType LookupIdent(const std::string& ident) {
    InitKeywords();
    auto it = keywords.find(ident);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENT;
}

void RegisterKeyword(const std::string& literal, TokenType type) {
    InitKeywords();
    keywords[literal] = type;
}

} // namespace darix

#ifndef DARIX_PARSER_PARSER_HPP
#define DARIX_PARSER_PARSER_HPP

#include "parser/ast.hpp"
#include "lexer/token.hpp"
#include <vector>
#include <memory>
#include <string>
#include <unordered_set>

namespace darix {

class ParseError {
public:
    size_t line;
    size_t column;
    std::string message;
    
    ParseError(size_t line, size_t column, std::string message)
        : line(line), column(column), message(std::move(message)) {}
    
    [[nodiscard]] std::string toString() const {
        return "ParseError at line " + std::to_string(line) + 
               ", column " + std::to_string(column) + ": " + message;
    }
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    
    [[nodiscard]] Program parse();
    [[nodiscard]] const std::vector<ParseError>& getErrors() const { return errors_; }
    [[nodiscard]] bool hasErrors() const { return !errors_.empty(); }
    
private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::vector<ParseError> errors_;
    std::vector<std::shared_ptr<Statement>> decorators_;
    
    // Parsing helpers
    [[nodiscard]] Token peek() const;
    [[nodiscard]] Token previous() const;
    [[nodiscard]] Token advance();
    [[nodiscard]] bool check(TokenType type) const;
    [[nodiscard]] bool match(TokenType type);
    [[nodiscard]] bool match(const std::unordered_set<TokenType>& types);
    [[nodiscard]] Token expect(TokenType type, const std::string& message);
    
    void synchronize();
    void error(const Token& token, const std::string& message);
    
    // Main parsing methods
    std::vector<std::shared_ptr<Statement>> parseProgram();
    std::shared_ptr<Statement> parseDeclaration();
    std::shared_ptr<Statement> parseVarDecl(bool isConst = false);
    std::shared_ptr<Statement> parseFunction(const std::string& kind, bool allowSignatureOnly = false);
    std::shared_ptr<Statement> parseClass();
    std::shared_ptr<Statement> parseEnum();
    std::shared_ptr<Statement> parseImport();
    std::shared_ptr<Statement> parseMatch();
    std::shared_ptr<Statement> parseTry();
    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Statement> parseIf();
    std::shared_ptr<Statement> parseFor();
    std::shared_ptr<Statement> parseWhile();
    std::shared_ptr<Statement> parseReturn();
    std::shared_ptr<Statement> parseBlock();
    std::shared_ptr<Statement> parseExpressionStmt();
    std::shared_ptr<Statement> parseAssert();
    std::shared_ptr<Statement> parseThrow();
    
    // Expression parsing (Pratt parsing for operator precedence)
    std::shared_ptr<Expression> parseExpression();
    std::shared_ptr<Expression> parseAssignment();
    std::shared_ptr<Expression> parseLambda();
    std::shared_ptr<Expression> parseOr();
    std::shared_ptr<Expression> parseAnd();
    std::shared_ptr<Expression> parseBitwiseOr();
    std::shared_ptr<Expression> parseBitwiseXor();
    std::shared_ptr<Expression> parseBitwiseAnd();
    std::shared_ptr<Expression> parseEquality();
    std::shared_ptr<Expression> parseRelational();
    std::shared_ptr<Expression> parseNullCoalescing();
    std::shared_ptr<Expression> parseShift();
    std::shared_ptr<Expression> parseAdditive();
    std::shared_ptr<Expression> parseMultiplicative();
    std::shared_ptr<Expression> parseIsAs();
    std::shared_ptr<Expression> parseUnary();
    std::shared_ptr<Expression> parsePipe();
    std::shared_ptr<Expression> parseExponentiation();
    std::shared_ptr<Expression> parseCall();
    std::shared_ptr<Expression> parsePrimary();
    
    // Helper parsing methods
    std::shared_ptr<Expression> parseCallSuffix(std::shared_ptr<Expression> callee);
    std::shared_ptr<Expression> parseGetSuffix(std::shared_ptr<Expression> object);
    std::shared_ptr<Expression> parseIndexSuffix(std::shared_ptr<Expression> object);
    std::shared_ptr<Expression> parseConditionalSuffix(std::shared_ptr<Expression> condition);
    
    std::vector<std::pair<std::string, TypeHintOpt>> parseParameters();
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> parseOptionalParameters();
    TypeHintOpt parseTypeHint();
    std::shared_ptr<Statement> parseFunctionBody();
    
    // Decorator handling
    void parseDecorators();
};

} // namespace darix

#endif // DARIX_PARSER_PARSER_HPP

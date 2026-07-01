#include "darix/parser.hpp"
#include <charconv>
#include <sstream>

namespace darix {

static std::unordered_map<TokenType, int> precedences = {
    {TokenType::ASSIGN,   ASSIGN},
    {TokenType::OR,       OR},
    {TokenType::AND,      AND},
    {TokenType::OR_KW,    OR},
    {TokenType::AND_KW,   AND},
    {TokenType::IN,       EQUALS},
    {TokenType::IS,       EQUALS},
    {TokenType::EQ,       EQUALS},
    {TokenType::NOT_EQ,   EQUALS},
    {TokenType::LT,       LESSGREATER},
    {TokenType::GT,       LESSGREATER},
    {TokenType::LE,       LESSGREATER},
    {TokenType::GE,       LESSGREATER},
    {TokenType::PLUS,     SUM},
    {TokenType::MINUS,    SUM},
    {TokenType::SLASH,    PRODUCT},
    {TokenType::MODULO,   PRODUCT},
    {TokenType::ASTERISK, PRODUCT},
    {TokenType::LPAREN,   CALL},
    {TokenType::DOT,      MEMBER},
    {TokenType::LBRACKET, INDEX},
};

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    registerParseFns();
    nextToken();
    nextToken();
}

void Parser::setReplMode(bool mode) { isReplMode_ = mode; }

void Parser::registerParseFns() {
    // Prefix
    prefixParseFns_[TokenType::IDENT]    = [this]() { return parseIdentifier(); };
    prefixParseFns_[TokenType::INT]      = [this]() { return parseIntegerLiteral(); };
    prefixParseFns_[TokenType::FLOAT]    = [this]() { return parseFloatLiteral(); };
    prefixParseFns_[TokenType::STRING]   = [this]() { return parseStringLiteral(); };
    prefixParseFns_[TokenType::BANG]     = [this]() { return parsePrefixExpression(); };
    prefixParseFns_[TokenType::MINUS]    = [this]() { return parsePrefixExpression(); };
    prefixParseFns_[TokenType::NOT_KW]   = [this]() { return parsePrefixExpression(); };
    prefixParseFns_[TokenType::TRUE]     = [this]() { return parseBoolean(); };
    prefixParseFns_[TokenType::FALSE]    = [this]() { return parseBoolean(); };
    prefixParseFns_[TokenType::NULL_TOKEN] = [this]() { return parseNull(); };
    prefixParseFns_[TokenType::LPAREN]   = [this]() { return parseGroupedExpression(); };
    prefixParseFns_[TokenType::IF]       = [this]() { return parseIfExpression(); };
    prefixParseFns_[TokenType::FUNCTION] = [this]() { return parseFunctionLiteral(); };
    prefixParseFns_[TokenType::LAMBDA]   = [this]() { return parseLambdaExpression(); };
    prefixParseFns_[TokenType::WHILE]    = [this]() { return parseWhileExpression(); };
    prefixParseFns_[TokenType::FOR]      = [this]() { return parseForExpression(); };
    prefixParseFns_[TokenType::LBRACKET] = [this]() { return parseArrayLiteral(); };
    prefixParseFns_[TokenType::LBRACE]   = [this]() { return parseMapLiteral(); };
    prefixParseFns_[TokenType::YIELD]    = [this]() { return parseYieldExpression(); };

    // Infix
    infixParseFns_[TokenType::ASSIGN]    = [this](auto l) { return parseAssignmentExpression(l); };
    infixParseFns_[TokenType::PLUS]      = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::MINUS]     = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::SLASH]     = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::MODULO]    = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::ASTERISK]  = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::EQ]        = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::NOT_EQ]    = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::LT]        = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::GT]        = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::LE]        = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::GE]        = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::OR]        = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::AND]       = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::OR_KW]     = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::AND_KW]    = [this](auto l) { return parseInfixExpression(l); };
    infixParseFns_[TokenType::IN]        = [this](auto l) { return parseInExpression(l); };
    infixParseFns_[TokenType::IS]        = [this](auto l) { return parseIsExpression(l); };
    infixParseFns_[TokenType::LPAREN]    = [this](auto l) { return parseCallExpression(l); };
    infixParseFns_[TokenType::LBRACKET]  = [this](auto l) { return parseIndexExpression(l); };
    infixParseFns_[TokenType::DOT]       = [this](auto l) { return parseMemberExpression(l); };
}

void Parser::nextToken() {
    curToken_ = peekToken_;
    peekToken_ = lexer_.nextToken();
}

const std::vector<std::string>& Parser::errors() const { return errors_; }

std::shared_ptr<Program> Parser::parseProgram() {
    auto program = std::make_shared<Program>();
    program->tag = NodeType::PROGRAM;
    while (curToken_.type != TokenType::EOF_TOKEN) {
        if (auto stmt = parseStatement()) {
            program->statements.push_back(stmt);
        }
        nextToken();
    }
    return program;
}

StatementPtr Parser::parseStatement() {
    if (curToken_.type == TokenType::ILLEGAL) {
        addError("illegal token: " + curToken_.literal);
        nextToken();
        return nullptr;
    }

    switch (curToken_.type) {
        case TokenType::IMPORT:    return parseImportStatement();
        case TokenType::CLASS:     return parseClassDeclaration();
        case TokenType::FUNCTION:  return parseFunctionDeclaration();
        case TokenType::VAR:       return parseLetStatement();
        case TokenType::RETURN:    return parseReturnStatement();
        case TokenType::WHILE:     return parseWhileStatement();
        case TokenType::FOR:       return parseForStatement();
        case TokenType::BREAK:     return parseBreakStatement();
        case TokenType::CONTINUE:  return parseContinueStatement();
        case TokenType::TRY:       return parseTryStatement();
        case TokenType::THROW:
        case TokenType::RAISE:     return parseThrowStatement();
        case TokenType::DEL:       return parseDelStatement();
        case TokenType::ASSERT:    return parseAssertStatement();
        case TokenType::PASS:      return parsePassStatement();
        case TokenType::GLOBAL:    return parseGlobalStatement();
        case TokenType::NONLOCAL:  return parseNonlocalStatement();
        case TokenType::AT:        return parseDecoratedDefinition();
        case TokenType::WITH:      return parseWithStatement();
        case TokenType::LBRACE:    return parseBlockStatementAsStatement();
        case TokenType::IDENT:
            if (isAssignment()) return parseAssignStatement();
            return parseExpressionStatement();
        case TokenType::RBRACE:
        case TokenType::SEMICOLON:
            return nullptr;
        default:
            return parseExpressionStatement();
    }
}

bool Parser::isAssignment() {
    return peekToken_.type == TokenType::ASSIGN;
}

ExpressionPtr Parser::parseExpression(int precedence) {
    auto it = prefixParseFns_.find(curToken_.type);
    if (it == prefixParseFns_.end()) {
        addError("no prefix parse function for " + std::string(TokenTypeToString(curToken_.type)) + " found");
        return nullptr;
    }

    auto leftExp = it->second();
    if (!leftExp) return nullptr;

    while (peekPrecedence() > precedence) {
        auto it2 = infixParseFns_.find(peekToken_.type);
        if (it2 == infixParseFns_.end()) break;
        nextToken();
        leftExp = it2->second(leftExp);
        if (!leftExp) break;
    }

    return leftExp;
}

// ============ Prefix parse functions ============

ExpressionPtr Parser::parseIdentifier() {
    auto node = std::make_shared<Identifier>();
    node->tag = NodeType::IDENTIFIER;
    node->token = curToken_;
    node->value = curToken_.literal;
    return node;
}

ExpressionPtr Parser::parseIntegerLiteral() {
    auto node = std::make_shared<IntegerLiteral>();
    node->tag = NodeType::INTEGER_LITERAL;
    node->token = curToken_;
    auto [ptr, ec] = std::from_chars(curToken_.literal.data(), curToken_.literal.data() + curToken_.literal.size(), node->value);
    if (ec != std::errc()) {
        addError("could not parse \"" + curToken_.literal + "\" as integer");
        return nullptr;
    }
    return node;
}

ExpressionPtr Parser::parseFloatLiteral() {
    auto node = std::make_shared<FloatLiteral>();
    node->token = curToken_;
    try {
        node->value = std::stod(curToken_.literal);
    } catch (...) {
        addError("could not parse \"" + curToken_.literal + "\" as float");
        return nullptr;
    }
    return node;
}

ExpressionPtr Parser::parseStringLiteral() {
    auto node = std::make_shared<StringLiteral>();
    node->token = curToken_;
    node->value = curToken_.literal;
    return node;
}

ExpressionPtr Parser::parseBoolean() {
    auto node = std::make_shared<BooleanLiteral>();
    node->token = curToken_;
    node->value = (curToken_.type == TokenType::TRUE);
    return node;
}

ExpressionPtr Parser::parseNull() {
    auto node = std::make_shared<NullLiteral>();
    node->token = curToken_;
    return node;
}

ExpressionPtr Parser::parsePrefixExpression() {
    auto node = std::make_shared<PrefixExpression>();
    node->token = curToken_;
    node->op = curToken_.literal;
    if (curToken_.type == TokenType::NOT_KW) node->op = "!";
    nextToken();
    node->right = parseExpression(PREFIX);
    return node;
}

ExpressionPtr Parser::parseGroupedExpression() {
    nextToken();
    auto exp = parseExpression(LOWEST);
    if (!expectPeek(TokenType::RPAREN)) return nullptr;
    return exp;
}

ExpressionPtr Parser::parseIfExpression() {
    auto expr = std::make_shared<IfExpression>();
    expr->tag = NodeType::IF_EXPRESSION;
    expr->token = curToken_;

    if (!expectPeek(TokenType::LPAREN)) return nullptr;
    nextToken();
    expr->condition = parseExpression(LOWEST);
    if (!expectPeek(TokenType::RPAREN) || !expectPeek(TokenType::LBRACE)) return nullptr;
    expr->consequence = parseBlockStatement();

    if (peekTokenIs(TokenType::ELSE)) {
        nextToken();
        if (peekTokenIs(TokenType::IF)) {
            nextToken();
            expr->alternative = parseIfExpression();
        } else {
            if (!expectPeek(TokenType::LBRACE)) return nullptr;
            expr->alternative = parseBlockStatement();
        }
    } else if (peekTokenIs(TokenType::ELIF)) {
        nextToken();
        expr->alternative = parseIfExpression();
    }

    return expr;
}

ExpressionPtr Parser::parseFunctionLiteral() {
    auto lit = std::make_shared<FunctionLiteral>();
    lit->token = curToken_;

    if (!expectPeek(TokenType::LPAREN)) return nullptr;
    auto params = parseFunctionParameters();
    lit->parameters = params;
    if (!expectPeek(TokenType::LBRACE)) return nullptr;
    lit->body = parseBlockStatement();
    return lit;
}

ExpressionPtr Parser::parseArrayLiteral() {
    auto array = std::make_shared<ArrayLiteral>();
    array->token = curToken_;
    nextToken();

    if (curTokenIs(TokenType::RBRACKET)) {
        array->elements = {};
        return array;
    }

    array->elements = parseExpressionList(TokenType::RBRACKET);
    return array;
}

ExpressionPtr Parser::parseMapLiteral() {
    auto lit = std::make_shared<MapLiteral>();
    lit->token = curToken_;

    if (peekTokenIs(TokenType::RBRACE)) {
        nextToken();
        return lit;
    }

    nextToken();
    for (;;) {
        auto key = parseExpression(LOWEST);
        if (!key || !expectPeek(TokenType::COLON)) return nullptr;
        nextToken();
        auto value = parseExpression(LOWEST);
        if (!value) return nullptr;
        lit->pairs.push_back({key, value});

        if (!peekTokenIs(TokenType::COMMA)) break;
        nextToken(); // comma
        nextToken(); // next key
    }

    if (!expectPeek(TokenType::RBRACE)) return nullptr;
    return lit;
}

ExpressionPtr Parser::parseWhileExpression() {
    auto stmt = parseWhileStatement();
    if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
        auto expr = std::make_shared<WhileExpression>();
        expr->token = whileStmt->token;
        expr->condition = whileStmt->condition;
        expr->body = whileStmt->body;
        return expr;
    }
    addError("expected while statement");
    return nullptr;
}

ExpressionPtr Parser::parseForExpression() {
    auto stmt = parseForStatement();
    if (auto block = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        return block;
    }
    if (auto forStmt = std::dynamic_pointer_cast<ForStatement>(stmt)) {
        auto block = std::make_shared<BlockStatement>();
        block->token = forStmt->token;
        block->statements.push_back(forStmt);
        return block;
    }
    if (stmt) {
        auto block = std::make_shared<BlockStatement>();
        block->token = curToken_;
        block->statements.push_back(stmt);
        return block;
    }
    return nullptr;
}

ExpressionPtr Parser::parseLambdaExpression() {
    auto expr = std::make_shared<LambdaExpression>();
    expr->token = curToken_;

    if (peekTokenIs(TokenType::IDENT)) {
        nextToken();
        auto ident = std::make_shared<Identifier>();
        ident->token = curToken_;
        ident->value = curToken_.literal;
        expr->parameters.push_back(ident);

        while (peekTokenIs(TokenType::COMMA)) {
            nextToken(); // comma
            if (!expectPeek(TokenType::IDENT)) return nullptr;
            auto p = std::make_shared<Identifier>();
            p->token = curToken_;
            p->value = curToken_.literal;
            expr->parameters.push_back(p);
        }
    }

    if (!expectPeek(TokenType::COLON)) return nullptr;
    nextToken();
    expr->body = parseExpression(LOWEST);
    return expr;
}

ExpressionPtr Parser::parseYieldExpression() {
    auto expr = std::make_shared<YieldExpression>();
    expr->token = curToken_;

    if (!peekTokenIs(TokenType::SEMICOLON) && !peekTokenIs(TokenType::RBRACE) && !peekTokenIs(TokenType::EOF_TOKEN)) {
        nextToken();
        expr->value = parseExpression(LOWEST);
    }

    return expr;
}

// ============ Infix parse functions ============

ExpressionPtr Parser::parseInfixExpression(ExpressionPtr left) {
    auto expr = std::make_shared<InfixExpression>();
    expr->tag = NodeType::INFIX_EXPRESSION;
    expr->token = curToken_;
    expr->op = curToken_.literal;
    expr->left = left;

    int prec = curPrecedence();
    nextToken();
    expr->right = parseExpression(prec);
    return expr;
}

ExpressionPtr Parser::parseCallExpression(ExpressionPtr fn) {
    auto exp = std::make_shared<CallExpression>();
    exp->tag = NodeType::CALL_EXPRESSION;
    exp->token = curToken_;
    exp->function = fn;
    nextToken();
    exp->arguments = parseExpressionList(TokenType::RPAREN);
    return exp;
}

ExpressionPtr Parser::parseIndexExpression(ExpressionPtr left) {
    auto exp = std::make_shared<IndexExpression>();
    exp->token = curToken_;
    exp->left = left;
    nextToken();
    exp->index = parseExpression(LOWEST);
    if (!expectPeek(TokenType::RBRACKET)) return nullptr;
    return exp;
}

ExpressionPtr Parser::parseMemberExpression(ExpressionPtr left) {
    auto exp = std::make_shared<MemberExpression>();
    exp->token = curToken_;
    exp->left = left;
    if (!expectPeek(TokenType::IDENT)) return nullptr;
    auto prop = std::make_shared<Identifier>();
    prop->token = curToken_;
    prop->value = curToken_.literal;
    exp->property = prop;
    return exp;
}

ExpressionPtr Parser::parseAssignmentExpression(ExpressionPtr left) {
    if (!isValidAssignmentTarget(left)) {
        addError("invalid assignment target");
        return nullptr;
    }

    auto expr = std::make_shared<AssignExpression>();
    expr->token = curToken_;
    expr->name = left;
    nextToken();
    expr->value = parseExpression(LOWEST);
    return expr;
}

ExpressionPtr Parser::parseInExpression(ExpressionPtr left) {
    auto expr = std::make_shared<InExpression>();
    expr->token = curToken_;
    expr->left = left;
    int prec = curPrecedence();
    nextToken();
    expr->right = parseExpression(prec);
    return expr;
}

ExpressionPtr Parser::parseIsExpression(ExpressionPtr left) {
    auto expr = std::make_shared<IsExpression>();
    expr->token = curToken_;
    expr->left = left;
    int prec = curPrecedence();
    nextToken();
    expr->right = parseExpression(prec);
    return expr;
}

// ============ Statement parse functions ============

StatementPtr Parser::parseLetStatement() {
    auto stmt = std::make_shared<LetStatement>();
    stmt->tag = NodeType::LET_STATEMENT;
    stmt->token = curToken_;

    if (!expectPeek(TokenType::IDENT)) return nullptr;
    auto name = std::make_shared<Identifier>();
    name->token = curToken_;
    name->value = curToken_.literal;
    stmt->name = name;

    if (peekTokenIs(TokenType::ASSIGN)) {
        nextToken(); // ASSIGN
        nextToken(); // value
        stmt->value = parseExpression(LOWEST);
    } else {
        auto nullNode = std::make_shared<NullLiteral>();
        nullNode->token = {TokenType::NULL_TOKEN, "null"};
        stmt->value = nullNode;
    }

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseClassDeclaration() {
    auto stmt = std::make_shared<ClassDeclaration>();
    stmt->token = curToken_;

    if (!expectPeek(TokenType::IDENT)) return nullptr;
    auto name = std::make_shared<Identifier>();
    name->token = curToken_;
    name->value = curToken_.literal;
    stmt->name = name;

    if (!expectPeek(TokenType::LBRACE)) return nullptr;
    stmt->body = parseBlockStatement();
    return stmt;
}

StatementPtr Parser::parseReturnStatement() {
    auto stmt = std::make_shared<ReturnStatement>();
    stmt->tag = NodeType::RETURN_STATEMENT;
    stmt->token = curToken_;

    if (peekTokenIs(TokenType::SEMICOLON) || peekTokenIs(TokenType::RBRACE) || peekTokenIs(TokenType::EOF_TOKEN)) {
        auto nullNode = std::make_shared<NullLiteral>();
        nullNode->token = {TokenType::NULL_TOKEN, "null"};
        stmt->returnValue = nullNode;
    } else {
        nextToken();
        stmt->returnValue = parseExpression(LOWEST);
    }

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseExpressionStatement() {
    auto stmt = std::make_shared<ExpressionStatement>();
    stmt->tag = NodeType::EXPRESSION_STATEMENT;
    stmt->token = curToken_;
    stmt->expression = parseExpression(LOWEST);

    if (auto assignExpr = std::dynamic_pointer_cast<AssignExpression>(stmt->expression)) {
        auto assignStmt = std::make_shared<AssignStatement>();
        assignStmt->token = assignExpr->token;
        assignStmt->target = assignExpr->name;
        assignStmt->value = assignExpr->value;
        consumeOptionalSemicolon();
        return assignStmt;
    }

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseBlockStatementAsStatement() {
    auto block = parseBlockStatement();
    auto sbs = std::make_shared<StandaloneBlockStatement>();
    sbs->token = block->token;
    sbs->block = block;
    return sbs;
}

StatementPtr Parser::parseAssignStatement() {
    auto stmt = std::make_shared<AssignStatement>();
    stmt->tag = NodeType::ASSIGN_STATEMENT;
    stmt->token = curToken_;

    ExpressionPtr target;
    if (curToken_.type == TokenType::IDENT) {
        target = parseIdentifier();
    } else {
        target = parseExpression(INDEX);
    }

    if (!target || !isValidAssignmentTarget(target)) {
        addError("invalid assignment target");
        return nullptr;
    }
    stmt->target = target;

    if (!expectPeek(TokenType::ASSIGN)) return nullptr;
    nextToken();
    stmt->value = parseExpression(LOWEST);

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseWhileStatement() {
    auto stmt = std::make_shared<WhileStatement>();
    stmt->token = curToken_;

    if (!expectPeek(TokenType::LPAREN)) return nullptr;
    nextToken();
    stmt->condition = parseExpression(LOWEST);
    if (!expectPeek(TokenType::RPAREN) || !expectPeek(TokenType::LBRACE)) return nullptr;
    stmt->body = parseBlockStatement();
    return stmt;
}

StatementPtr Parser::parseForStatement() {
    auto stmt = std::make_shared<ForStatement>();
    stmt->token = curToken_;
    if (!expectPeek(TokenType::LPAREN)) return nullptr;
    nextToken();

    // init
    if (curToken_.type != TokenType::SEMICOLON) {
        if (curToken_.type == TokenType::VAR) {
            stmt->init = parseLetStatement();
        } else {
            stmt->init = parseExpressionStatement();
        }
    }
    if (!expectCurrent(TokenType::SEMICOLON)) return nullptr;
    nextToken();

    // condition
    if (curToken_.type != TokenType::SEMICOLON) {
        stmt->condition = parseExpression(LOWEST);
    }
    if (!expectPeek(TokenType::SEMICOLON)) return nullptr;
    nextToken();

    // post
    if (curToken_.type != TokenType::RPAREN) {
        if (curToken_.type == TokenType::IDENT && peekToken_.type == TokenType::ASSIGN) {
            stmt->post = parseAssignStatement();
        } else {
            stmt->post = parseExpressionStatement();
        }
    }

    if (!expectPeek(TokenType::RPAREN) || !expectPeek(TokenType::LBRACE)) return nullptr;
    stmt->body = parseBlockStatement();
    return stmt;
}

StatementPtr Parser::parseBreakStatement() {
    auto stmt = std::make_shared<BreakStatement>();
    stmt->token = curToken_;
    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseContinueStatement() {
    auto stmt = std::make_shared<ContinueStatement>();
    stmt->token = curToken_;
    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseTryStatement() {
    auto stmt = std::make_shared<TryStatement>();
    stmt->token = curToken_;

    if (!expectPeek(TokenType::LBRACE)) return nullptr;
    stmt->tryBlock = parseBlockStatement();

    while (peekTokenIs(TokenType::CATCH)) {
        nextToken();
        auto catchClause = parseCatchClause();
        if (!catchClause) return nullptr;
        stmt->catchClauses.push_back(catchClause);
    }

    if (peekTokenIs(TokenType::FINALLY)) {
        nextToken();
        if (!expectPeek(TokenType::LBRACE)) return nullptr;
        stmt->finallyBlock = parseBlockStatement();
    }

    if (stmt->catchClauses.empty() && !stmt->finallyBlock) {
        addError("try statement must have at least one catch clause or finally block");
        return nullptr;
    }

    return stmt;
}

std::shared_ptr<CatchClause> Parser::parseCatchClause() {
    auto clause = std::make_shared<CatchClause>();
    clause->token = curToken_;

    if (peekTokenIs(TokenType::LPAREN)) {
        nextToken(); // LPAREN
        if (peekTokenIs(TokenType::IDENT)) {
            nextToken();
            auto firstIdent = std::make_shared<Identifier>();
            firstIdent->token = curToken_;
            firstIdent->value = curToken_.literal;

            if (peekTokenIs(TokenType::IDENT)) {
                clause->exceptionType = firstIdent;
                nextToken();
                auto var = std::make_shared<Identifier>();
                var->token = curToken_;
                var->value = curToken_.literal;
                clause->variable = var;
            } else {
                clause->variable = firstIdent;
            }
        }
        if (!expectPeek(TokenType::RPAREN)) return nullptr;
    }

    if (!expectPeek(TokenType::LBRACE)) return nullptr;
    clause->catchBlock = parseBlockStatement();
    return clause;
}

StatementPtr Parser::parseThrowStatement() {
    auto stmt = std::make_shared<ThrowStatement>();
    stmt->token = curToken_;
    nextToken();
    stmt->exception = parseExpression(LOWEST);
    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseImportStatement() {
    auto stmt = std::make_shared<ImportStatement>();
    stmt->token = curToken_;

    // Accept both: import "go:math" and import math
    if (peekTokenIs(TokenType::STRING)) {
        nextToken();
        auto path = std::make_shared<StringLiteral>();
        path->token = curToken_;
        path->value = curToken_.literal;
        stmt->path = path;
    } else if (peekTokenIs(TokenType::IDENT)) {
        nextToken();
        auto path = std::make_shared<StringLiteral>();
        path->token = curToken_;
        path->value = curToken_.literal;
        stmt->path = path;
    } else {
        addError("expected module name or string after import");
        return nullptr;
    }
    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseFunctionDeclaration() {
    auto stmt = std::make_shared<FunctionDeclaration>();
    stmt->token = curToken_;

    if (!expectPeek(TokenType::IDENT)) return nullptr;
    auto name = std::make_shared<Identifier>();
    name->token = curToken_;
    name->value = curToken_.literal;
    stmt->name = name;

    if (!expectPeek(TokenType::LPAREN)) return nullptr;
    stmt->parameters = parseFunctionParameters();
    if (!expectPeek(TokenType::LBRACE)) return nullptr;
    stmt->body = parseBlockStatement();
    return stmt;
}

StatementPtr Parser::parseDelStatement() {
    auto stmt = std::make_shared<DelStatement>();
    stmt->token = curToken_;
    nextToken();
    stmt->target = parseExpression(LOWEST);
    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseAssertStatement() {
    auto stmt = std::make_shared<AssertStatement>();
    stmt->token = curToken_;
    nextToken();
    stmt->condition = parseExpression(LOWEST);

    if (peekTokenIs(TokenType::COMMA)) {
        nextToken(); // comma
        nextToken(); // message
        stmt->message = parseExpression(LOWEST);
    }

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parsePassStatement() {
    auto stmt = std::make_shared<PassStatement>();
    stmt->token = curToken_;
    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseGlobalStatement() {
    auto stmt = std::make_shared<GlobalStatement>();
    stmt->token = curToken_;

    if (!expectPeek(TokenType::IDENT)) return nullptr;
    auto ident = std::make_shared<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.literal;
    stmt->names.push_back(ident);

    while (peekTokenIs(TokenType::COMMA)) {
        nextToken();
        if (!expectPeek(TokenType::IDENT)) return nullptr;
        auto id = std::make_shared<Identifier>();
        id->token = curToken_;
        id->value = curToken_.literal;
        stmt->names.push_back(id);
    }

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseNonlocalStatement() {
    auto stmt = std::make_shared<NonlocalStatement>();
    stmt->token = curToken_;

    if (!expectPeek(TokenType::IDENT)) return nullptr;
    auto ident = std::make_shared<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.literal;
    stmt->names.push_back(ident);

    while (peekTokenIs(TokenType::COMMA)) {
        nextToken();
        if (!expectPeek(TokenType::IDENT)) return nullptr;
        auto id = std::make_shared<Identifier>();
        id->token = curToken_;
        id->value = curToken_.literal;
        stmt->names.push_back(id);
    }

    consumeOptionalSemicolon();
    return stmt;
}

StatementPtr Parser::parseWithStatement() {
    auto stmt = std::make_shared<WithStatement>();
    stmt->token = curToken_;
    nextToken();
    stmt->context = parseExpression(LOWEST);

    if (peekTokenIs(TokenType::AS)) {
        nextToken(); // as
        if (!expectPeek(TokenType::IDENT)) return nullptr;
        auto var = std::make_shared<Identifier>();
        var->token = curToken_;
        var->value = curToken_.literal;
        stmt->variable = var;
    }

    if (!expectPeek(TokenType::LBRACE)) return nullptr;
    stmt->body = parseBlockStatement();
    return stmt;
}

StatementPtr Parser::parseDecoratedDefinition() {
    std::vector<ExpressionPtr> decorators;

    while (curTokenIs(TokenType::AT)) {
        nextToken(); // skip @
        auto decorator = parseExpression(LOWEST);
        if (decorator) decorators.push_back(decorator);
        consumeOptionalSemicolon();
    }

    StatementPtr def;
    if (curTokenIs(TokenType::FUNCTION)) {
        def = parseFunctionDeclaration();
    } else if (curTokenIs(TokenType::CLASS)) {
        def = parseClassDeclaration();
    } else {
        addError("expected function or class declaration after decorator");
        return nullptr;
    }

    if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclaration>(def)) {
        funcDecl->decorators = decorators;
    } else if (auto classDecl = std::dynamic_pointer_cast<ClassDeclaration>(def)) {
        classDecl->decorators = decorators;
    }

    return def;
}

// ============ Helpers ============

std::shared_ptr<BlockStatement> Parser::parseBlockStatement() {
    auto block = std::make_shared<BlockStatement>();
    block->tag = NodeType::BLOCK_STATEMENT;
    block->token = curToken_;
    nextToken();

    while (!curTokenIs(TokenType::RBRACE) && !curTokenIs(TokenType::EOF_TOKEN)) {
        if (auto stmt = parseStatement()) {
            block->statements.push_back(stmt);
        }
        nextToken();
    }

    return block;
}

std::vector<IdentifierPtr> Parser::parseFunctionParameters() {
    return parseIdentifierList(TokenType::RPAREN);
}

std::vector<IdentifierPtr> Parser::parseIdentifierList(TokenType end) {
    std::vector<IdentifierPtr> identifiers;

    if (peekTokenIs(end)) {
        nextToken();
        return identifiers;
    }

    if (!expectPeek(TokenType::IDENT)) return {};
    auto ident = std::make_shared<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.literal;
    identifiers.push_back(ident);

    while (peekTokenIs(TokenType::COMMA)) {
        nextToken(); // comma
        if (peekTokenIs(end)) {
            nextToken(); // closing
            return identifiers;
        }
        nextToken();
        if (curToken_.type != TokenType::IDENT) {
            addError("expected identifier, got " + std::string(TokenTypeToString(curToken_.type)));
            return {};
        }
        auto id = std::make_shared<Identifier>();
        id->token = curToken_;
        id->value = curToken_.literal;
        identifiers.push_back(id);
    }

    if (!expectPeek(end)) return {};
    return identifiers;
}

std::vector<ExpressionPtr> Parser::parseExpressionList(TokenType end) {
    std::vector<ExpressionPtr> list;

    if (curTokenIs(end)) return list;

    if (auto expr = parseExpression(LOWEST)) {
        list.push_back(expr);
    }

    while (peekTokenIs(TokenType::COMMA)) {
        nextToken(); // comma
        if (peekTokenIs(end)) {
            nextToken(); // closing
            return list;
        }
        nextToken(); // next expr
        if (auto expr = parseExpression(LOWEST)) {
            list.push_back(expr);
        }
    }

    if (!expectPeek(end)) {
        if (isReplMode_ && peekTokenIs(TokenType::SEMICOLON)) {
            nextToken();
        }
    }

    return list;
}

// ============ Utility ============

bool Parser::curTokenIs(TokenType t) const { return curToken_.type == t; }
bool Parser::peekTokenIs(TokenType t) const { return peekToken_.type == t; }

bool Parser::expectPeek(TokenType t) {
    if (peekToken_.type == t) {
        nextToken();
        return true;
    }
    if (isReplMode_ && (peekToken_.type == TokenType::EOF_TOKEN || peekToken_.type == TokenType::SEMICOLON)) {
        addError("warning: expected " + std::string(TokenTypeToString(t)) + ", assuming complete expression");
        return true;
    }
    addError("expected next token to be " + std::string(TokenTypeToString(t)) + ", got " + std::string(TokenTypeToString(peekToken_.type)));
    return false;
}

bool Parser::expectCurrent(TokenType t) {
    if (curToken_.type == t) return true;
    addError("expected current token to be " + std::string(TokenTypeToString(t)) + ", got " + std::string(TokenTypeToString(curToken_.type)));
    return false;
}

void Parser::consumeOptionalSemicolon() {
    if (peekTokenIs(TokenType::SEMICOLON)) nextToken();
}

int Parser::curPrecedence() const {
    auto it = precedences.find(curToken_.type);
    return (it != precedences.end()) ? it->second : LOWEST;
}

int Parser::peekPrecedence() const {
    auto it = precedences.find(peekToken_.type);
    return (it != precedences.end()) ? it->second : LOWEST;
}

bool Parser::isValidAssignmentTarget(const ExpressionPtr& expr) const {
    return std::dynamic_pointer_cast<Identifier>(expr) ||
           std::dynamic_pointer_cast<IndexExpression>(expr) ||
           std::dynamic_pointer_cast<MemberExpression>(expr);
}

void Parser::addError(const std::string& msg) {
    std::string formatted;
    auto file = curToken_.file;
    int line = curToken_.line;
    int col = curToken_.column;
    if (line == 0 && peekToken_.line != 0) {
        file = peekToken_.file;
        line = peekToken_.line;
        col = peekToken_.column;
    }
    if (!file.empty() && line > 0 && col > 0) {
        formatted = file + ":" + std::to_string(line) + ":" + std::to_string(col) + ": " + msg;
    } else if (line > 0 && col > 0) {
        formatted = std::to_string(line) + ":" + std::to_string(col) + ": " + msg;
    } else {
        formatted = msg;
    }
    errors_.push_back(formatted);
}

} // namespace darix

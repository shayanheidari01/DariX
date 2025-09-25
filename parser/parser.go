// darix/parser.go

package parser

import (
	"darix/ast"
	"darix/lexer"
	"darix/token"
	"fmt"
	"strconv"
)

const (
	_ int = iota
	LOWEST
	ASSIGN      // =
	EQUALS      // ==
	LESSGREATER // > or <
	SUM         // +
	OR
	AND
	PRODUCT // * / %
	PREFIX  // -X or !X
	CALL    // myFunction(X)
	INDEX   // array[index] or map[key]
	MEMBER  // obj.prop
)

var precedences = map[token.TokenType]int{
	token.ASSIGN:   ASSIGN,
	token.OR:       OR,
	token.AND:      AND,
	token.OR_KW:    OR,      // 'or' keyword
	token.AND_KW:   AND,     // 'and' keyword
	token.IN:       EQUALS,  // 'in' operator
	token.IS:       EQUALS,  // 'is' operator
	token.EQ:       EQUALS,
	token.NOT_EQ:   EQUALS,
	token.LT:       LESSGREATER,
	token.GT:       LESSGREATER,
	token.LE:       LESSGREATER,
	token.GE:       LESSGREATER,
	token.PLUS:     SUM,
	token.MINUS:    SUM,
	token.SLASH:    PRODUCT,
	token.MODULO:   PRODUCT,
	token.ASTERISK: PRODUCT,
	token.LPAREN:   CALL,
	token.DOT:      MEMBER,
	token.LBRACKET: INDEX,
}

type Parser struct {
	l *lexer.Lexer

	curToken  token.Token
	peekToken token.Token

	errors []string

	prefixParseFns map[token.TokenType]prefixParseFn
	infixParseFns  map[token.TokenType]infixParseFn
	isReplMode     bool
}

type (
	prefixParseFn func() ast.Expression
	infixParseFn  func(ast.Expression) ast.Expression
)

func New(l *lexer.Lexer) *Parser {
	p := &Parser{
		l:              l,
		errors:         make([]string, 0, 8), // Pre-allocate capacity
		prefixParseFns: make(map[token.TokenType]prefixParseFn, 16),
		infixParseFns:  make(map[token.TokenType]infixParseFn, 16),
		isReplMode:     false,
	}

	p.registerParseFns()
	p.nextToken()
	p.nextToken()

	return p
}

func (p *Parser) SetReplMode(mode bool) {
	p.isReplMode = mode
}

// Combined registration for better performance
func (p *Parser) registerParseFns() {
	// Prefix functions
	p.prefixParseFns[token.IDENT] = p.parseIdentifier
	p.prefixParseFns[token.INT] = p.parseIntegerLiteral
	p.prefixParseFns[token.FLOAT] = p.parseFloatLiteral
	p.prefixParseFns[token.STRING] = p.parseStringLiteral
	p.prefixParseFns[token.BANG] = p.parsePrefixExpression
	p.prefixParseFns[token.MINUS] = p.parsePrefixExpression
	p.prefixParseFns[token.NOT_KW] = p.parsePrefixExpression  // 'not' keyword
	p.prefixParseFns[token.TRUE] = p.parseBoolean
	p.prefixParseFns[token.FALSE] = p.parseBoolean
	p.prefixParseFns[token.NULL] = p.parseNull
	p.prefixParseFns[token.LPAREN] = p.parseGroupedExpression
	p.prefixParseFns[token.IF] = p.parseIfExpression
	p.prefixParseFns[token.FUNCTION] = p.parseFunctionLiteral
	p.prefixParseFns[token.LAMBDA] = p.parseLambdaExpression  // lambda expressions
	p.prefixParseFns[token.WHILE] = p.parseWhileExpression
	p.prefixParseFns[token.FOR] = p.parseForExpression
	p.prefixParseFns[token.LBRACKET] = p.parseArrayLiteral
	p.prefixParseFns[token.LBRACE] = p.parseMapLiteral
	p.prefixParseFns[token.YIELD] = p.parseYieldExpression  // yield expressions

	// Infix functions
	p.infixParseFns[token.ASSIGN] = p.parseAssignmentExpression
	p.infixParseFns[token.PLUS] = p.parseInfixExpression
	p.infixParseFns[token.MINUS] = p.parseInfixExpression
	p.infixParseFns[token.SLASH] = p.parseInfixExpression
	p.infixParseFns[token.MODULO] = p.parseInfixExpression
	p.infixParseFns[token.ASTERISK] = p.parseInfixExpression
	p.infixParseFns[token.EQ] = p.parseInfixExpression
	p.infixParseFns[token.NOT_EQ] = p.parseInfixExpression
	p.infixParseFns[token.LT] = p.parseInfixExpression
	p.infixParseFns[token.GT] = p.parseInfixExpression
	p.infixParseFns[token.LE] = p.parseInfixExpression
	p.infixParseFns[token.GE] = p.parseInfixExpression
	p.infixParseFns[token.OR] = p.parseInfixExpression
	p.infixParseFns[token.AND] = p.parseInfixExpression
	// New keyword operators
	p.infixParseFns[token.OR_KW] = p.parseInfixExpression   // 'or' keyword
	p.infixParseFns[token.AND_KW] = p.parseInfixExpression  // 'and' keyword
	p.infixParseFns[token.IN] = p.parseInExpression         // 'in' operator
	p.infixParseFns[token.IS] = p.parseIsExpression         // 'is' operator
	p.infixParseFns[token.LPAREN] = p.parseCallExpression
	p.infixParseFns[token.LBRACKET] = p.parseIndexExpression
	p.infixParseFns[token.DOT] = p.parseMemberExpression
}

func (p *Parser) nextToken() {
	p.curToken = p.peekToken
	p.peekToken = p.l.NextToken()
}

func (p *Parser) ParseProgram() *ast.Program {
	program := &ast.Program{
		Statements: make([]ast.Statement, 0, 16), // Pre-allocate capacity
	}

	for p.curToken.Type != token.EOF {
		if stmt := p.parseStatement(); stmt != nil {
			program.Statements = append(program.Statements, stmt)
		}
		p.nextToken()
	}

	return program
}

func (p *Parser) parseStatement() ast.Statement {
	if p.curToken.Type == token.ILLEGAL {
		p.addError("illegal token: %s", p.curToken.Literal)
		p.nextToken()
		return nil
	}

	switch p.curToken.Type {
	case token.IMPORT:
		return p.parseImportStatement()
	case token.CLASS:
		return p.parseClassDeclaration()
	case token.FUNCTION:
		return p.parseFunctionDeclaration()
	case token.VAR:
		return p.parseLetStatement()
	case token.RETURN:
		return p.parseReturnStatement()
	case token.WHILE:
		return p.parseWhileStatement()
	case token.FOR:
		return p.parseForStatement()
	case token.BREAK:
		return p.parseBreakStatement()
	case token.CONTINUE:
		return p.parseContinueStatement()
	case token.TRY:
		return p.parseTryStatement()
	case token.THROW, token.RAISE:
		return p.parseThrowStatement()
	// New keywords
	case token.DEL:
		return p.parseDelStatement()
	case token.ASSERT:
		return p.parseAssertStatement()
	case token.PASS:
		return p.parsePassStatement()
	case token.GLOBAL:
		return p.parseGlobalStatement()
	case token.NONLOCAL:
		return p.parseNonlocalStatement()
	case token.AT:
		return p.parseDecoratedDefinition()
	case token.WITH:
		return p.parseWithStatement()
	case token.LBRACE:
		// Parse standalone block statements
		return p.parseBlockStatementAsStatement()
	case token.IDENT:
		if p.isAssignment() {
			return p.parseAssignStatement()
		}
		return p.parseExpressionStatement()
	case token.RBRACE, token.SEMICOLON:
		return nil
	default:
		return p.parseExpressionStatement()
	}
}

// Assignment detection with proper index assignment support
func (p *Parser) isAssignment() bool {
	if p.peekToken.Type == token.ASSIGN {
		return true
	}
	// For index assignment: arr[i] = value
	// We'll parse this as an expression and convert if it's an assignment
	return false
}

type parserState struct {
	curToken  token.Token
	peekToken token.Token
}

func (p *Parser) saveState() parserState {
	return parserState{p.curToken, p.peekToken}
}

func (p *Parser) restoreState(state parserState) {
	p.curToken = state.curToken
	p.peekToken = state.peekToken
}

func (p *Parser) parseExpression(precedence int) ast.Expression {
	prefix := p.prefixParseFns[p.curToken.Type]
	if prefix == nil {
		p.addError("no prefix parse function for %s found", p.curToken.Type)
		return nil
	}

	leftExp := prefix()
	if leftExp == nil {
		return nil
	}

	for p.peekPrecedence() > precedence {
		infix := p.infixParseFns[p.peekToken.Type]
		if infix == nil {
			break
		}
		p.nextToken()
		leftExp = infix(leftExp)
		if leftExp == nil {
			break
		}
	}

	return leftExp
}

// --- Parse Functions ---

func (p *Parser) parseIdentifier() ast.Expression {
	return &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}
}

func (p *Parser) parseIntegerLiteral() ast.Expression {
	value, err := strconv.ParseInt(p.curToken.Literal, 0, 64)
	if err != nil {
		p.addError("could not parse %q as integer", p.curToken.Literal)
		return nil
	}
	return &ast.IntegerLiteral{Token: p.curToken, Value: value}
}

func (p *Parser) parseFloatLiteral() ast.Expression {
	value, err := strconv.ParseFloat(p.curToken.Literal, 64)
	if err != nil {
		p.addError("could not parse %q as float", p.curToken.Literal)
		return nil
	}
	return &ast.FloatLiteral{Token: p.curToken, Value: value}
}

func (p *Parser) parseStringLiteral() ast.Expression {
	return &ast.StringLiteral{Token: p.curToken, Value: p.curToken.Literal}
}

func (p *Parser) parseBoolean() ast.Expression {
	return &ast.Boolean{Token: p.curToken, Value: p.curToken.Type == token.TRUE}
}

func (p *Parser) parseNull() ast.Expression {
	return &ast.Null{Token: p.curToken}
}

func (p *Parser) parsePrefixExpression() ast.Expression {
	expr := &ast.PrefixExpression{Token: p.curToken, Operator: p.curToken.Literal}
	// Normalize 'not' keyword to '!' for interpreter compatibility
	if p.curToken.Type == token.NOT_KW {
		expr.Operator = "!"
	}
	p.nextToken()
	expr.Right = p.parseExpression(PREFIX)
	return expr
}

func (p *Parser) parseGroupedExpression() ast.Expression {
	p.nextToken()
	exp := p.parseExpression(LOWEST)
	if !p.expectPeek(token.RPAREN) {
		return nil
	}
	return exp
}

func (p *Parser) parseIfExpression() ast.Expression {
	expr := &ast.IfExpression{Token: p.curToken}

	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	p.nextToken()
	expr.Condition = p.parseExpression(LOWEST)
	if !p.expectPeek(token.RPAREN) || !p.expectPeek(token.LBRACE) {
		return nil
	}
	expr.Consequence = p.parseBlockStatement()

	if p.peekToken.Type == token.ELSE {
		p.nextToken()
		if p.peekToken.Type == token.IF {
			p.nextToken()
			expr.Alternative = p.parseIfExpression()
		} else {
			if !p.expectPeek(token.LBRACE) {
				return nil
			}
			expr.Alternative = p.parseBlockStatement()
		}
	}

	return expr
}

func (p *Parser) parseFunctionLiteral() ast.Expression {
	lit := &ast.FunctionLiteral{Token: p.curToken}

	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	params := p.parseFunctionParameters()
	if params == nil {
		return nil
	}
	lit.Parameters = params
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	lit.Body = p.parseBlockStatement()

	return lit
}

func (p *Parser) parseArrayLiteral() ast.Expression {
	array := &ast.ArrayLiteral{Token: p.curToken}
	p.nextToken()

	if p.curTokenIs(token.RBRACKET) {
		array.Elements = []ast.Expression{}
		return array
	}

	array.Elements = p.parseExpressionList(token.RBRACKET)
	return array
}

func (p *Parser) parseMapLiteral() ast.Expression {
	lit := &ast.MapLiteral{Token: p.curToken, Pairs: make(map[ast.Expression]ast.Expression)}

	if p.peekToken.Type == token.RBRACE {
		p.nextToken()
		return lit
	}

	p.nextToken()
	for {
		key := p.parseExpression(LOWEST)
		if key == nil || !p.expectPeek(token.COLON) {
			return nil
		}
		p.nextToken()
		value := p.parseExpression(LOWEST)
		if value == nil {
			return nil
		}
		lit.Pairs[key] = value

		if p.peekToken.Type != token.COMMA {
			break
		}
		p.nextToken() // consume comma
		p.nextToken() // move to next key
	}

	if !p.expectPeek(token.RBRACE) {
		return nil
	}
	return lit
}

func (p *Parser) parseInfixExpression(left ast.Expression) ast.Expression {
	expr := &ast.InfixExpression{
		Token:    p.curToken,
		Operator: p.curToken.Literal,
		Left:     left,
	}

	precedence := p.curPrecedence()
	p.nextToken()
	expr.Right = p.parseExpression(precedence)

	return expr
}

func (p *Parser) parseCallExpression(fn ast.Expression) ast.Expression {
	exp := &ast.CallExpression{Token: p.curToken, Function: fn}
	p.nextToken()
	exp.Arguments = p.parseExpressionList(token.RPAREN)
	return exp
}

func (p *Parser) parseIndexExpression(left ast.Expression) ast.Expression {
	exp := &ast.IndexExpression{Token: p.curToken, Left: left}
	p.nextToken()
	exp.Index = p.parseExpression(LOWEST)
	if !p.expectPeek(token.RBRACKET) {
		return nil
	}
	return exp
}

func (p *Parser) parseMemberExpression(left ast.Expression) ast.Expression {
	exp := &ast.MemberExpression{Token: p.curToken, Left: left}
	if !p.expectPeek(token.IDENT) {
		return nil
	}
	exp.Property = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}
	return exp
}

func (p *Parser) parseAssignmentExpression(left ast.Expression) ast.Expression {
	if !p.isValidAssignmentTarget(left) {
		p.addError("invalid assignment target: expected identifier, index, or member expression, got %T", left)
		return nil
	}

	expr := &ast.AssignExpression{Token: p.curToken, Name: left}
	p.nextToken()
	expr.Value = p.parseExpression(LOWEST)
	if expr.Value == nil {
		p.addError("invalid expression in assignment")
		return nil
	}

	return expr
}

func (p *Parser) isValidAssignmentTarget(expr ast.Expression) bool {
	switch expr.(type) {
	case *ast.Identifier, *ast.IndexExpression, *ast.MemberExpression:
		return true
	default:
		return false
	}
}

// Statement parsers
func (p *Parser) parseLetStatement() ast.Statement {
	stmt := &ast.LetStatement{Token: p.curToken}

	if !p.expectPeek(token.IDENT) {
		return nil
	}
	stmt.Name = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}

	// Check if there's an assignment
	if p.peekToken.Type == token.ASSIGN {
		p.nextToken() // consume ASSIGN
		p.nextToken() // move to value
		stmt.Value = p.parseExpression(LOWEST)
	} else {
		// No assignment, initialize to null
		stmt.Value = &ast.Null{Token: token.Token{Type: token.NULL, Literal: "null"}}
	}

	p.consumeOptionalSemicolon()
	return stmt
}

func (p *Parser) parseClassDeclaration() ast.Statement {
	stmt := &ast.ClassDeclaration{Token: p.curToken}

	if !p.expectPeek(token.IDENT) {
		return nil
	}
	stmt.Name = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}

	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.Body = p.parseBlockStatement()
	return stmt
}

func (p *Parser) parseReturnStatement() ast.Statement {
	stmt := &ast.ReturnStatement{Token: p.curToken}

	// Check if there's a value to return
	if p.peekToken.Type == token.SEMICOLON || p.peekToken.Type == token.RBRACE || p.peekToken.Type == token.EOF {
		// No return value
		stmt.ReturnValue = &ast.Null{Token: token.Token{Type: token.NULL, Literal: "null"}}
	} else {
		p.nextToken()
		stmt.ReturnValue = p.parseExpression(LOWEST)
	}

	p.consumeOptionalSemicolon()
	return stmt
}

func (p *Parser) parseExpressionStatement() ast.Statement {
	stmt := &ast.ExpressionStatement{Token: p.curToken}
	stmt.Expression = p.parseExpression(LOWEST)

	// Handle assignments that weren't caught as statements
	if assignExpr, ok := stmt.Expression.(*ast.AssignExpression); ok {
		// Convert assignment expression to assignment statement
		assignStmt := &ast.AssignStatement{
			Token:  assignExpr.Token,
			Target: assignExpr.Name,
			Value:  assignExpr.Value,
		}
		p.consumeOptionalSemicolon()
		return assignStmt
	}

	p.consumeOptionalSemicolon()
	return stmt
}

func (p *Parser) parseBlockStatementAsStatement() ast.Statement {
	// This handles standalone block statements like { var x = 1; }
	// These are different from function/if blocks - they don't create new scopes
	block := p.parseBlockStatement()
	return &ast.StandaloneBlockStatement{
		Token: block.Token,
		Block: block,
	}
}

func (p *Parser) parseAssignStatement() ast.Statement {
	stmt := &ast.AssignStatement{Token: p.curToken}

	// Parse the target as a simple expression without allowing infix operations
	var target ast.Expression
	switch p.curToken.Type {
	case token.IDENT:
		target = p.parseIdentifier()
	default:
		// For more complex targets like array[index], parse as expression
		target = p.parseExpression(INDEX) // Use INDEX precedence to stop before ASSIGN
	}

	if target == nil || !p.isValidAssignmentTarget(target) {
		p.addError("invalid assignment target")
		return nil
	}
	stmt.Target = target

	if !p.expectPeek(token.ASSIGN) {
		return nil
	}
	p.nextToken()
	stmt.Value = p.parseExpression(LOWEST)
	if stmt.Value == nil {
		p.addError("invalid expression in assignment")
		return nil
	}

	p.consumeOptionalSemicolon()
	return stmt
}

func (p *Parser) parseWhileStatement() ast.Statement {
	stmt := &ast.WhileStatement{Token: p.curToken}

	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	p.nextToken()
	stmt.Condition = p.parseExpression(LOWEST)
	if !p.expectPeek(token.RPAREN) || !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.Body = p.parseBlockStatement()
	return stmt
}

func (p *Parser) parseForStatement() ast.Statement {
	// Desugar for(init; condition; post) { body } into:
	// { init; while (condition or true) { body; post; } }
	tok := p.curToken
	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	p.nextToken()

	// init
	var initStmt ast.Statement
	if p.curToken.Type != token.SEMICOLON {
		if p.curToken.Type == token.VAR {
			initStmt = p.parseLetStatement()
		} else {
			initStmt = p.parseExpressionStatement()
		}
	}
	if !p.expectCurrent(token.SEMICOLON) {
		return nil
	}
	p.nextToken()

	// condition
	var condExp ast.Expression
	if p.curToken.Type != token.SEMICOLON {
		condExp = p.parseExpression(LOWEST)
	}
	if !p.expectPeek(token.SEMICOLON) {
		return nil
	}
	p.nextToken()

	// post
	var postStmt ast.Statement
	if p.curToken.Type != token.RPAREN {
		if p.curToken.Type == token.IDENT && p.peekToken.Type == token.ASSIGN {
			postStmt = p.parseAssignStatement()
		} else {
			postStmt = p.parseExpressionStatement()
		}
	}

	if !p.expectPeek(token.RPAREN) || !p.expectPeek(token.LBRACE) {
		return nil
	}
	body := p.parseBlockStatement()

	// while body: original body + post (if any)
	whileBody := &ast.BlockStatement{Token: body.Token, Statements: make([]ast.Statement, 0, len(body.Statements)+1)}
	whileBody.Statements = append(whileBody.Statements, body.Statements...)
	if postStmt != nil {
		whileBody.Statements = append(whileBody.Statements, postStmt)
	}

	// default condition to true if omitted
	if condExp == nil {
		condExp = &ast.Boolean{Token: token.Token{Type: token.TRUE, Literal: "true"}, Value: true}
	}
	// while statement
	whileStmt := &ast.WhileStatement{Token: tok, Condition: condExp, Body: whileBody}
	// wrap into a block with init first
	desugared := &ast.BlockStatement{Token: tok, Statements: make([]ast.Statement, 0, 2)}
	if initStmt != nil {
		desugared.Statements = append(desugared.Statements, initStmt)
	}
	desugared.Statements = append(desugared.Statements, whileStmt)
	return desugared
}

func (p *Parser) parseBreakStatement() ast.Statement {
	stmt := &ast.BreakStatement{Token: p.curToken}
	p.consumeOptionalSemicolon()
	return stmt
}

func (p *Parser) parseContinueStatement() ast.Statement {
	stmt := &ast.ContinueStatement{Token: p.curToken}
	p.consumeOptionalSemicolon()
	return stmt
}

func (p *Parser) parseForExpression() ast.Expression {
	// Reuse the statement parser and return the desugared block as an expression
	stmt := p.parseForStatement()
	if stmt == nil {
		return nil
	}
	if block, ok := stmt.(*ast.BlockStatement); ok {
		return block
	}
	return &ast.BlockStatement{Token: p.curToken, Statements: []ast.Statement{stmt}}
}

// Helper functions
func (p *Parser) parseFunctionParameters() []*ast.Identifier {
	return p.parseIdentifierList(token.RPAREN)
}

func (p *Parser) parseExpressionList(end token.TokenType) []ast.Expression {
	list := make([]ast.Expression, 0, 4)

	if p.curTokenIs(end) {
		// Don't advance - leave the end token for the caller to consume
		return list
	}

	if expr := p.parseExpression(LOWEST); expr != nil {
		list = append(list, expr)
	}

	for p.peekTokenIs(token.COMMA) {
		p.nextToken() // consume comma
		if p.peekTokenIs(end) {
			p.nextToken() // move to closing token
			return list
		}
		p.nextToken() // move to next expr
		if expr := p.parseExpression(LOWEST); expr != nil {
			list = append(list, expr)
		}
	}

	if !p.expectPeek(end) {
		if p.isReplMode && p.peekTokenIs(token.SEMICOLON) {
			p.nextToken()
		}
	}

	return list
}

func (p *Parser) parseBlockStatement() *ast.BlockStatement {
	block := &ast.BlockStatement{
		Token:      p.curToken,
		Statements: make([]ast.Statement, 0, 8),
	}
	p.nextToken()

	for !p.curTokenIs(token.RBRACE) && !p.curTokenIs(token.EOF) {
		if stmt := p.parseStatement(); stmt != nil {
			block.Statements = append(block.Statements, stmt)
		}
		p.nextToken()
	}

	return block
}

// Utility functions
func (p *Parser) curTokenIs(t token.TokenType) bool {
	return p.curToken.Type == t
}

func (p *Parser) peekTokenIs(t token.TokenType) bool {
	return p.peekToken.Type == t
}

func (p *Parser) expectCurrent(t token.TokenType) bool {
	if p.curToken.Type == t {
		return true
	}
	p.addError("expected current token to be %s, got %s", t, p.curToken.Type)
	return false
}

func (p *Parser) expectPeek(t token.TokenType) bool {
	if p.peekToken.Type == t {
		p.nextToken()
		return true
	}

	if p.isReplMode && (p.peekToken.Type == token.EOF || p.peekToken.Type == token.SEMICOLON) {
		p.addError("warning: expected %s, got %s, assuming complete expression", t, p.peekToken.Type)
		return true
	}

	p.addError("expected next token to be %s, got %s", t, p.peekToken.Type)
	return false
}

func (p *Parser) consumeOptionalSemicolon() {
	if p.peekTokenIs(token.SEMICOLON) {
		p.nextToken()
	}
}

func (p *Parser) curPrecedence() int {
	if precedence, ok := precedences[p.curToken.Type]; ok {
		return precedence
	}
	return LOWEST
}

func (p *Parser) peekPrecedence() int {
	if precedence, ok := precedences[p.peekToken.Type]; ok {
		return precedence
	}
	return LOWEST
}

// Error handling
func (p *Parser) Errors() []string {
	return p.errors
}

func (p *Parser) addError(format string, args ...interface{}) {
	// Prefer current token position; fallback to peek token.
	file := p.curToken.File
	line, col := p.curToken.Line, p.curToken.Column
	if line == 0 && p.peekToken.Line != 0 {
		file, line, col = p.peekToken.File, p.peekToken.Line, p.peekToken.Column
	}
	base := fmt.Sprintf(format, args...)
	switch {
	case file != "" && line > 0 && col > 0:
		p.errors = append(p.errors, fmt.Sprintf("%s:%d:%d: %s", file, line, col, base))
	case line > 0 && col > 0:
		p.errors = append(p.errors, fmt.Sprintf("%d:%d: %s", line, col, base))
	default:
		p.errors = append(p.errors, base)
	}
}

// ===== EXCEPTION HANDLING PARSERS =====

// parseTryStatement parses try-catch-finally statements
func (p *Parser) parseTryStatement() ast.Statement {
	stmt := &ast.TryStatement{Token: p.curToken}

	// Parse try block
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.TryBlock = p.parseBlockStatement()

	// Parse catch clauses
	for p.peekToken.Type == token.CATCH {
		p.nextToken() // consume CATCH
		catchClause := p.parseCatchClause()
		if catchClause == nil {
			return nil
		}
		stmt.CatchClauses = append(stmt.CatchClauses, catchClause)
	}

	// Parse optional finally block
	if p.peekToken.Type == token.FINALLY {
		p.nextToken() // consume FINALLY
		if !p.expectPeek(token.LBRACE) {
			return nil
		}
		stmt.FinallyBlock = p.parseBlockStatement()
	}

	// Must have at least one catch clause or finally block
	if len(stmt.CatchClauses) == 0 && stmt.FinallyBlock == nil {
		p.addError("try statement must have at least one catch clause or finally block")
		return nil
	}

	return stmt
}

// parseCatchClause parses individual catch clauses
func (p *Parser) parseCatchClause() *ast.CatchClause {
	catchClause := &ast.CatchClause{Token: p.curToken}

	// Parse optional exception type and variable binding
	if p.peekToken.Type == token.LPAREN {
		p.nextToken() // consume LPAREN

		// Check for exception type or variable
		if p.peekToken.Type == token.IDENT {
			p.nextToken() // move to identifier

			// This could be either an exception type or variable name
			firstIdent := &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}

			// Check if there's another identifier (type variable pattern)
			if p.peekToken.Type == token.IDENT {
				// First identifier is exception type
				catchClause.ExceptionType = firstIdent
				p.nextToken() // move to variable name
				catchClause.Variable = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}
			} else {
				// Single identifier is variable name (catches all exceptions)
				catchClause.Variable = firstIdent
			}
		}

		if !p.expectPeek(token.RPAREN) {
			return nil
		}
	}

	// Parse catch block
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	catchClause.CatchBlock = p.parseBlockStatement()

	return catchClause
}

// parseThrowStatement parses throw/raise statements
func (p *Parser) parseThrowStatement() ast.Statement {
	stmt := &ast.ThrowStatement{Token: p.curToken}

	// Parse the exception expression
	p.nextToken()
	stmt.Exception = p.parseExpression(LOWEST)
	if stmt.Exception == nil {
		p.addError("expected exception expression after %s", stmt.Token.Literal)
		return nil
	}

	p.consumeOptionalSemicolon()
	return stmt
}

// parseImportStatement parses import statements
func (p *Parser) parseImportStatement() ast.Statement {
	stmt := &ast.ImportStatement{Token: p.curToken}

	if !p.expectPeek(token.STRING) {
		return nil
	}
	stmt.Path = &ast.StringLiteral{Token: p.curToken, Value: p.curToken.Literal}
	p.consumeOptionalSemicolon()
	return stmt
}

// parseFunctionDeclaration parses function declarations
func (p *Parser) parseFunctionDeclaration() ast.Statement {
	stmt := &ast.FunctionDeclaration{Token: p.curToken}

	if !p.expectPeek(token.IDENT) {
		return nil
	}
	stmt.Name = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}

	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	params := p.parseFunctionParameters()
	if params == nil {
		return nil
	}
	stmt.Parameters = params
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.Body = p.parseBlockStatement()
	return stmt
}

func (p *Parser) parseIdentifierList(end token.TokenType) []*ast.Identifier {
	identifiers := make([]*ast.Identifier, 0, 4)

	if p.peekToken.Type == end {
		p.nextToken()
		return identifiers
	}

	if !p.expectPeek(token.IDENT) {
		return nil
	}
	identifiers = append(identifiers, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})

	for p.peekToken.Type == token.COMMA {
		p.nextToken() // consume comma
		if p.peekToken.Type == end {
			p.nextToken() // consume closing delimiter
			return identifiers
		}
		p.nextToken()
		if p.curToken.Type != token.IDENT {
			p.addError("expected identifier, got %s", p.curToken.Type)
			return nil
		}
		identifiers = append(identifiers, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
	}

	if !p.expectPeek(end) {
		return nil
	}

	return identifiers
}

// parseWhileExpression parses while loops as expressions
func (p *Parser) parseWhileExpression() ast.Expression {
	stmt := p.parseWhileStatement()
	if whileStmt, ok := stmt.(*ast.WhileStatement); ok && whileStmt != nil {
		return &ast.WhileExpression{
			Token:     whileStmt.Token,
			Condition: whileStmt.Condition,
			Body:      whileStmt.Body,
		}
	}
	p.addError("expected while statement")
	return nil
}

// ===== NEW KEYWORD PARSING FUNCTIONS =====

// parseDelStatement parses 'del' statements
func (p *Parser) parseDelStatement() ast.Statement {
	stmt := &ast.DelStatement{Token: p.curToken}
	
	p.nextToken()
	
	// Parse the target (identifier, index expression, or member expression)
	stmt.Target = p.parseExpression(LOWEST)
	if stmt.Target == nil {
		return nil
	}
	
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	
	return stmt
}

// parseAssertStatement parses 'assert' statements
func (p *Parser) parseAssertStatement() ast.Statement {
	stmt := &ast.AssertStatement{Token: p.curToken}
	
	p.nextToken()
	
	// Parse the condition
	stmt.Condition = p.parseExpression(LOWEST)
	if stmt.Condition == nil {
		return nil
	}
	
	// Check for optional message
	if p.peekToken.Type == token.COMMA {
		p.nextToken() // consume comma
		p.nextToken() // move to message
		stmt.Message = p.parseExpression(LOWEST)
	}
	
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	
	return stmt
}

// parsePassStatement parses 'pass' statements
func (p *Parser) parsePassStatement() ast.Statement {
	stmt := &ast.PassStatement{Token: p.curToken}
	
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	
	return stmt
}

// parseGlobalStatement parses 'global' statements
func (p *Parser) parseGlobalStatement() ast.Statement {
	stmt := &ast.GlobalStatement{Token: p.curToken}
	
	if !p.expectPeek(token.IDENT) {
		return nil
	}
	
	stmt.Names = append(stmt.Names, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
	
	// Parse additional identifiers separated by commas
	for p.peekToken.Type == token.COMMA {
		p.nextToken() // consume comma
		if !p.expectPeek(token.IDENT) {
			return nil
		}
		stmt.Names = append(stmt.Names, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
	}
	
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	
	return stmt
}

// parseNonlocalStatement parses 'nonlocal' statements
func (p *Parser) parseNonlocalStatement() ast.Statement {
	stmt := &ast.NonlocalStatement{Token: p.curToken}
	
	if !p.expectPeek(token.IDENT) {
		return nil
	}
	
	stmt.Names = append(stmt.Names, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
	
	// Parse additional identifiers separated by commas
	for p.peekToken.Type == token.COMMA {
		p.nextToken() // consume comma
		if !p.expectPeek(token.IDENT) {
			return nil
		}
		stmt.Names = append(stmt.Names, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
	}
	
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	
	return stmt
}

// parseWithStatement parses 'with' statements
func (p *Parser) parseWithStatement() ast.Statement {
	stmt := &ast.WithStatement{Token: p.curToken}
	
	p.nextToken()
	
	// Parse context expression
	stmt.Context = p.parseExpression(LOWEST)
	if stmt.Context == nil {
		return nil
	}
	
	// Check for optional 'as' variable
	if p.peekToken.Type == token.AS {
		p.nextToken() // consume 'as'
		if !p.expectPeek(token.IDENT) {
			return nil
		}
		stmt.Variable = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}
	}
	
	// Parse body block
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	
	stmt.Body = p.parseBlockStatement()
	return stmt
}

// parseLambdaExpression parses lambda expressions
func (p *Parser) parseLambdaExpression() ast.Expression {
	expr := &ast.LambdaExpression{Token: p.curToken}
	
	// Parse parameters (optional)
	if p.peekToken.Type == token.IDENT {
		p.nextToken()
		expr.Parameters = append(expr.Parameters, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
		
		// Parse additional parameters separated by commas
		for p.peekToken.Type == token.COMMA {
			p.nextToken() // consume comma
			if !p.expectPeek(token.IDENT) {
				return nil
			}
			expr.Parameters = append(expr.Parameters, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
		}
	}
	
	// Expect colon
	if !p.expectPeek(token.COLON) {
		return nil
	}
	
	// Parse body expression
	p.nextToken()
	expr.Body = p.parseExpression(LOWEST)
	if expr.Body == nil {
		return nil
	}
	
	return expr
}

// parseYieldExpression parses yield expressions
func (p *Parser) parseYieldExpression() ast.Expression {
	expr := &ast.YieldExpression{Token: p.curToken}
	
	// Check if there's a value to yield
	if p.peekToken.Type != token.SEMICOLON && p.peekToken.Type != token.RBRACE && p.peekToken.Type != token.EOF {
		p.nextToken()
		expr.Value = p.parseExpression(LOWEST)
	}
	
	return expr
}

// parseInExpression parses 'in' expressions
func (p *Parser) parseInExpression(left ast.Expression) ast.Expression {
	expr := &ast.InExpression{
		Token: p.curToken,
		Left:  left,
	}
	
	precedence := p.curPrecedence()
	p.nextToken()
	expr.Right = p.parseExpression(precedence)
	
	return expr
}

// parseIsExpression parses 'is' expressions
func (p *Parser) parseIsExpression(left ast.Expression) ast.Expression {
	expr := &ast.IsExpression{
		Token: p.curToken,
		Left:  left,
	}
	
	precedence := p.curPrecedence()
	p.nextToken()
	expr.Right = p.parseExpression(precedence)
	
	return expr
}

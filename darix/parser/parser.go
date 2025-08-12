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
	EQUALS      // ==
	LESSGREATER // > or <
	SUM         // +
	OR
	AND
	PRODUCT // * / %
	PREFIX  // -X or !X
	CALL    // myFunction(X)
	INDEX   // array[index] or map[key]
)

var precedences = map[token.TokenType]int{
	token.ASSIGN:   LOWEST,
	token.OR:       OR,
	token.AND:      AND,
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
	token.LBRACKET: INDEX,
}

type Parser struct {
	l *lexer.Lexer

	curToken  token.Token
	peekToken token.Token

	errors []string

	prefixParseFns map[token.TokenType]prefixParseFn
	infixParseFns  map[token.TokenType]infixParseFn

	// افزودن فلگ برای تشخیص حالت REPL
	isReplMode bool
}

type (
	prefixParseFn func() ast.Expression
	infixParseFn  func(ast.Expression) ast.Expression
)

func New(l *lexer.Lexer) *Parser {
	p := &Parser{
		l:          l,
		errors:     []string{},
		isReplMode: false, // مقداردهی اولیه به false
	}

	// ثبت توابع پارس
	p.registerPrefixParseFns()
	p.registerInfixParseFns()

	// خواندن دو توکن اول
	p.nextToken()
	p.nextToken()

	return p
}

// تابع برای فعال کردن حالت REPL
func (p *Parser) SetReplMode(mode bool) {
	p.isReplMode = mode
}

func (p *Parser) registerPrefixParseFns() {
	p.prefixParseFns = map[token.TokenType]prefixParseFn{
		token.IDENT:    p.parseIdentifier,
		token.INT:      p.parseIntegerLiteral,
		token.FLOAT:    p.parseFloatLiteral,
		token.STRING:   p.parseStringLiteral,
		token.BANG:     p.parsePrefixExpression,
		token.MINUS:    p.parsePrefixExpression,
		token.TRUE:     p.parseBoolean,
		token.FALSE:    p.parseBoolean,
		token.NULL:     p.parseNull,
		token.LPAREN:   p.parseGroupedExpression,
		token.IF:       p.parseIfExpression,
		token.FUNCTION: p.parseFunctionLiteral,
		token.WHILE:    p.parseWhileExpression, // تغییر این خط
		token.FOR:      p.parseForExpression,   // تغییر این خط
		token.LBRACKET: p.parseArrayLiteral,
		token.LBRACE:   p.parseMapLiteral,
	}
}

func (p *Parser) parseNull() ast.Expression {
	return &ast.Null{Token: p.curToken}
}

func (p *Parser) registerInfixParseFns() {
	p.infixParseFns = map[token.TokenType]infixParseFn{
		token.ASSIGN:   p.parseAssignmentExpression,
		token.PLUS:     p.parseInfixExpression,
		token.MINUS:    p.parseInfixExpression,
		token.SLASH:    p.parseInfixExpression,
		token.MODULO:   p.parseInfixExpression,
		token.ASTERISK: p.parseInfixExpression,
		token.EQ:       p.parseInfixExpression,
		token.NOT_EQ:   p.parseInfixExpression,
		token.LT:       p.parseInfixExpression,
		token.GT:       p.parseInfixExpression,
		token.LE:       p.parseInfixExpression,
		token.GE:       p.parseInfixExpression,
		token.LPAREN:   p.parseCallExpression,
		token.LBRACKET: p.parseIndexExpression,
	}
}

func (p *Parser) nextToken() {
	p.curToken = p.peekToken
	p.peekToken = p.l.NextToken()
}

func (p *Parser) ParseProgram() *ast.Program {
	program := &ast.Program{}
	program.Statements = []ast.Statement{}

	for p.curToken.Type != token.EOF {
		stmt := p.parseStatement()
		if stmt != nil {
			program.Statements = append(program.Statements, stmt)
		}
		p.nextToken()
	}

	return program
}

// func (p *Parser) parseStatement() ast.Statement {
// 	switch p.curToken.Type {
// 	case token.VAR:
// 		return p.parseLetStatement()
// 	case token.RETURN:
// 		return p.parseReturnStatement()
// 	case token.IDENT:
// 		if p.peekToken.Type == token.ASSIGN {
// 			return p.parseAssignStatement()
// 		}
// 		return p.parseExpressionStatement()
// 	case token.BREAK:
// 		return p.parseBreakStatement()
// 	case token.CONTINUE:
// 		return p.parseContinueStatement()
// 	case token.WHILE:
// 		return p.parseWhileStatement()
// 	case token.FOR:
// 		return p.parseForStatement()
// 	default:
// 		return p.parseExpressionStatement()
// 	}
// }

// parseStatement dispatches on the current token to parse
// let/var declarations, return, loops, break/continue,
// or falls back to expression statements.
// It also ignores stray '}' and ';'.
func (p *Parser) parseStatement() ast.Statement {
	if p.curToken.Type == token.ILLEGAL {
		p.errors = append(p.errors, fmt.Sprintf("illegal token: %s", p.curToken.Literal))
		p.nextToken()
		return nil
	}

	switch p.curToken.Type {
	case token.IMPORT:
		return p.parseImportStatement()
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
	case token.IDENT:
		// بررسی برای تخصیص (شناسه یا عبارت شاخص‌گذاری)
		if p.peekToken.Type == token.ASSIGN ||
			(p.peekToken.Type == token.LBRACKET && p.peekNextTokenType() == token.ASSIGN) {
			return p.parseAssignStatement()
		}
		return p.parseExpressionStatement()
	case token.RBRACE, token.SEMICOLON:
		return nil
	default:
		return p.parseExpressionStatement()
	}
}

func (p *Parser) peekTokenIs(t token.TokenType) bool {
	return p.peekToken.Type == t
}

// تابع کمکی برای بررسی توکن بعدی (برای پشتیبانی از person["job"] = ...)
func (p *Parser) peekNextTokenType() token.TokenType {
	currentPeek := p.peekToken
	p.nextToken()
	nextType := p.peekToken.Type
	p.peekToken = currentPeek // بازگرداندن peekToken به حالت قبل
	return nextType
}

func (p *Parser) parseLetStatement() ast.Statement {
	stmt := &ast.LetStatement{Token: p.curToken}

	if !p.expectPeek(token.IDENT) {
		return nil
	}

	stmt.Name = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}

	if !p.expectPeek(token.ASSIGN) {
		return nil
	}

	p.nextToken()

	stmt.Value = p.parseExpression(LOWEST)

	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseReturnStatement() ast.Statement {
	stmt := &ast.ReturnStatement{
		Token: p.curToken,
	}

	p.nextToken()

	stmt.ReturnValue = p.parseExpression(LOWEST)

	// اختیاری: چک کردن سمیکلون
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseExpressionStatement() ast.Statement {
	stmt := &ast.ExpressionStatement{Token: p.curToken}
	stmt.Expression = p.parseExpression(LOWEST)

	// مصرف سمیکالن اختیاری
	if p.peekTokenIs(token.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

// parseExpression is the main Pratt parser function.
func (p *Parser) parseExpression(precedence int) ast.Expression {
	prefix := p.prefixParseFns[p.curToken.Type]
	if prefix == nil {
		p.noPrefixParseFnError(p.curToken.Type)
		return nil
	}
	leftExp := prefix()

	for {
		peekPrecedence := p.peekPrecedence()
		if peekPrecedence <= precedence {
			break
		}

		infix := p.infixParseFns[p.peekToken.Type]
		if infix == nil {
			break
		}

		p.nextToken()
		leftExp = infix(leftExp)
	}

	return leftExp
}

// --- Prefix Parse Functions ---

func (p *Parser) parseIdentifier() ast.Expression {
	return &ast.Identifier{
		Token: p.curToken,
		Value: p.curToken.Literal,
	}
}

func (p *Parser) parseIntegerLiteral() ast.Expression {
	lit := &ast.IntegerLiteral{Token: p.curToken}
	value, err := strconv.ParseInt(p.curToken.Literal, 0, 64)
	if err != nil {
		p.errors = append(p.errors, fmt.Sprintf("could not parse %q as integer", p.curToken.Literal))
		return nil
	}
	lit.Value = value
	return lit
}

func (p *Parser) parseStringLiteral() ast.Expression {
	return &ast.StringLiteral{
		Token: p.curToken,
		Value: p.curToken.Literal,
	}
}

func (p *Parser) parseBoolean() ast.Expression {
	return &ast.Boolean{
		Token: p.curToken,
		Value: p.curToken.Type == token.TRUE,
	}
}

func (p *Parser) parsePrefixExpression() ast.Expression {
	expression := &ast.PrefixExpression{
		Token:    p.curToken,
		Operator: p.curToken.Literal,
	}

	p.nextToken()

	expression.Right = p.parseExpression(PREFIX)

	return expression
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

	// 1) parse '(' condition ')'
	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	p.nextToken()
	expr.Condition = p.parseExpression(LOWEST)
	if !p.expectPeek(token.RPAREN) {
		return nil
	}

	// 2) parse consequence block
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	expr.Consequence = p.parseBlockStatement()

	// 3) optional else / else if
	if p.peekToken.Type == token.ELSE {
		p.nextToken() // consume ELSE

		if p.peekToken.Type == token.IF {
			p.nextToken() // consume IF
			// بازگشتی یک IfExpression برمی‌گرداند
			expr.Alternative = p.parseIfExpression()
		} else {
			// simple else { … }
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

	lit.Parameters = p.parseFunctionParameters()

	if !p.expectPeek(token.LBRACE) {
		return nil
	}

	lit.Body = p.parseBlockStatement()

	return lit
}

// parseArrayLiteral parses array literals like [1, 2, x].
// parseArrayLiteral parses array literals like [1, 2, x].
// parseArrayLiteral: پردازش آرایه‌ها مثل [1, 2, "Hello"]
func (p *Parser) parseArrayLiteral() ast.Expression {
	array := &ast.ArrayLiteral{Token: p.curToken}

	// حرکت به بعد از '['
	p.nextToken()

	// بررسی آرایه خالی
	if p.curTokenIs(token.RBRACKET) {
		array.Elements = []ast.Expression{}
		p.nextToken() // مصرف ']'
		return array
	}

	// پردازش عناصر آرایه
	array.Elements = p.parseExpressionList(token.RBRACKET)

	return array
}

// این متد را در فایل parser.go و درون بسته parser اضافه کنید
func (p *Parser) curTokenIs(t token.TokenType) bool {
	return p.curToken.Type == t
}

// همگام‌سازی پارسر پس از خطا در آرایه
func (p *Parser) syncAfterArray() {
	for !p.curTokenIs(token.RBRACKET) &&
		!p.curTokenIs(token.SEMICOLON) &&
		!p.curTokenIs(token.EOF) {
		p.nextToken()
	}

	// اگر به ; یا } یا EOF رسیدیم، حرکت به جلو
	if p.curTokenIs(token.SEMICOLON) ||
		p.curTokenIs(token.RBRACE) ||
		p.curTokenIs(token.EOF) {
		p.nextToken()
	}
}

// parseMapLiteral: { key1: val1, key2: val2, … }
func (p *Parser) parseMapLiteral() ast.Expression {
	lit := &ast.MapLiteral{Token: p.curToken, Pairs: make(map[ast.Expression]ast.Expression)}
	// اگر فوراً } بود:
	if p.peekToken.Type == token.RBRACE {
		p.nextToken() // مصرف {
		p.nextToken() // مصرف }
		return lit
	}
	p.nextToken() // برو به اولین کلید
	for {
		key := p.parseExpression(LOWEST)
		p.expectPeek(token.COLON)
		p.nextToken()
		value := p.parseExpression(LOWEST)
		lit.Pairs[key] = value

		if p.peekToken.Type != token.COMMA {
			break
		}
		p.nextToken() // مصرف کاما
		p.nextToken() // به کلید بعدی
	}
	p.expectPeek(token.RBRACE)
	return lit
}

// --- Infix Parse Functions ---

func (p *Parser) parseInfixExpression(left ast.Expression) ast.Expression {
	expression := &ast.InfixExpression{
		Token:    p.curToken,
		Operator: p.curToken.Literal,
		Left:     left,
	}

	precedence := p.curPrecedence()
	p.nextToken()

	expression.Right = p.parseExpression(precedence)

	return expression
}

// optimized helper: consume closing or sync in REPL
func (p *Parser) consumeClosing(expected token.TokenType) bool {
	if p.peekTokenIs(expected) {
		p.nextToken()
		return true
	}
	if p.isReplMode && (p.peekTokenIs(token.SEMICOLON) || p.peekTokenIs(token.EOF)) {
		p.errors = append(p.errors,
			fmt.Sprintf("warning: missing closing %s, got %s, assuming complete", expected, p.peekToken.Type))
		p.nextToken()
		return true
	}
	p.peekError(expected)
	return false
}

// parseCallExpression handles fn(args...)
func (p *Parser) parseCallExpression(fn ast.Expression) ast.Expression {
	exp := &ast.CallExpression{Token: p.curToken, Function: fn}
	p.nextToken() // consume '('
	exp.Arguments = p.parseExpressionList(token.RPAREN)
	if !p.consumeClosing(token.RPAREN) {
		return nil
	}
	return exp
}

// parseIndexExpression handles arr[index]
func (p *Parser) parseIndexExpression(left ast.Expression) ast.Expression {
	exp := &ast.IndexExpression{Token: p.curToken, Left: left}
	p.nextToken() // consume '['
	exp.Index = p.parseExpression(LOWEST)
	if !p.consumeClosing(token.RBRACKET) {
		return nil
	}
	return exp
}

// --- Helper Functions for Parameters and Arguments ---

func (p *Parser) parseFunctionParameters() []*ast.Identifier {
	identifiers := make([]*ast.Identifier, 0)

	if p.peekToken.Type == token.RPAREN {
		p.nextToken()
		return identifiers
	}

	p.nextToken()

	identifiers = append(identifiers, &ast.Identifier{
		Token: p.curToken,
		Value: p.curToken.Literal,
	})

	for p.peekToken.Type == token.COMMA {
		p.nextToken()
		p.nextToken()

		identifiers = append(identifiers, &ast.Identifier{
			Token: p.curToken,
			Value: p.curToken.Literal,
		})
	}

	if !p.expectPeek(token.RPAREN) {
		return nil
	}

	return identifiers
}

// darix/parser.go

// ... (بقیه کد parser.go)

// parseExpressionList is shared between call args and array literals
func (p *Parser) parseExpressionList(end token.TokenType) []ast.Expression {
	list := []ast.Expression{}
	if p.curTokenIs(end) {
		return list
	}

	// parse first
	expr := p.parseExpression(LOWEST)
	if expr != nil {
		list = append(list, expr)
	}

	// parse comma-separated
	for p.peekTokenIs(token.COMMA) {
		p.nextToken() // consume ','
		p.nextToken() // move to next expr
		expr := p.parseExpression(LOWEST)
		if expr != nil {
			list = append(list, expr)
		}
	}

	// expect closing token
	if p.peekTokenIs(end) {
		p.nextToken()
	} else if p.isReplMode && p.peekTokenIs(token.SEMICOLON) {
		// consume semicolon
		p.nextToken()
	} else {
		p.errors = append(p.errors,
			fmt.Sprintf("expected closing %s, got %s", end, p.peekToken.Type))
	}

	return list
}

func (p *Parser) parseBlockStatement() *ast.BlockStatement {
	block := &ast.BlockStatement{
		Token:      p.curToken,
		Statements: []ast.Statement{},
	}
	// اولین توکن داخل بلوک
	p.nextToken()

	// تا زمانی که به '}' یا EOF نرسیده‌ایم ادامه بده
	for p.curToken.Type != token.RBRACE && p.curToken.Type != token.EOF {
		stmt := p.parseStatement()
		if stmt != nil {
			block.Statements = append(block.Statements, stmt)
		}
		p.nextToken()
	}

	return block
}

// --- Helper Functions for Tokens ---

func (p *Parser) expectPeek(t token.TokenType) bool {
	if p.peekToken.Type == t {
		p.nextToken()
		return true
	}

	// در حالت REPL، EOF یا سمیکالن را به عنوان پایان عبارت بپذیر
	if p.isReplMode && (p.peekToken.Type == token.EOF || p.peekToken.Type == token.SEMICOLON) {
		p.errors = append(p.errors, fmt.Sprintf("warning: expected %s, got %s, assuming complete expression", t, p.peekToken.Type))
		return true // به عنوان یافتن توکن در نظر بگیر
	}

	p.peekError(t)
	return false
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

// --- Error Handling Functions ---

func (p *Parser) Errors() []string {
	return p.errors
}

func (p *Parser) peekError(t token.TokenType) {
	msg := fmt.Sprintf("expected next token to be %s, got %s instead",
		t, p.peekToken.Type)
	p.errors = append(p.errors, msg)
}

func (p *Parser) noPrefixParseFnError(t token.TokenType) {
	msg := fmt.Sprintf("no prefix parse function for %s found", t)
	p.errors = append(p.errors, msg)
}

func (p *Parser) parseFloatLiteral() ast.Expression {
	lit := &ast.FloatLiteral{Token: p.curToken}
	val, err := strconv.ParseFloat(p.curToken.Literal, 64)
	if err != nil {
		p.errors = append(p.errors, fmt.Sprintf("could not parse %q as float", p.curToken.Literal))
		return nil
	}
	lit.Value = val
	return lit
}

// parseWhileStatement parses: while (condition) { body }
func (p *Parser) parseWhileStatement() ast.Statement {
	stmt := &ast.WhileStatement{Token: p.curToken}

	// انتظار '('
	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	p.nextToken()
	stmt.Condition = p.parseExpression(LOWEST)
	if !p.expectPeek(token.RPAREN) {
		return nil
	}
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.Body = p.parseBlockStatement()
	return stmt
}

// parseForStatement parses: for (init; cond; post) { body }
func (p *Parser) parseForStatement() ast.Statement {
	stmt := &ast.ForStatement{Token: p.curToken}

	// 1) باید ببینیم '(' بعد از for هست
	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	// آماده‌سازی برای خواندن اولین توکن داخل ()
	p.nextToken()

	// ——— init ———
	if p.curToken.Type == token.SEMICOLON {
		// init خالی
	} else if p.curToken.Type == token.VAR {
		// اگر let داریم، از parseLetStatement کمک می‌گیریم (که ; را هم در صورت وجود می‌خورد)
		stmt.Init = p.parseLetStatement()
	} else {
		// بقیه‌اش را به‌صورت expressionStatement بخوان
		stmt.Init = p.parseExpressionStatement()
	}
	// بعد از init، باید curToken روی سمیکالن باشد
	if p.curToken.Type != token.SEMICOLON {
		p.peekError(token.SEMICOLON)
		return nil
	}
	// عبور از ';'
	p.nextToken()

	// ——— condition ———
	if p.curToken.Type != token.SEMICOLON {
		// تا قبلِ از ';' شرط را به‌عنوان یک expression بخوان
		stmt.Condition = p.parseExpression(LOWEST)
	}
	// بعد از این بخش حتما باید semicolon باشد
	if !p.expectPeek(token.SEMICOLON) {
		return nil
	}

	// ——— post ———
	p.nextToken() // به اولین توکن بعد از ';'
	// Post
	if p.curToken.Type != token.RPAREN {
		if p.curToken.Type == token.IDENT && p.peekToken.Type == token.ASSIGN {
			stmt.Post = p.parseAssignStatement()
		} else {
			stmt.Post = p.parseExpressionStatement()
		}
	}

	// در این زبان پس از post نباید سمیکولن اضافی داشته باشیم
	// curToken حالا یا روی ')' است یا بعد از semicolonِ expressionStatement

	// ——— پایان پرانتز ———
	if !p.expectPeek(token.RPAREN) {
		return nil
	}
	// و بعد باید بلوک { ... } باشد
	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.Body = p.parseBlockStatement()
	return stmt
}

// parseAssignStatement parses: IDENT = <expr> ;
// parseAssignStatement: پردازش تخصیص‌ها مثل var x = 10;
func (p *Parser) parseAssignStatement() ast.Statement {
	stmt := &ast.AssignStatement{Token: p.curToken}

	// پارس هدف تخصیص (شناسه یا عبارت شاخص‌گذاری)
	target := p.parseExpression(LOWEST)
	if target == nil {
		p.errors = append(p.errors, "invalid assignment target")
		return nil
	}

	// بررسی اینکه هدف یک شناسه یا عبارت شاخص‌گذاری است
	_, isIdentifier := target.(*ast.Identifier)
	_, isIndexExpression := target.(*ast.IndexExpression)
	if !isIdentifier && !isIndexExpression {
		p.errors = append(p.errors, "invalid assignment target: expected identifier or index expression")
		return nil
	}

	stmt.Target = target

	// انتظار توکن تخصیص
	if !p.expectPeek(token.ASSIGN) {
		return nil
	}

	// مصرف توکن تخصیص و حرکت به سمت راست
	p.nextToken()

	// پارس سمت راست
	stmt.Value = p.parseExpression(LOWEST)
	if stmt.Value == nil {
		p.errors = append(p.errors, "invalid expression in assignment")
		return nil
	}

	// مصرف سمیکالن اختیاری
	if p.peekTokenIs(token.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

// parseBreakStatement parses: break;
func (p *Parser) parseBreakStatement() ast.Statement {
	stmt := &ast.BreakStatement{Token: p.curToken}
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	return stmt
}

// parseContinueStatement parses: continue;
func (p *Parser) parseContinueStatement() ast.Statement {
	stmt := &ast.ContinueStatement{Token: p.curToken}
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	return stmt
}

// parseFunctionDeclaration: parses "func name(params) { body }"
func (p *Parser) parseFunctionDeclaration() ast.Statement {
	stmt := &ast.FunctionDeclaration{Token: p.curToken}

	// مصرف 'func'
	if !p.expectPeek(token.IDENT) {
		return nil
	}
	stmt.Name = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}

	if !p.expectPeek(token.LPAREN) {
		return nil
	}
	stmt.Parameters = p.parseFunctionParameters() // همانی که برای literals داریم

	if !p.expectPeek(token.LBRACE) {
		return nil
	}
	stmt.Body = p.parseBlockStatement()
	return stmt
}

// parseImportStatement: import "module.sht";
func (p *Parser) parseImportStatement() ast.Statement {
	stmt := &ast.ImportStatement{Token: p.curToken}

	if !p.expectPeek(token.STRING) { // مسیر باید رشته باشد
		return nil
	}
	stmt.Path = &ast.StringLiteral{Token: p.curToken, Value: p.curToken.Literal}

	// سمیکالن اختیاری
	if p.peekToken.Type == token.SEMICOLON {
		p.nextToken()
	}
	return stmt
}

// parses `identifier = expression`
func (p *Parser) parseAssignmentExpression(left ast.Expression) ast.Expression {
	// بررسی اینکه سمت چپ یک شناسه یا عبارت شاخص‌گذاری است
	_, isIdentifier := left.(*ast.Identifier)
	_, isIndexExpression := left.(*ast.IndexExpression)
	if !isIdentifier && !isIndexExpression {
		p.errors = append(p.errors, fmt.Sprintf("invalid assignment target: expected identifier or index expression, got %T", left))
		return nil
	}

	expr := &ast.AssignExpression{
		Token: p.curToken, // توکن '='
		Name:  left,       // استفاده از Expression به جای Identifier
	}

	// پارس سمت راست
	p.nextToken()
	expr.Value = p.parseExpression(LOWEST)
	if expr.Value == nil {
		p.errors = append(p.errors, fmt.Sprintf("invalid expression in assignment at %s", p.curToken.Literal))
		return nil
	}

	return expr
}

// تبدیل while به expression
func (p *Parser) parseWhileExpression() ast.Expression {
	// ابتدا parseWhileStatement را فراخوانی می‌کنیم
	stmt := p.parseWhileStatement()

	// سپس با استفاده از type assertion، آن را به *ast.WhileStatement تبدیل می‌کنیم
	whileStmt, ok := stmt.(*ast.WhileStatement)
	if !ok || whileStmt == nil {
		p.errors = append(p.errors, "expected while statement")
		return nil
	}

	// حالا می‌توانیم به فیلدهای خاص دسترسی پیدا کنیم
	return &ast.WhileExpression{
		Token:     whileStmt.Token,
		Condition: whileStmt.Condition,
		Body:      whileStmt.Body,
	}
}

// تبدیل for به expression
func (p *Parser) parseForExpression() ast.Expression {
	// ابتدا parseForStatement را فراخوانی می‌کنیم
	stmt := p.parseForStatement()

	// سپس با استفاده از type assertion، آن را به *ast.ForStatement تبدیل می‌کنیم
	forStmt, ok := stmt.(*ast.ForStatement)
	if !ok || forStmt == nil {
		p.errors = append(p.errors, "expected for statement")
		return nil
	}

	// حالا می‌توانیم به فیلدهای خاص دسترسی پیدا کنیم
	return &ast.ForExpression{
		Token:     forStmt.Token,
		Init:      forStmt.Init,
		Condition: forStmt.Condition,
		Post:      forStmt.Post,
		Body:      forStmt.Body,
	}
}

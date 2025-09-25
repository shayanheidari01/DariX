// darix/ast/ast.go

package ast

import (
	"darix/token"
	"fmt"
	"sort"
	"strings"
)

// Node is the base interface for all AST nodes.
type Node interface {
	TokenLiteral() string
	String() string
}

// Statement is the interface for all statement nodes.
type Statement interface {
	Node
	statementNode()
}

// Expression is the interface for all expression nodes.
type Expression interface {
	Node
	expressionNode()
}

// Program is the root node of the AST.
type Program struct {
	Statements []Statement
}

func (p *Program) TokenLiteral() string {
	if len(p.Statements) > 0 {
		return p.Statements[0].TokenLiteral()
	}
	return ""
}

func (p *Program) String() string {
	return statementsString(p.Statements)
}

func statementsString(stmts []Statement) string {
	if len(stmts) == 0 {
		return ""
	}

	var out strings.Builder
	for _, stmt := range stmts {
		if stmt == nil {
			continue
		}
		out.WriteString(stmt.String())
	}
	return out.String()
}

func statementString(stmt Statement) string {
	if stmt == nil {
		return ""
	}
	return stmt.String()
}

func expressionString(expr Expression) string {
	if expr == nil {
		return ""
	}
	return expr.String()
}

func identifierString(ident *Identifier) string {
	if ident == nil {
		return ""
	}
	return ident.String()
}

func identifierStrings(identifiers []*Identifier) []string {
	if len(identifiers) == 0 {
		return nil
	}

	out := make([]string, 0, len(identifiers))
	for _, ident := range identifiers {
		if ident == nil {
			continue
		}
		out = append(out, ident.String())
	}
	return out
}

func expressionStrings(expressions []Expression) []string {
	if len(expressions) == 0 {
		return nil
	}

	out := make([]string, 0, len(expressions))
	for _, expr := range expressions {
		if expr == nil {
			continue
		}
		out = append(out, expr.String())
	}
	return out
}

func blockString(block *BlockStatement) string {
	if block == nil {
		return "{}"
	}
	return block.String()
}

func mapPairStrings(pairs map[Expression]Expression) []string {
	if len(pairs) == 0 {
		return nil
	}

	out := make([]string, 0, len(pairs))
	for key, value := range pairs {
		out = append(out, expressionString(key)+":"+expressionString(value))
	}
	sort.Strings(out)
	return out
}

// ImportStatement represents import statements
type ImportStatement struct {
	Token token.Token    // IMPORT token
	Path  *StringLiteral // module file path string
}

func (is *ImportStatement) statementNode()       {}
func (is *ImportStatement) TokenLiteral() string { return is.Token.Literal }
func (is *ImportStatement) String() string {
	var out strings.Builder
	out.WriteString("import")
	if is.Path != nil {
		out.WriteString(" ")
		out.WriteString(expressionString(is.Path))
	}
	out.WriteString(";")
	return out.String()
}

// LetStatement represents a 'var' statement.
type LetStatement struct {
	Token token.Token // the 'var' token
	Name  *Identifier
	Value Expression
}

func (ls *LetStatement) statementNode()       {}
func (ls *LetStatement) TokenLiteral() string { return ls.Token.Literal }
func (ls *LetStatement) String() string {
	var out strings.Builder
	out.WriteString(ls.TokenLiteral())
	out.WriteString(" ")
	out.WriteString(identifierString(ls.Name))
	out.WriteString(" = ")
	out.WriteString(expressionString(ls.Value))
	out.WriteString(";")
	return out.String()
}

// AssignStatement represents assignment statements
type AssignStatement struct {
	Token  token.Token // assignment token
	Target Expression  // assignment target (identifier or index expression)
	Value  Expression  // assignment value
}

func (as *AssignStatement) statementNode()       {}
func (as *AssignStatement) TokenLiteral() string { return as.Token.Literal }
func (as *AssignStatement) String() string {
	var out strings.Builder
	out.WriteString(expressionString(as.Target))
	out.WriteString(" = ")
	out.WriteString(expressionString(as.Value))
	out.WriteString(";")
	return out.String()
}

// ReturnStatement represents a 'return' statement.
type ReturnStatement struct {
	Token       token.Token // the 'return' token
	ReturnValue Expression
}

func (rs *ReturnStatement) statementNode()       {}
func (rs *ReturnStatement) TokenLiteral() string { return rs.Token.Literal }
func (rs *ReturnStatement) String() string {
	var out strings.Builder
	out.WriteString(rs.TokenLiteral())
	out.WriteString(" ")
	out.WriteString(expressionString(rs.ReturnValue))
	out.WriteString(";")
	return out.String()
}

// ExpressionStatement represents a standalone expression.
type ExpressionStatement struct {
	Token      token.Token // the first token of the expression
	Expression Expression
}

func (es *ExpressionStatement) statementNode()       {}
func (es *ExpressionStatement) TokenLiteral() string { return es.Token.Literal }
func (es *ExpressionStatement) String() string {
	return expressionString(es.Expression)
}

// BlockStatement represents a block of statements enclosed in braces.
type BlockStatement struct {
	Token      token.Token
	Statements []Statement
}

func (bs *BlockStatement) statementNode()       {}
func (bs *BlockStatement) expressionNode()      {} // Add expression interface implementation
func (bs *BlockStatement) TokenLiteral() string { return bs.Token.Literal }
func (bs *BlockStatement) String() string {
	var out strings.Builder
	out.WriteString("{")
	out.WriteString(statementsString(bs.Statements))
	out.WriteString("}")
	return out.String()
}

// StandaloneBlockStatement represents standalone block statements that don't create new scope
type StandaloneBlockStatement struct {
	Token token.Token
	Block *BlockStatement
}

func (sbs *StandaloneBlockStatement) statementNode()       {}
func (sbs *StandaloneBlockStatement) TokenLiteral() string { return sbs.Token.Literal }
func (sbs *StandaloneBlockStatement) String() string {
	return blockString(sbs.Block)
}

// BreakStatement represents a 'break' statement
type BreakStatement struct {
	Token token.Token // the 'break' token
}

func (bs *BreakStatement) statementNode()       {}
func (bs *BreakStatement) TokenLiteral() string { return bs.Token.Literal }
func (bs *BreakStatement) String() string       { return "break;" }

// ContinueStatement represents a 'continue' statement
type ContinueStatement struct {
	Token token.Token // the 'continue' token
}

func (cs *ContinueStatement) statementNode()       {}
func (cs *ContinueStatement) TokenLiteral() string { return cs.Token.Literal }
func (cs *ContinueStatement) String() string       { return "continue;" }

// WhileStatement represents while loops as statements
type WhileStatement struct {
	Token     token.Token     // 'while' token
	Condition Expression      // condition
	Body      *BlockStatement // body
}

func (ws *WhileStatement) statementNode()       {}
func (ws *WhileStatement) TokenLiteral() string { return ws.Token.Literal }
func (ws *WhileStatement) String() string {
	condition := ""
	if ws.Condition != nil {
		condition = ws.Condition.String()
	}
	body := ""
	if ws.Body != nil {
		body = ws.Body.String()
	}
	return fmt.Sprintf("while(%s) %s", condition, body)
}

// ForStatement represents for loops as statements
type ForStatement struct {
	Token     token.Token     // 'for' token
	Init      Statement       // initialization statement (or nil)
	Condition Expression      // loop condition (or nil means true)
	Post      Statement       // post-iteration statement (or nil)
	Body      *BlockStatement // body
}

func (fs *ForStatement) statementNode()       {}
func (fs *ForStatement) TokenLiteral() string { return fs.Token.Literal }
func (fs *ForStatement) String() string {
	init := ""
	if fs.Init != nil {
		init = fs.Init.String()
	}
	condition := ""
	if fs.Condition != nil {
		condition = fs.Condition.String()
	}
	post := ""
	if fs.Post != nil {
		post = fs.Post.String()
	}
	body := ""
	if fs.Body != nil {
		body = fs.Body.String()
	}
	return fmt.Sprintf("for(%s; %s; %s) %s", init, condition, post, body)
}

// FunctionDeclaration represents function declarations
type FunctionDeclaration struct {
	Token      token.Token     // FUNC token
	Name       *Identifier     // function name
	Parameters []*Identifier   // parameters
	Body       *BlockStatement // body
}

func (fd *FunctionDeclaration) statementNode()       {}
func (fd *FunctionDeclaration) TokenLiteral() string { return fd.Token.Literal }
func (fd *FunctionDeclaration) String() string {
	var out strings.Builder
	params := make([]string, 0, len(fd.Parameters))
	for _, p := range fd.Parameters {
		if p != nil {
			params = append(params, p.String())
		}
	}
	out.WriteString("func ")
	if fd.Name != nil {
		out.WriteString(fd.Name.String())
	}
	out.WriteString("(")
	out.WriteString(strings.Join(params, ", "))
	out.WriteString(") ")
	if fd.Body != nil {
		out.WriteString(fd.Body.String())
	}
	return out.String()
}

// Identifier represents an identifier (variable name).
type Identifier struct {
	Token token.Token // the IDENT token
	Value string
}

func (i *Identifier) expressionNode()      {}
func (i *Identifier) TokenLiteral() string { return i.Token.Literal }
func (i *Identifier) String() string       { return i.Value }

// IntegerLiteral represents an integer literal.
type IntegerLiteral struct {
	Token token.Token
	Value int64
}

func (il *IntegerLiteral) expressionNode()      {}
func (il *IntegerLiteral) TokenLiteral() string { return il.Token.Literal }
func (il *IntegerLiteral) String() string       { return il.Token.Literal }

// FloatLiteral represents a float literal.
type FloatLiteral struct {
	Token token.Token // FLOAT token
	Value float64
}

func (fl *FloatLiteral) expressionNode()      {}
func (fl *FloatLiteral) TokenLiteral() string { return fl.Token.Literal }
func (fl *FloatLiteral) String() string       { return fl.Token.Literal }

// StringLiteral represents a string literal.
type StringLiteral struct {
	Token token.Token
	Value string
}

func (sl *StringLiteral) expressionNode()      {}
func (sl *StringLiteral) TokenLiteral() string { return sl.Token.Literal }
func (sl *StringLiteral) String() string       { return `"` + sl.Value + `"` }

// Boolean represents a boolean literal (true or false).
type Boolean struct {
	Token token.Token
	Value bool
}

func (b *Boolean) expressionNode()      {}
func (b *Boolean) TokenLiteral() string { return b.Token.Literal }
func (b *Boolean) String() string       { return b.Token.Literal }

// Null represents null values
type Null struct {
	Token token.Token
}

func (n *Null) expressionNode()      {}
func (n *Null) TokenLiteral() string { return n.Token.Literal }
func (n *Null) String() string       { return "null" }

// AssignExpression represents assignment expressions like x = y
type AssignExpression struct {
	Token token.Token // '=' token
	Name  Expression  // identifier or index expression
	Value Expression  // right-hand side value
}

func (ae *AssignExpression) expressionNode()      {}
func (ae *AssignExpression) TokenLiteral() string { return ae.Token.Literal }
func (ae *AssignExpression) String() string {
	var out strings.Builder
	out.WriteString(expressionString(ae.Name))
	out.WriteString(" = ")
	out.WriteString(expressionString(ae.Value))
	return out.String()
}

// PrefixExpression represents a prefix operator expression (e.g., -x, !y).
type PrefixExpression struct {
	Token    token.Token // The prefix token, e.g. !
	Operator string
	Right    Expression
}

func (pe *PrefixExpression) expressionNode()      {}
func (pe *PrefixExpression) TokenLiteral() string { return pe.Token.Literal }
func (pe *PrefixExpression) String() string {
	var out strings.Builder
	out.WriteString("(")
	out.WriteString(pe.Operator)
	out.WriteString(expressionString(pe.Right))
	out.WriteString(")")
	return out.String()
}

// InfixExpression represents an infix operator expression (e.g., x + y).
type InfixExpression struct {
	Token    token.Token // The operator token, e.g. +
	Left     Expression
	Operator string
	Right    Expression
}

func (ie *InfixExpression) expressionNode()      {}
func (ie *InfixExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *InfixExpression) String() string {
	var out strings.Builder
	out.WriteString("(")
	out.WriteString(expressionString(ie.Left))
	out.WriteString(" " + ie.Operator + " ")
	out.WriteString(expressionString(ie.Right))
	out.WriteString(")")
	return out.String()
}

// IfExpression represents an 'if' expression.
type IfExpression struct {
	Token       token.Token // The 'if' token
	Condition   Expression
	Consequence *BlockStatement
	Alternative Expression
}

func (ie *IfExpression) expressionNode()      {}
func (ie *IfExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *IfExpression) String() string {
	var out strings.Builder
	out.WriteString("if")
	out.WriteString(expressionString(ie.Condition))
	out.WriteString(" ")
	out.WriteString(blockString(ie.Consequence))
	if ie.Alternative != nil {
		out.WriteString("else ")
		out.WriteString(ie.Alternative.String())
	}
	return out.String()
}

// FunctionLiteral represents a function literal.
type FunctionLiteral struct {
	Token      token.Token // The 'func' token
	Parameters []*Identifier
	Body       *BlockStatement
}

func (fl *FunctionLiteral) expressionNode()      {}
func (fl *FunctionLiteral) TokenLiteral() string { return fl.Token.Literal }
func (fl *FunctionLiteral) String() string {
	var out strings.Builder
	out.WriteString(fl.TokenLiteral())
	out.WriteString("(")
	out.WriteString(strings.Join(identifierStrings(fl.Parameters), ", "))
	out.WriteString(") ")
	out.WriteString(blockString(fl.Body))
	return out.String()
}

// CallExpression represents a function call expression.
type CallExpression struct {
	Token     token.Token // The '(' token
	Function  Expression  // Identifier or FunctionLiteral
	Arguments []Expression
}

func (ce *CallExpression) expressionNode()      {}
func (ce *CallExpression) TokenLiteral() string { return ce.Token.Literal }
func (ce *CallExpression) String() string {
	var out strings.Builder
	out.WriteString(expressionString(ce.Function))
	out.WriteString("(")
	out.WriteString(strings.Join(expressionStrings(ce.Arguments), ", "))
	out.WriteString(")")
	return out.String()
}

// ArrayLiteral represents an array literal like [1, 2, x].
type ArrayLiteral struct {
	Token    token.Token // the '[' token
	Elements []Expression
}

func (al *ArrayLiteral) expressionNode()      {}
func (al *ArrayLiteral) TokenLiteral() string { return al.Token.Literal }
func (al *ArrayLiteral) String() string {
	var out strings.Builder
	out.WriteString("[")
	out.WriteString(strings.Join(expressionStrings(al.Elements), ", "))
	out.WriteString("]")
	return out.String()
}

// MapLiteral represents a map literal like {"key": value, x: y}.
type MapLiteral struct {
	Token token.Token // the '{' token
	Pairs map[Expression]Expression
}

func (ml *MapLiteral) expressionNode()      {}
func (ml *MapLiteral) TokenLiteral() string { return ml.Token.Literal }
func (ml *MapLiteral) String() string {
	var out strings.Builder
	out.WriteString("{")
	out.WriteString(strings.Join(mapPairStrings(ml.Pairs), ", "))
	out.WriteString("}")
	return out.String()
}

// IndexExpression represents array/map indexing: arr[index]
type IndexExpression struct {
    Token token.Token // '[' token
    Left  Expression
    Index Expression
}

func (ie *IndexExpression) expressionNode()      {}
func (ie *IndexExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *IndexExpression) String() string {
	return fmt.Sprintf("(%s[%s])", expressionString(ie.Left), expressionString(ie.Index))
}

// MemberExpression represents attribute access: obj.prop
type MemberExpression struct {
    Token    token.Token // '.' token
    Left     Expression
    Property *Identifier
}

func (me *MemberExpression) expressionNode()      {}
func (me *MemberExpression) TokenLiteral() string { return me.Token.Literal }
func (me *MemberExpression) String() string {
    return fmt.Sprintf("(%s.%s)", expressionString(me.Left), identifierString(me.Property))
}

// ClassDeclaration represents: class Name { ... }
type ClassDeclaration struct {
    Token token.Token // 'class' token
    Name  *Identifier
    Body  *BlockStatement
}

func (cd *ClassDeclaration) statementNode()       {}
func (cd *ClassDeclaration) TokenLiteral() string { return cd.Token.Literal }
func (cd *ClassDeclaration) String() string {
    var out strings.Builder
    out.WriteString("class ")
    out.WriteString(identifierString(cd.Name))
    out.WriteString(" ")
    out.WriteString(blockString(cd.Body))
    return out.String()
}

// WhileExpression represents a while loop used as an expression
type WhileExpression struct {
    Token     token.Token     // 'while' token
    Condition Expression      // condition
    Body      *BlockStatement // body
}

func (we *WhileExpression) expressionNode()      {}
func (we *WhileExpression) TokenLiteral() string { return we.Token.Literal }
func (we *WhileExpression) String() string {
	return fmt.Sprintf("while(%s) %s", expressionString(we.Condition), blockString(we.Body))
}


// ThrowStatement represents a 'throw' or 'raise' statement
type ThrowStatement struct {
    Token     token.Token // the 'throw' or 'raise' token
    Exception Expression  // the exception to throw
}

func (ts *ThrowStatement) statementNode()       {}
func (ts *ThrowStatement) TokenLiteral() string { return ts.Token.Literal }
func (ts *ThrowStatement) String() string {
    var out strings.Builder
    out.WriteString(ts.TokenLiteral())
    out.WriteString(" ")
    out.WriteString(expressionString(ts.Exception))
    out.WriteString(";")
    return out.String()
}

// TryStatement represents a try-catch-finally block
type TryStatement struct {
	Token        token.Token     // the 'try' token
	TryBlock     *BlockStatement // the try block
	CatchClauses []*CatchClause  // catch clauses
	FinallyBlock *BlockStatement // the finally block (optional)
}

func (ts *TryStatement) statementNode()       {}
func (ts *TryStatement) TokenLiteral() string { return ts.Token.Literal }
func (ts *TryStatement) String() string {
	var out strings.Builder
	out.WriteString("try ")
	out.WriteString(blockString(ts.TryBlock))

	for _, catchClause := range ts.CatchClauses {
		if catchClause == nil {
			continue
		}
		out.WriteString(" ")
		out.WriteString(catchClause.String())
	}

	if ts.FinallyBlock != nil {
		out.WriteString(" finally ")
		out.WriteString(ts.FinallyBlock.String())
	}

	return out.String()
}

// CatchClause represents a catch clause in a try-catch block
type CatchClause struct {
	Token         token.Token     // the 'catch' token
	ExceptionType *Identifier     // exception type to catch (optional, catches all if nil)
	Variable      *Identifier     // variable to bind the exception to (optional)
	CatchBlock    *BlockStatement // the catch block
}

func (cc *CatchClause) String() string {
	var out strings.Builder
	out.WriteString("catch")

	if cc.ExceptionType != nil {
		out.WriteString(" (")
		out.WriteString(cc.ExceptionType.String())
		if name := identifierString(cc.Variable); name != "" {
			out.WriteString(" ")
			out.WriteString(name)
		}
		out.WriteString(")")
	} else if name := identifierString(cc.Variable); name != "" {
		out.WriteString(" (")
		out.WriteString(name)
		out.WriteString(")")
	}

	if cc.CatchBlock != nil {
		out.WriteString(" ")
		out.WriteString(cc.CatchBlock.String())
	}

	return out.String()
}

// ExceptionExpression represents an exception object creation
type ExceptionExpression struct {
	Token   token.Token // the first token
	Type    *Identifier // exception type
	Message Expression  // exception message
}

func (ee *ExceptionExpression) expressionNode()      {}
func (ee *ExceptionExpression) TokenLiteral() string { return ee.Token.Literal }
func (ee *ExceptionExpression) String() string {
	var out strings.Builder
	out.WriteString(identifierString(ee.Type))
	out.WriteString("(")
	out.WriteString(expressionString(ee.Message))
	out.WriteString(")")
	return out.String()
}

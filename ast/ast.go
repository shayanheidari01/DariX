// darix/ast/ast.go

package ast

import (
	"darix/token"
	"fmt"
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
	var out strings.Builder
	for _, s := range p.Statements {
		out.WriteString(s.String())
	}
	return out.String()
}

// ImportStatement represents import statements
type ImportStatement struct {
	Token token.Token    // IMPORT token
	Path  *StringLiteral // module file path string
}

func (is *ImportStatement) statementNode()       {}
func (is *ImportStatement) TokenLiteral() string { return is.Token.Literal }
func (is *ImportStatement) String() string {
	if is.Path == nil {
		return "import;"
	}
	return fmt.Sprintf("import %s;", is.Path.Value)
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
	if ls.Name != nil {
		out.WriteString(ls.Name.String())
	}
	out.WriteString(" = ")
	if ls.Value != nil {
		out.WriteString(ls.Value.String())
	}
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
	if as.Target != nil {
		out.WriteString(as.Target.String())
	}
	out.WriteString(" = ")
	if as.Value != nil {
		out.WriteString(as.Value.String())
	}
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
	if rs.ReturnValue != nil {
		out.WriteString(rs.ReturnValue.String())
	}
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
	if es.Expression != nil {
		return es.Expression.String()
	}
	return ""
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
	for _, s := range bs.Statements {
		out.WriteString(s.String())
	}
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
	if sbs.Block != nil {
		return sbs.Block.String()
	}
	return "{}"
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
	if ae.Name != nil {
		out.WriteString(ae.Name.String())
	}
	out.WriteString(" = ")
	if ae.Value != nil {
		out.WriteString(ae.Value.String())
	}
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
	if pe.Right != nil {
		out.WriteString(pe.Right.String())
	}
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
	if ie.Left != nil {
		out.WriteString(ie.Left.String())
	}
	out.WriteString(" " + ie.Operator + " ")
	if ie.Right != nil {
		out.WriteString(ie.Right.String())
	}
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
	if ie.Condition != nil {
		out.WriteString(ie.Condition.String())
	}
	out.WriteString(" ")
	if ie.Consequence != nil {
		out.WriteString(ie.Consequence.String())
	}
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
	params := make([]string, 0, len(fl.Parameters))
	for _, p := range fl.Parameters {
		if p != nil {
			params = append(params, p.String())
		}
	}
	out.WriteString(fl.TokenLiteral())
	out.WriteString("(")
	out.WriteString(strings.Join(params, ", "))
	out.WriteString(") ")
	if fl.Body != nil {
		out.WriteString(fl.Body.String())
	}
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
	args := make([]string, 0, len(ce.Arguments))
	for _, a := range ce.Arguments {
		if a != nil {
			args = append(args, a.String())
		}
	}
	if ce.Function != nil {
		out.WriteString(ce.Function.String())
	}
	out.WriteString("(")
	out.WriteString(strings.Join(args, ", "))
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
	elements := make([]string, 0, len(al.Elements))
	for _, el := range al.Elements {
		if el != nil {
			elements = append(elements, el.String())
		}
	}
	out.WriteString("[")
	out.WriteString(strings.Join(elements, ", "))
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
	pairs := make([]string, 0, len(ml.Pairs))
	for key, value := range ml.Pairs {
		keyStr := ""
		if key != nil {
			keyStr = key.String()
		}
		valueStr := ""
		if value != nil {
			valueStr = value.String()
		}
		pairs = append(pairs, keyStr+":"+valueStr)
	}
	out.WriteString("{")
	out.WriteString(strings.Join(pairs, ", "))
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
	leftStr := ""
	if ie.Left != nil {
		leftStr = ie.Left.String()
	}
	indexStr := ""
	if ie.Index != nil {
		indexStr = ie.Index.String()
	}
	return fmt.Sprintf("(%s[%s])", leftStr, indexStr)
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
	condition := ""
	if we.Condition != nil {
		condition = we.Condition.String()
	}
	body := ""
	if we.Body != nil {
		body = we.Body.String()
	}
	return fmt.Sprintf("while(%s) %s", condition, body)
}

// ForExpression represents a for loop used as an expression
type ForExpression struct {
	Token     token.Token     // 'for' token
	Init      Statement       // initialization statement
	Condition Expression      // loop condition
	Post      Statement       // post-iteration statement
	Body      *BlockStatement // body
}

func (fe *ForExpression) expressionNode()      {}
func (fe *ForExpression) TokenLiteral() string { return fe.Token.Literal }
func (fe *ForExpression) String() string {
	init := ""
	if fe.Init != nil {
		init = fe.Init.String()
	}
	condition := ""
	if fe.Condition != nil {
		condition = fe.Condition.String()
	}
	post := ""
	if fe.Post != nil {
		post = fe.Post.String()
	}
	body := ""
	if fe.Body != nil {
		body = fe.Body.String()
	}
	return fmt.Sprintf("for(%s; %s; %s) %s", init, condition, post, body)
}

// ===== EXCEPTION HANDLING AST NODES =====

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
	if ts.Exception != nil {
		out.WriteString(ts.Exception.String())
	}
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
	if ts.TryBlock != nil {
		out.WriteString(ts.TryBlock.String())
	}

	for _, catchClause := range ts.CatchClauses {
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
		if cc.Variable != nil {
			out.WriteString(" ")
			out.WriteString(cc.Variable.String())
		}
		out.WriteString(")")
	} else if cc.Variable != nil {
		out.WriteString(" (")
		out.WriteString(cc.Variable.String())
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
	if ee.Type != nil {
		out.WriteString(ee.Type.String())
	}
	out.WriteString("(")
	if ee.Message != nil {
		out.WriteString(ee.Message.String())
	}
	out.WriteString(")")
	return out.String()
}

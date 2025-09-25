// darix/parser/decorators.go
package parser

import (
	"darix/ast"
	"darix/token"
)

// parseDecoratedDefinition parses one or more decorators followed by a function or class definition
// Grammar (informal):
//   DecoratedDef := ('@' Expression (';' )?) + (FunctionDeclaration | ClassDeclaration)
// Decorators are stored on the resulting AST node and applied at runtime.
func (p *Parser) parseDecoratedDefinition() ast.Statement {
	decorators := make([]ast.Expression, 0, 2)
	// Current token should be '@'
	for p.curToken.Type == token.AT {
		// Move to start of decorator expression
		p.nextToken()
		dec := p.parseExpression(LOWEST)
		if dec == nil {
			return nil
		}
		decorators = append(decorators, dec)

		// Optional semicolon after decorator expression
		if p.peekToken.Type == token.SEMICOLON {
			p.nextToken()
		}

		// If another '@' follows, consume and continue loop
		if p.peekToken.Type == token.AT {
			p.nextToken()
			continue
		}
		break
	}

	// Next token must be 'func' or 'class'
	if p.peekToken.Type != token.FUNCTION && p.peekToken.Type != token.CLASS {
		p.addError("decorator must be followed by function or class definition")
		return nil
	}
	p.nextToken()

	if p.curToken.Type == token.FUNCTION {
		stmt := p.parseFunctionDeclaration()
		if fd, ok := stmt.(*ast.FunctionDeclaration); ok {
			fd.Decorators = decorators
		}
		return stmt
	}

	// CLASS
	stmt := p.parseClassDeclaration()
	if cd, ok := stmt.(*ast.ClassDeclaration); ok {
		cd.Decorators = decorators
	}
	return stmt
}

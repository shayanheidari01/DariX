package compiler

import (
	"darix/ast"
	"darix/code"
	"darix/internal/version"
	"darix/object"
	"darix/token"
	"fmt"
)

type Compiler struct {
	instructions code.Instructions
	constants    []object.Object
	symbolTable  *SymbolTable
	debugEntries []DebugEntry
}

func New() *Compiler {
	return &Compiler{
		instructions: code.Instructions{},
		constants:    []object.Object{},
		symbolTable:  NewSymbolTable(),
		debugEntries: make([]DebugEntry, 0, 64),
	}
}

func (c *Compiler) Bytecode() *Bytecode {
	// run simple peephole optimization before packaging
	c.instructions = peephole(c.instructions)
	return &Bytecode{Magic: BytecodeMagic, Version: version.Version, Instructions: c.instructions, Constants: c.constants, Debug: DebugInfo{Entries: c.debugEntries}}
}

func (c *Compiler) emit(op code.Opcode, operands ...int) int {
	ins := code.Make(op, operands...)
	pos := len(c.instructions)
	c.instructions = append(c.instructions, ins...)
	return pos
}

// emitAt emits an instruction and records a debug entry for the given AST node
func (c *Compiler) emitAt(n ast.Node, op code.Opcode, operands ...int) int {
	pos := c.emit(op, operands...)
	// record debug info if available
	if file, line, col, fn := tokenInfoFromNode(n); file != "" || line > 0 || col > 0 {
		c.debugEntries = append(c.debugEntries, DebugEntry{PC: pos, File: file, Line: line, Column: col, Function: fn})
	}
	return pos
}

func (c *Compiler) addConstant(obj object.Object) int {
	c.constants = append(c.constants, obj)
	return len(c.constants) - 1
}

func foldConstExpr(node ast.Node) (object.Object, bool) {
	switch node := node.(type) {
	case *ast.IntegerLiteral:
		return object.NewInteger(node.Value), true
	case *ast.FloatLiteral:
		return object.NewFloat(node.Value), true
	case *ast.StringLiteral:
		return object.NewString(node.Value), true
	case *ast.Boolean:
		return object.NewBoolean(node.Value), true
	case *ast.PrefixExpression:
		right, ok := foldConstExpr(node.Right)
		if !ok {
			return nil, false
		}
		switch node.Operator {
		case "-":
			if i, ok := right.(*object.Integer); ok {
				return object.NewInteger(-i.Value), true
			}
			if f, ok := right.(*object.Float); ok {
				return object.NewFloat(-f.Value), true
			}
		case "!":
			if b, ok := right.(*object.Boolean); ok {
				return object.NewBoolean(!b.Value), true
			}
		}
	case *ast.InfixExpression:
		left, ok := foldConstExpr(node.Left)
		if !ok {
			return nil, false
		}
		right, ok := foldConstExpr(node.Right)
		if !ok {
			return nil, false
		}
		switch node.Operator {
		case "+":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewInteger(i1.Value + i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewFloat(float64(i1.Value) + f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewFloat(f1.Value + float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewFloat(f1.Value + f2.Value), true
				}
			}
			if s1, ok := left.(*object.String); ok {
				if s2, ok := right.(*object.String); ok {
					return object.NewString(s1.Value + s2.Value), true
				}
			}
		case "-":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewInteger(i1.Value - i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewFloat(float64(i1.Value) - f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewFloat(f1.Value - float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewFloat(f1.Value - f2.Value), true
				}
			}
		case "*":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewInteger(i1.Value * i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewFloat(float64(i1.Value) * f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewFloat(f1.Value * float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewFloat(f1.Value * f2.Value), true
				}
			}
		case "/":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					if i2.Value == 0 {
						return nil, false
					}
					return object.NewInteger(i1.Value / i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					if f2.Value == 0 {
						return nil, false
					}
					return object.NewFloat(float64(i1.Value) / f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					if i2.Value == 0 {
						return nil, false
					}
					return object.NewFloat(f1.Value / float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					if f2.Value == 0 {
						return nil, false
					}
					return object.NewFloat(f1.Value / f2.Value), true
				}
			}
		case "==":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(i1.Value == i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(float64(i1.Value) == f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(f1.Value == float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(f1.Value == f2.Value), true
				}
			}
			if s1, ok := left.(*object.String); ok {
				if s2, ok := right.(*object.String); ok {
					return object.NewBoolean(s1.Value == s2.Value), true
				}
			}
			if b1, ok := left.(*object.Boolean); ok {
				if b2, ok := right.(*object.Boolean); ok {
					return object.NewBoolean(b1.Value == b2.Value), true
				}
			}
		case "!=":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(i1.Value != i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(float64(i1.Value) != f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(f1.Value != float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(f1.Value != f2.Value), true
				}
			}
			if s1, ok := left.(*object.String); ok {
				if s2, ok := right.(*object.String); ok {
					return object.NewBoolean(s1.Value != s2.Value), true
				}
			}
			if b1, ok := left.(*object.Boolean); ok {
				if b2, ok := right.(*object.Boolean); ok {
					return object.NewBoolean(b1.Value != b2.Value), true
				}
			}
		case ">":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(i1.Value > i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(float64(i1.Value) > f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(f1.Value > float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(f1.Value > f2.Value), true
				}
			}
		case "<":
			if i1, ok := left.(*object.Integer); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(i1.Value < i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(float64(i1.Value) < f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					return object.NewBoolean(f1.Value < float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					return object.NewBoolean(f1.Value < f2.Value), true
				}
			}
			if s1, ok := left.(*object.String); ok {
				if s2, ok := right.(*object.String); ok {
					return object.NewBoolean(s1.Value < s2.Value), true
				}
			}
		}
	}
	return nil, false
}

func (c *Compiler) compileStatements(stmts []ast.Statement) error {
	for _, stmt := range stmts {
		if err := c.Compile(stmt); err != nil {
			return err
		}
	}
	return nil
}

func (c *Compiler) compileBlock(block *ast.BlockStatement) error {
	if block == nil {
		return nil
	}
	return c.compileStatements(block.Statements)
}

func (c *Compiler) compileExpressions(exprs []ast.Expression) error {
	for _, expr := range exprs {
		if err := c.Compile(expr); err != nil {
			return err
		}
	}
	return nil
}

func requireArgCount(name string, args []ast.Expression, expected int) error {
	if len(args) != expected {
		return fmt.Errorf("%s: expected %d argument, got %d", name, expected, len(args))
	}
	return nil
}

func (c *Compiler) compileBuiltinCall(node *ast.CallExpression, name string) (bool, error) {
	switch name {
	case "print":
		if err := c.compileExpressions(node.Arguments); err != nil {
			return true, err
		}
		c.emitAt(node, code.OpPrint, len(node.Arguments))
		return true, nil
	case "len":
		if err := requireArgCount("len", node.Arguments, 1); err != nil {
			return true, err
		}
		if err := c.Compile(node.Arguments[0]); err != nil {
			return true, err
		}
		c.emitAt(node, code.OpLen)
		return true, nil
	case "type":
		if err := requireArgCount("type", node.Arguments, 1); err != nil {
			return true, err
		}
		if err := c.Compile(node.Arguments[0]); err != nil {
			return true, err
		}
		c.emitAt(node, code.OpType)
		return true, nil
	}
	return false, nil
}

func (c *Compiler) Compile(node ast.Node) error {
	switch n := node.(type) {
	case *ast.Program:
		return c.compileStatements(n.Statements)
	case *ast.BlockStatement:
		return c.compileStatements(n.Statements)
	case *ast.ExpressionStatement:
		if err := c.Compile(n.Expression); err != nil {
			return err
		}
		c.emitAt(n, code.OpPop)
	case *ast.IntegerLiteral:
		idx := c.addConstant(object.NewInteger(n.Value))
		c.emitAt(n, code.OpConstant, idx)
	case *ast.FloatLiteral:
		idx := c.addConstant(object.NewFloat(n.Value))
		c.emitAt(n, code.OpConstant, idx)
	case *ast.StringLiteral:
		idx := c.addConstant(object.NewString(n.Value))
		c.emitAt(n, code.OpConstant, idx)
	case *ast.Boolean:
		if n.Value {
			c.emitAt(n, code.OpTrue)
		} else {
			c.emitAt(n, code.OpFalse)
		}
	case *ast.Null:
		c.emitAt(n, code.OpNull)
	case *ast.ArrayLiteral:
		if err := c.compileExpressions(n.Elements); err != nil {
			return err
		}
		c.emitAt(n, code.OpArray, len(n.Elements))
	case *ast.IndexExpression:
		if err := c.Compile(n.Left); err != nil {
			return err
		}
		if err := c.Compile(n.Index); err != nil {
			return err
		}
		c.emitAt(n, code.OpIndex)
	case *ast.PrefixExpression:
		// Constant folding
		if obj, ok := foldConstExpr(n); ok {
			idx := c.addConstant(obj)
			c.emitAt(n, code.OpConstant, idx)
			return nil
		}
		if err := c.Compile(n.Right); err != nil {
			return err
		}
		switch n.Operator {
		case "-":
			c.emitAt(n, code.OpMinus)
		case "!":
			c.emitAt(n, code.OpBang)
		default:
			return fmt.Errorf("unsupported prefix operator %s", n.Operator)
		}
	case *ast.InfixExpression:
		// Constant folding for literals
		if obj, ok := foldConstExpr(n); ok {
			idx := c.addConstant(obj)
			c.emitAt(n, code.OpConstant, idx)
			return nil
		}
		// Handle <= and >= by reversing operands if needed
		if n.Operator == "<=" {
			if err := c.Compile(n.Right); err != nil {
				return err
			}
			if err := c.Compile(n.Left); err != nil {
				return err
			}
			c.emitAt(n, code.OpGreaterEqual)
			return nil
		}
		if n.Operator == ">=" {
			if err := c.Compile(n.Left); err != nil {
				return err
			}
			if err := c.Compile(n.Right); err != nil {
				return err
			}
			c.emitAt(n, code.OpGreaterEqual)
			return nil
		}
		if err := c.Compile(n.Left); err != nil {
			return err
		}
		if err := c.Compile(n.Right); err != nil {
			return err
		}
		switch n.Operator {
		case "+":
			c.emitAt(n, code.OpAdd)
		case "-":
			c.emitAt(n, code.OpSub)
		case "*":
			c.emitAt(n, code.OpMul)
		case "/":
			c.emitAt(n, code.OpDiv)
		case "%":
			c.emitAt(n, code.OpMod)
		case "==":
			c.emitAt(n, code.OpEqual)
		case "!=":
			c.emitAt(n, code.OpNotEqual)
		case ">":
			c.emitAt(n, code.OpGreaterThan)
		case "<":
			c.emitAt(n, code.OpLessThan)
		case ">=":
			c.emitAt(n, code.OpGreaterEqual)
		case "<=":
			c.emitAt(n, code.OpLessEqual)
		default:
			return fmt.Errorf("unsupported infix operator %s", n.Operator)
		}
	case *ast.LetStatement:
		if err := c.Compile(n.Value); err != nil {
			return err
		}
		sym := c.symbolTable.Define(n.Name.Value)
		c.emitAt(n, code.OpSetGlobal, sym.Index)
	case *ast.Identifier:
		sym, ok := c.symbolTable.Resolve(n.Value)
		if !ok {
			return fmt.Errorf("undefined variable %s", n.Value)
		}
		c.emitAt(n, code.OpGetGlobal, sym.Index)
	case *ast.AssignStatement:
		switch target := n.Target.(type) {
		case *ast.Identifier:
			if err := c.Compile(n.Value); err != nil {
				return err
			}
			sym, ok := c.symbolTable.Resolve(target.Value)
			if !ok {
				// Implicit define if not defined yet
				sym = c.symbolTable.Define(target.Value)
			}
			c.emitAt(n, code.OpSetGlobal, sym.Index)
		case *ast.IndexExpression:
			if err := c.Compile(target.Left); err != nil {
				return err
			}
			if err := c.Compile(target.Index); err != nil {
				return err
			}
			if err := c.Compile(n.Value); err != nil {
				return err
			}
			c.emitAt(n, code.OpSetIndex)
		default:
			return ErrUnsupportedFeature
		}
	case *ast.IfExpression:
		// condition
		if err := c.Compile(n.Condition); err != nil {
			return err
		}
		// jump if not truthy to else (or end)
		jntPos := c.emitAt(n, code.OpJumpNotTruthy, 9999)
		// consequence
		if err := c.compileBlock(n.Consequence); err != nil {
			return err
		}
		// jump to end
		jmpPos := c.emitAt(n, code.OpJump, 9999)
		// patch jntPos to here
		c.replaceOperand(jntPos, len(c.instructions))
		// alternative
		if n.Alternative != nil {
			if err := c.Compile(n.Alternative); err != nil {
				return err
			}
		}
		// patch jmpPos to end
		c.replaceOperand(jmpPos, len(c.instructions))
		return nil
	case *ast.WhileStatement:
		// while (cond) { body }
		condPos := len(c.instructions)
		if err := c.Compile(n.Condition); err != nil {
			return err
		}
		// jump out if not truthy
		jntPos := c.emitAt(n, code.OpJumpNotTruthy, 9999)
		// body
		if err := c.compileBlock(n.Body); err != nil {
			return err
		}
		// jump back to condition
		c.emitAt(n, code.OpJump, condPos)
		// patch exit to here
		c.replaceOperand(jntPos, len(c.instructions))
		return nil
	case *ast.CallExpression:
		if ident, ok := n.Function.(*ast.Identifier); ok {
			handled, err := c.compileBuiltinCall(n, ident.Value)
			if handled {
				return err
			}
		}
		return ErrUnsupportedFeature
	default:
		return ErrUnsupportedFeature
	}
	return nil
}

func (c *Compiler) replaceOperand(pos int, operand int) {
	op := code.Opcode(c.instructions[pos])
	ins := code.Make(op, operand)
	c.replaceInstruction(pos, ins)
}

func (c *Compiler) replaceInstruction(pos int, newIns []byte) {
	for i := 0; i < len(newIns); i++ {
		c.instructions[pos+i] = newIns[i]
	}
}

// tokenInfoFromNode extracts file/line/column/function name from an AST node
func tokenInfoFromNode(n ast.Node) (string, int, int, string) {
	var t token.Token
	switch node := n.(type) {
	case *ast.Program:
		return "", 0, 0, "<module>"
	case *ast.ExpressionStatement:
		t = node.Token
	case *ast.LetStatement:
		t = node.Token
	case *ast.AssignStatement:
		t = node.Token
	case *ast.ReturnStatement:
		t = node.Token
	case *ast.BlockStatement:
		t = node.Token
	case *ast.StandaloneBlockStatement:
		t = node.Token
	case *ast.WhileStatement:
		t = node.Token
	case *ast.ForStatement:
		t = node.Token
	case *ast.FunctionDeclaration:
		t = node.Token
		if node.Name != nil {
			return t.File, t.Line, t.Column, node.Name.Value
		}
		return t.File, t.Line, t.Column, "<func>"
	case *ast.Identifier:
		t = node.Token
	case *ast.IntegerLiteral:
		t = node.Token
	case *ast.FloatLiteral:
		t = node.Token
	case *ast.StringLiteral:
		t = node.Token
	case *ast.Boolean:
		t = node.Token
	case *ast.Null:
		t = node.Token
	case *ast.PrefixExpression:
		t = node.Token
	case *ast.InfixExpression:
		t = node.Token
	case *ast.IfExpression:
		t = node.Token
	case *ast.FunctionLiteral:
		t = node.Token
		return t.File, t.Line, t.Column, "<lambda>"
	case *ast.CallExpression:
		t = node.Token
	case *ast.ArrayLiteral:
		t = node.Token
	case *ast.MapLiteral:
		t = node.Token
	case *ast.IndexExpression:
		t = node.Token
	default:
		return "", 0, 0, ""
	}
	return t.File, t.Line, t.Column, "<module>"
}

// peephole performs small, local optimizations while preserving instruction stream length
// Important: It never changes the total length of bytecode to keep absolute jump targets valid
func peephole(ins code.Instructions) code.Instructions {
	if len(ins) == 0 {
		return ins
	}
	out := make(code.Instructions, len(ins))
	copy(out, ins)
	for i := 0; i < len(out); {
		op := code.Opcode(out[i])
		switch op {
		case code.OpJump:
			def, _ := code.Lookup(code.OpJump)
			operands, _ := code.ReadOperands(def, out[i+1:])
			target := operands[0]
			// OpJump is 3 bytes; if it jumps to the very next instruction, it's a no-op
			if target == i+3 {
				out[i] = byte(code.OpNop)
				out[i+1] = byte(code.OpNop)
				out[i+2] = byte(code.OpNop)
			}
			i += 3
		case code.OpJumpNotTruthy:
			def, _ := code.Lookup(code.OpJumpNotTruthy)
			operands, _ := code.ReadOperands(def, out[i+1:])
			target := operands[0]
			// Replace with Pop when target is next instruction to preserve stack balance
			if target == i+3 {
				out[i] = byte(code.OpPop)
				out[i+1] = byte(code.OpNop)
				out[i+2] = byte(code.OpNop)
			}
			i += 3
		case code.OpConstant:
			// Pattern: Constant x; Pop -> remove both
			if i+3 < len(out) && code.Opcode(out[i+3]) == code.OpPop {
				out[i] = byte(code.OpNop)
				out[i+1] = byte(code.OpNop)
				out[i+2] = byte(code.OpNop)
				out[i+3] = byte(code.OpNop)
				i += 4
				continue
			}
			i += 3
		default:
			def, ok := code.Lookup(op)
			if !ok {
				// Unknown byte, advance one to avoid infinite loop
				i += 1
				continue
			}
			// advance by opcode byte + operand widths
			i += 1
			for _, w := range def.OperandWidths {
				i += w
			}
		}
	}
	return out
}

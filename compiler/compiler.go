package compiler

import (
	"darix/ast"
	"darix/code"
	"darix/object"
	"fmt"
)

type Compiler struct {
	instructions code.Instructions
	constants    []object.Object
	symbolTable  *SymbolTable
}

func New() *Compiler {
	return &Compiler{
		instructions: code.Instructions{},
		constants:    []object.Object{},
		symbolTable:  NewSymbolTable(),
	}
}

func (c *Compiler) Bytecode() *Bytecode {
	return &Bytecode{Instructions: c.instructions, Constants: c.constants}
}

func (c *Compiler) emit(op code.Opcode, operands ...int) int {
	ins := code.Make(op, operands...)
	pos := len(c.instructions)
	c.instructions = append(c.instructions, ins...)
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
					if i2.Value == 0 { return nil, false }
					return object.NewInteger(i1.Value / i2.Value), true
				}
				if f2, ok := right.(*object.Float); ok {
					if f2.Value == 0 { return nil, false }
					return object.NewFloat(float64(i1.Value) / f2.Value), true
				}
			}
			if f1, ok := left.(*object.Float); ok {
				if i2, ok := right.(*object.Integer); ok {
					if i2.Value == 0 { return nil, false }
					return object.NewFloat(f1.Value / float64(i2.Value)), true
				}
				if f2, ok := right.(*object.Float); ok {
					if f2.Value == 0 { return nil, false }
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

func (c *Compiler) Compile(node ast.Node) error {
	switch n := node.(type) {
	case *ast.Program:
		for _, s := range n.Statements {
			if err := c.Compile(s); err != nil {
				return err
			}
		}
	case *ast.BlockStatement:
		for _, s := range n.Statements {
			if err := c.Compile(s); err != nil {
				return err
			}
		}
		return nil
	case *ast.ExpressionStatement:
		if err := c.Compile(n.Expression); err != nil {
			return err
		}
		c.emit(code.OpPop)
	case *ast.IntegerLiteral:
		idx := c.addConstant(object.NewInteger(n.Value))
		c.emit(code.OpConstant, idx)
	case *ast.FloatLiteral:
		idx := c.addConstant(object.NewFloat(n.Value))
		c.emit(code.OpConstant, idx)
	case *ast.StringLiteral:
		idx := c.addConstant(object.NewString(n.Value))
		c.emit(code.OpConstant, idx)
	case *ast.Boolean:
		if n.Value {
			c.emit(code.OpTrue)
		} else {
			c.emit(code.OpFalse)
		}
	case *ast.Null:
		c.emit(code.OpNull)
	case *ast.PrefixExpression:
		// Constant folding
		if obj, ok := foldConstExpr(n); ok {
			idx := c.addConstant(obj)
			c.emit(code.OpConstant, idx)
			return nil
		}
		if err := c.Compile(n.Right); err != nil {
			return err
		}
		switch n.Operator {
		case "-":
			c.emit(code.OpMinus)
		case "!":
			c.emit(code.OpBang)
		default:
			return fmt.Errorf("unsupported prefix operator %s", n.Operator)
		}
	case *ast.InfixExpression:
		// Constant folding for literals
		if obj, ok := foldConstExpr(n); ok {
			idx := c.addConstant(obj)
			c.emit(code.OpConstant, idx)
			return nil
		}
		// Handle < by reversing operands and using >
		if n.Operator == "<" {
			if err := c.Compile(n.Right); err != nil {
				return err
			}
			if err := c.Compile(n.Left); err != nil {
				return err
			}
			c.emit(code.OpGreaterThan)
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
			c.emit(code.OpAdd)
		case "-":
			c.emit(code.OpSub)
		case "*":
			c.emit(code.OpMul)
		case "/":
			c.emit(code.OpDiv)
		case "==":
			c.emit(code.OpEqual)
		case "!=":
			c.emit(code.OpNotEqual)
		case ">":
			c.emit(code.OpGreaterThan)
		default:
			return fmt.Errorf("unsupported infix operator %s", n.Operator)
		}
	case *ast.LetStatement:
		if err := c.Compile(n.Value); err != nil {
			return err
		}
		sym := c.symbolTable.Define(n.Name.Value)
		c.emit(code.OpSetGlobal, sym.Index)
	case *ast.Identifier:
		sym, ok := c.symbolTable.Resolve(n.Value)
		if !ok {
			return fmt.Errorf("undefined variable %s", n.Value)
		}
		c.emit(code.OpGetGlobal, sym.Index)
	case *ast.AssignStatement:
		// Only identifier targets are supported for now
		ident, ok := n.Target.(*ast.Identifier)
		if !ok {
			return ErrUnsupportedFeature
		}
		if err := c.Compile(n.Value); err != nil {
			return err
		}
		sym, ok := c.symbolTable.Resolve(ident.Value)
		if !ok {
			// Implicit define if not defined yet
			sym = c.symbolTable.Define(ident.Value)
		}
		c.emit(code.OpSetGlobal, sym.Index)
	case *ast.IfExpression:
		// condition
		if err := c.Compile(n.Condition); err != nil {
			return err
		}
		// jump if not truthy to else (or end)
		jntPos := c.emit(code.OpJumpNotTruthy, 9999)
		// consequence
		if err := c.Compile(n.Consequence); err != nil {
			return err
		}
		// jump to end
		jmpPos := c.emit(code.OpJump, 9999)
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
		jntPos := c.emit(code.OpJumpNotTruthy, 9999)
		// body
		if n.Body != nil {
			if err := c.Compile(n.Body); err != nil {
				return err
			}
		}
		// jump back to condition
		c.emit(code.OpJump, condPos)
		// patch exit to here
		c.replaceOperand(jntPos, len(c.instructions))
		return nil
	case *ast.CallExpression:
		// Only support builtin print(...) for now
		if ident, ok := n.Function.(*ast.Identifier); ok && ident.Value == "print" {
			argc := len(n.Arguments)
			for _, a := range n.Arguments {
				if err := c.Compile(a); err != nil {
					return err
				}
			}
			c.emit(code.OpPrint, argc)
			return nil
		}
		return ErrUnsupportedFeature
	default:
		return ErrUnsupportedFeature
	}
	return nil
}

func (c *Compiler) replaceOperand(pos int, operand int) {
	op := code.Opcode(c.instructions[pos])
	def, _ := code.Lookup(op)
	operands, _ := code.ReadOperands(def, c.instructions[pos+1:])
	// write new operand
	ins := code.Make(op, operand)
	c.replaceInstruction(pos, ins)
	_ = operands
}

func (c *Compiler) replaceInstruction(pos int, newIns []byte) {
	for i := 0; i < len(newIns); i++ {
		c.instructions[pos+i] = newIns[i]
	}
}

package interpreter

import (
    "darix/ast"
    "darix/object"
    "strings"
    "fmt"
)

// evalAssertStatement evaluates an assert statement and throws AssertionError if false
func (i *Interpreter) evalAssertStatement(node *ast.AssertStatement, env *object.Environment) object.Object {
    cond := i.eval(node.Condition, env)
    if isError(cond) {
        return cond
    }
    if isTruthy(cond) {
        return object.NULL
    }
    // Build message
    msg := "assertion failed"
    if node.Message != nil {
        val := i.eval(node.Message, env)
        if isError(val) {
            return val
        }
        if s, ok := val.(*object.String); ok {
            msg = s.Value
        } else {
            msg = val.Inspect()
        }
    }
    ex := object.NewException(object.ASSERTION_ERROR, msg)
    return object.NewExceptionSignal(ex)
}

// evalInExpression evaluates membership tests: x in y
func (i *Interpreter) evalInExpression(node *ast.InExpression, env *object.Environment) object.Object {
    left := i.eval(node.Left, env)
    if isError(left) {
        return left
    }
    right := i.eval(node.Right, env)
    if isError(right) {
        return right
    }

    switch container := right.(type) {
    case *object.Array:
        for _, elem := range container.Elements {
            if object.Equals(elem, left) {
                return object.TRUE
            }
        }
        return object.FALSE
    case *object.Map:
        for k := range container.Pairs {
            if object.Equals(k, left) {
                return object.TRUE
            }
        }
        return object.FALSE
    case *object.Hash:
        if hk, ok := left.(object.Hashable); ok {
            if _, ok2 := container.Pairs[hk.HashKey()]; ok2 {
                return object.TRUE
            }
            return object.FALSE
        }
        return object.NewError("unusable as hash key: %s", left.Type())
    case *object.String:
        if s, ok := left.(*object.String); ok {
            return nativeBoolToBooleanObject(strings.Contains(container.Inspect(), s.Value))
        }
        return nativeBoolToBooleanObject(strings.Contains(container.Inspect(), left.Inspect()))
    default:
        return object.NewError("'in' not supported: %s in %s", left.Type(), right.Type())
    }
}

// evalIsExpression performs identity comparison (same object reference)
func (i *Interpreter) evalIsExpression(node *ast.IsExpression, env *object.Environment) object.Object {
    left := i.eval(node.Left, env)
    if isError(left) {
        return left
    }
    right := i.eval(node.Right, env)
    if isError(right) {
        return right
    }
    return nativeBoolToBooleanObject(left == right)
}

// evalGlobalStatement marks variables as global by ensuring they exist in the
// interpreter's root environment. Subsequent assignments will update the
// global variable (since evalAssignStatement uses Update() first).
func (i *Interpreter) evalGlobalStatement(node *ast.GlobalStatement, env *object.Environment) object.Object {
    if node == nil || len(node.Names) == 0 {
        return object.NULL
    }
    // Root/module environment for this interpreter instance
    root := i.env
    for _, ident := range node.Names {
        if ident == nil { continue }
        if _, ok := root.Get(ident.Value); !ok {
            root.Set(ident.Value, object.NULL)
        }
    }
    return object.NULL
}

// evalNonlocalStatement is a relaxed implementation: since we don't have
// exported accessors to walk the Environment.outer chain, we approximate by
// creating a placeholder in the interpreter's root environment if the name is
// not found anywhere. This at least prevents accidental creation of a new local
// variable and allows Update() to find a binding. TODO: enhance Environment API
// to properly resolve the nearest enclosing non-global scope.
func (i *Interpreter) evalNonlocalStatement(node *ast.NonlocalStatement, env *object.Environment) object.Object {
    if node == nil || len(node.Names) == 0 {
        return object.NULL
    }
    // For each name, ensure there is an existing binding in an outer (non-global) scope.
    for _, ident := range node.Names {
        if ident == nil {
            continue
        }
        name := ident.Value
        // Walk outwards to find the nearest scope that already defines the name locally
        found := false
        for e := env.Outer(); e != nil; e = e.Outer() {
            if e.HasLocal(name) {
                found = true
                break
            }
            // Stop before we pass beyond the module/global scope
            if e.Outer() == nil {
                break
            }
        }
        if !found {
            // Raise a runtime exception so try/catch can handle it
            ex := object.NewException(object.RUNTIME_ERROR, fmt.Sprintf("nonlocal: no binding for '%s' found in enclosing scope", name))
            return object.NewExceptionSignal(ex)
        }
    }
    return object.NULL
}

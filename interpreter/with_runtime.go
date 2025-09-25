package interpreter

import (
    "darix/ast"
    "darix/object"
)

// evalWithStatement evaluates 'with' context manager blocks.
// Protocol: the context object must provide __enter__ and __exit__ methods.
// If 'as var' is present, the result of __enter__ is bound to that variable within the with block scope only.
func (i *Interpreter) evalWithStatement(node *ast.WithStatement, env *object.Environment) object.Object {
    // Evaluate context
    ctx := i.eval(node.Context, env)
    if isError(ctx) {
        return ctx
    }

    // New scope for the with-body so bindings do not leak
    blockEnv := object.NewEnclosedEnvironment(env)

    // If variable specified, bind initial context
    if node.Variable != nil {
        blockEnv.Set(node.Variable.Value, ctx)
    }

    // Resolve __enter__ and __exit__ methods
    var enterFunc object.Object
    var exitFunc object.Object

    switch c := ctx.(type) {
    case *object.Instance:
        if m, ok := c.Class.Members["__enter__"]; ok {
            switch f := m.(type) {
            case *object.Function:
                enterFunc = &object.BoundMethod{Self: c, Fn: f}
            case *object.Builtin:
                enterFunc = f
            case *object.BoundMethod:
                enterFunc = f
            }
        }
        if m, ok := c.Class.Members["__exit__"]; ok {
            switch f := m.(type) {
            case *object.Function:
                exitFunc = &object.BoundMethod{Self: c, Fn: f}
            case *object.Builtin:
                exitFunc = f
            case *object.BoundMethod:
                exitFunc = f
            }
        }
    case *object.Class:
        if m, ok := c.Members["__enter__"]; ok {
            switch f := m.(type) {
            case *object.Function:
                enterFunc = f
            case *object.Builtin:
                enterFunc = f
            }
        }
        if m, ok := c.Members["__exit__"]; ok {
            switch f := m.(type) {
            case *object.Function:
                exitFunc = f
            case *object.Builtin:
                exitFunc = f
            }
        }
    }

    // Call __enter__ if present; update binding if 'as var' is used
    if enterFunc != nil {
        entered := i.applyFunction(enterFunc, []object.Object{})
        if isError(entered) {
            return entered
        }
        if ex, isEx := entered.(*object.ExceptionSignal); isEx {
            return ex
        }
        if node.Variable != nil {
            if !blockEnv.Update(node.Variable.Value, entered) {
                blockEnv.Set(node.Variable.Value, entered)
            }
        }
    }

    // Execute body; ensure __exit__ is invoked afterward
    result := i.evalBlockStatementWithScoping(node.Body, blockEnv, false)

    // Determine if body raised an exception
    var bodyException *object.ExceptionSignal
    if exSig, isEx := result.(*object.ExceptionSignal); isEx {
        bodyException = exSig
    }

    // Always execute __exit__ after body, regardless of body outcome
    if exitFunc != nil {
        // Prepare arguments depending on function signature (0 or 3-arg compatibility)
        var args []object.Object
        // Build (exc_type, exc, tb) if exception happened, else (null, null, null)
        var three []object.Object
        if bodyException != nil && bodyException.Exception != nil {
            excType := object.NewString(bodyException.Exception.ExceptionType)
            three = []object.Object{excType, bodyException.Exception, bodyException.Exception.StackTrace}
        } else {
            three = []object.Object{object.NULL, object.NULL, object.NULL}
        }

        // Inspect parameter count when possible
        switch fn := exitFunc.(type) {
        case *object.BoundMethod:
            if fn.Fn != nil && len(fn.Fn.Parameters) == 3 {
                args = three
            } else {
                args = []object.Object{}
            }
        case *object.Function:
            if len(fn.Parameters) == 3 {
                args = three
            } else {
                args = []object.Object{}
            }
        default:
            // Builtins or others: pass no args for compatibility
            args = []object.Object{}
        }

        exitRes := i.applyFunction(exitFunc, args)
        // If __exit__ returns true and there was an exception, suppress it
        if bodyException != nil {
            if b, ok := exitRes.(*object.Boolean); ok && b == object.TRUE {
                return object.NULL
            }
        }
        // Propagate errors thrown inside __exit__
        if ex, isEx := exitRes.(*object.ExceptionSignal); isEx {
            return ex
        }
        if err, isErr := exitRes.(*object.Error); isErr {
            return err
        }
    }

    // If body raised and not suppressed, propagate
    if bodyException != nil {
        return bodyException
    }

    return result
}

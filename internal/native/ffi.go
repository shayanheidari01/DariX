package native

import (
	"darix/object"
	"fmt"
	"reflect"
)

var ffiRegistry = map[string]reflect.Value{}

// RegisterFFI registers a Go function by name for reflective calls.
// Example: RegisterFFI("math.Sqrt", math.Sqrt)
func RegisterFFI(name string, fn any) {
	v := reflect.ValueOf(fn)
	if v.Kind() != reflect.Func {
		panic("RegisterFFI: non-func value")
	}
	ffiRegistry[name] = v
}

func init() {
	Register("ffi", map[string]*object.Builtin{
		"ffi_call": {Fn: ffiCall},
	})
}

func ffiCall(args ...object.Object) object.Object {
	if len(args) < 1 {
		return object.NewError("ffi_call: expected at least 1 argument (name)")
	}
	nameObj, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ffi_call: first argument must be string (function name)")
	}
	if !ModuleAllowed("ffi") {
		return object.NewError("ffi_call: access to native module ffi denied by policy")
	}
	fn, ok := ffiRegistry[nameObj.Value]
	if !ok {
		return object.NewError("ffi_call: function %q not registered", nameObj.Value)
	}

	in, convErr := toGoArgs(fn, args[1:])
	if convErr != nil {
		return object.NewError("ffi_call: %s", convErr)
	}
	var outs []reflect.Value
	var recovered any
	func() {
		defer func() {
			if r := recover(); r != nil {
				recovered = r
			}
		}()
		outs = fn.Call(in)
	}()
	if recovered != nil {
		ex := object.NewException(object.RUNTIME_ERROR, fmt.Sprintf("panic in ffi: %v", recovered))
		return object.NewExceptionSignal(ex)
	}
	// Handle common patterns: (T), (T, error), (error)
	if len(outs) == 0 {
		return object.NULL
	}
	if len(outs) == 1 {
		// single result
		return fromGoValue(outs[0])
	}
	if len(outs) == 2 {
		// if last is error and not nil => error
		if errv := outs[1]; errv.IsValid() && !errv.IsZero() {
			if e, ok := errv.Interface().(error); ok && e != nil {
				return object.NewError("%s", e.Error())
			}
		}
		return fromGoValue(outs[0])
	}
	// multiple return values: return first, ignore others
	return fromGoValue(outs[0])
}

func toGoArgs(fn reflect.Value, args []object.Object) ([]reflect.Value, error) {
	t := fn.Type()
	if t.IsVariadic() {
		// Simplify: require exact or more args; rely on conversion per param
	} else if len(args) != t.NumIn() {
		return nil, fmt.Errorf("wrong number of arguments: expected %d, got %d", t.NumIn(), len(args))
	}
	in := make([]reflect.Value, 0, len(args))
	for i := 0; i < len(args); i++ {
		var pt reflect.Type
		if t.IsVariadic() && i >= t.NumIn()-1 {
			pt = t.In(t.NumIn() - 1).Elem()
		} else {
			pt = t.In(i)
		}
		gv, err := toGoValue(args[i], pt)
		if err != nil {
			return nil, err
		}
		in = append(in, gv)
	}
	return in, nil
}

func toGoValue(o object.Object, want reflect.Type) (reflect.Value, error) {
	switch v := o.(type) {
	case *object.Integer:
		// map to any int kind by conversion
		iv := reflect.ValueOf(v.Value)
		if want.Kind() >= reflect.Int && want.Kind() <= reflect.Int64 {
			return iv.Convert(want), nil
		}
		if want.Kind() >= reflect.Uint && want.Kind() <= reflect.Uint64 {
			if v.Value < 0 {
				return reflect.Value{}, fmt.Errorf("cannot convert negative to unsigned")
			}
			return reflect.ValueOf(uint64(v.Value)).Convert(want), nil
		}
		if want.Kind() == reflect.Float32 || want.Kind() == reflect.Float64 {
			return reflect.ValueOf(float64(v.Value)).Convert(want), nil
		}
		if want.Kind() == reflect.Interface {
			return iv, nil
		}
		return reflect.Value{}, fmt.Errorf("unsupported int->%s conversion", want.String())
	case *object.Float:
		fv := reflect.ValueOf(v.Value)
		if want.Kind() == reflect.Float32 || want.Kind() == reflect.Float64 {
			return fv.Convert(want), nil
		}
		if want.Kind() >= reflect.Int && want.Kind() <= reflect.Int64 {
			return reflect.ValueOf(int64(v.Value)).Convert(want), nil
		}
		if want.Kind() == reflect.Interface {
			return fv, nil
		}
		return reflect.Value{}, fmt.Errorf("unsupported float->%s conversion", want.String())
	case *object.String:
		sv := reflect.ValueOf(v.Value)
		if want.Kind() == reflect.String || want.Kind() == reflect.Interface {
			return sv.Convert(want), nil
		}
		return reflect.Value{}, fmt.Errorf("unsupported string->%s conversion", want.String())
	case *object.Boolean:
		bv := reflect.ValueOf(v.Value)
		if want.Kind() == reflect.Bool || want.Kind() == reflect.Interface {
			return bv.Convert(want), nil
		}
		return reflect.Value{}, fmt.Errorf("unsupported bool->%s conversion", want.String())
	case *object.Array:
		if want.Kind() == reflect.Slice {
			elemT := want.Elem()
			n := len(v.Elements)
			sl := reflect.MakeSlice(want, n, n)
			for i := 0; i < n; i++ {
				gv, err := toGoValue(v.Elements[i], elemT)
				if err != nil {
					return reflect.Value{}, fmt.Errorf("slice elem %d: %w", i, err)
				}
				sl.Index(i).Set(gv)
			}
			return sl, nil
		}
		if want.Kind() == reflect.Interface {
			return reflect.ValueOf(v.Elements), nil
		}
		return reflect.Value{}, fmt.Errorf("unsupported array->%s conversion", want.String())
	case *object.Map:
		if want.Kind() == reflect.Map {
			mk := want.Key()
			mv := want.Elem()
			m := reflect.MakeMapWithSize(want, len(v.Pairs))
			for k, val := range v.Pairs {
				gk, err := toGoValue(k, mk)
				if err != nil {
					return reflect.Value{}, fmt.Errorf("map key: %w", err)
				}
				gv, err := toGoValue(val, mv)
				if err != nil {
					return reflect.Value{}, fmt.Errorf("map value: %w", err)
				}
				m.SetMapIndex(gk, gv)
			}
			return m, nil
		}
		if want.Kind() == reflect.Interface {
			return reflect.ValueOf(v.Pairs), nil
		}
		return reflect.Value{}, fmt.Errorf("unsupported map->%s conversion", want.String())
	case *object.Null:
		if want.Kind() == reflect.Interface || want.Kind() == reflect.Pointer || want.Kind() == reflect.Slice || want.Kind() == reflect.Map || want.Kind() == reflect.Func {
			return reflect.Zero(want), nil
		}
		return reflect.Value{}, fmt.Errorf("cannot convert null to %s", want.String())
	default:
		return reflect.Value{}, fmt.Errorf("unsupported arg type %s", o.Type())
	}
}

func fromGoValue(v reflect.Value) object.Object {
	if !v.IsValid() || (v.Kind() == reflect.Pointer && v.IsNil()) {
		return object.NULL
	}
	// unwrap interfaces
	if v.Kind() == reflect.Interface {
		v = v.Elem()
	}
	switch v.Kind() {
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		return object.NewInteger(v.Int())
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Uintptr:
		return object.NewInteger(int64(v.Uint()))
	case reflect.Float32, reflect.Float64:
		return object.NewFloat(v.Float())
	case reflect.Bool:
		return object.NewBoolean(v.Bool())
	case reflect.String:
		return object.NewString(v.String())
	case reflect.Slice, reflect.Array:
		n := v.Len()
		elems := make([]object.Object, n)
		for i := 0; i < n; i++ {
			elems[i] = fromGoValue(v.Index(i))
		}
		return object.NewArray(elems)
	case reflect.Map:
		pairs := make(map[object.Object]object.Object, v.Len())
		for _, k := range v.MapKeys() {
			keyObj := fromGoValue(k)
			valObj := fromGoValue(v.MapIndex(k))
			pairs[keyObj] = valObj
		}
		return object.NewMap(pairs)
	default:
		return object.NewString(fmt.Sprintf("%v", v.Interface()))
	}
}

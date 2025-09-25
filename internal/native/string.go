package native

import (
	"darix/object"
	"strings"
	"unicode"
)

func init() {
	Register("string", map[string]*object.Builtin{
		"str_upper":      {Fn: strUpper},
		"str_lower":      {Fn: strLower},
		"str_trim":       {Fn: strTrim},
		"str_trim_left":  {Fn: strTrimLeft},
		"str_trim_right": {Fn: strTrimRight},
		"str_split":      {Fn: strSplit},
		"str_join":       {Fn: strJoin},
		"str_replace":    {Fn: strReplace},
		"str_contains":   {Fn: strContains},
		"str_starts":     {Fn: strStarts},
		"str_ends":       {Fn: strEnds},
		"str_index":      {Fn: strIndex},
		"str_last_index": {Fn: strLastIndex},
		"str_repeat":     {Fn: strRepeat},
		"str_reverse":    {Fn: strReverse},
		"str_is_alpha":   {Fn: strIsAlpha},
		"str_is_digit":   {Fn: strIsDigit},
		"str_is_space":   {Fn: strIsSpace},
	})
}

func strUpper(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_upper: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_upper: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_upper: argument must be string, got %s", args[0].Type())
	}
	
	return object.NewString(strings.ToUpper(str.Value))
}

func strLower(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_lower: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_lower: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_lower: argument must be string, got %s", args[0].Type())
	}
	
	return object.NewString(strings.ToLower(str.Value))
}

func strTrim(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_trim: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_trim: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_trim: argument must be string, got %s", args[0].Type())
	}
	
	return object.NewString(strings.TrimSpace(str.Value))
}

func strTrimLeft(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_trim_left: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_trim_left: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	cutset, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_trim_left: arguments must be strings")
	}
	
	return object.NewString(strings.TrimLeft(str.Value, cutset.Value))
}

func strTrimRight(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_trim_right: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_trim_right: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	cutset, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_trim_right: arguments must be strings")
	}
	
	return object.NewString(strings.TrimRight(str.Value, cutset.Value))
}

func strSplit(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_split: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_split: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	sep, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_split: arguments must be strings")
	}
	
	parts := strings.Split(str.Value, sep.Value)
	elements := make([]object.Object, len(parts))
	for i, part := range parts {
		elements[i] = object.NewString(part)
	}
	
	return object.NewArray(elements)
}

func strJoin(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_join: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_join: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	sep, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_join: first argument must be array, second must be string")
	}
	
	parts := make([]string, len(arr.Elements))
	for i, elem := range arr.Elements {
		if str, ok := elem.(*object.String); ok {
			parts[i] = str.Value
		} else {
			parts[i] = elem.Inspect()
		}
	}
	
	return object.NewString(strings.Join(parts, sep.Value))
}

func strReplace(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_replace: access to native module string denied by policy")
	}
	if len(args) != 3 {
		return object.NewError("str_replace: expected 3 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	old, ok2 := args[1].(*object.String)
	new, ok3 := args[2].(*object.String)
	if !ok1 || !ok2 || !ok3 {
		return object.NewError("str_replace: arguments must be strings")
	}
	
	return object.NewString(strings.ReplaceAll(str.Value, old.Value, new.Value))
}

func strContains(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_contains: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_contains: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	substr, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_contains: arguments must be strings")
	}
	
	return object.NewBoolean(strings.Contains(str.Value, substr.Value))
}

func strStarts(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_starts: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_starts: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	prefix, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_starts: arguments must be strings")
	}
	
	return object.NewBoolean(strings.HasPrefix(str.Value, prefix.Value))
}

func strEnds(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_ends: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_ends: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	suffix, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_ends: arguments must be strings")
	}
	
	return object.NewBoolean(strings.HasSuffix(str.Value, suffix.Value))
}

func strIndex(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_index: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_index: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	substr, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_index: arguments must be strings")
	}
	
	index := strings.Index(str.Value, substr.Value)
	return object.NewInteger(int64(index))
}

func strLastIndex(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_last_index: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_last_index: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	substr, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("str_last_index: arguments must be strings")
	}
	
	index := strings.LastIndex(str.Value, substr.Value)
	return object.NewInteger(int64(index))
}

func strRepeat(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_repeat: access to native module string denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("str_repeat: expected 2 arguments, got %d", len(args))
	}
	
	str, ok1 := args[0].(*object.String)
	count, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("str_repeat: first argument must be string, second must be integer")
	}
	
	if count.Value < 0 {
		return object.NewError("str_repeat: count cannot be negative")
	}
	
	return object.NewString(strings.Repeat(str.Value, int(count.Value)))
}

func strReverse(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_reverse: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_reverse: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_reverse: argument must be string, got %s", args[0].Type())
	}
	
	runes := []rune(str.Value)
	for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
		runes[i], runes[j] = runes[j], runes[i]
	}
	
	return object.NewString(string(runes))
}

func strIsAlpha(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_is_alpha: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_is_alpha: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_is_alpha: argument must be string, got %s", args[0].Type())
	}
	
	if len(str.Value) == 0 {
		return object.NewBoolean(false)
	}
	
	for _, r := range str.Value {
		if !unicode.IsLetter(r) {
			return object.NewBoolean(false)
		}
	}
	
	return object.NewBoolean(true)
}

func strIsDigit(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_is_digit: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_is_digit: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_is_digit: argument must be string, got %s", args[0].Type())
	}
	
	if len(str.Value) == 0 {
		return object.NewBoolean(false)
	}
	
	for _, r := range str.Value {
		if !unicode.IsDigit(r) {
			return object.NewBoolean(false)
		}
	}
	
	return object.NewBoolean(true)
}

func strIsSpace(args ...object.Object) object.Object {
	if !ModuleAllowed("string") {
		return object.NewError("str_is_space: access to native module string denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("str_is_space: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("str_is_space: argument must be string, got %s", args[0].Type())
	}
	
	if len(str.Value) == 0 {
		return object.NewBoolean(false)
	}
	
	for _, r := range str.Value {
		if !unicode.IsSpace(r) {
			return object.NewBoolean(false)
		}
	}
	
	return object.NewBoolean(true)
}

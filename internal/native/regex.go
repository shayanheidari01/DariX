package native

import (
	"darix/object"
	"regexp"
)

func init() {
	Register("regex", map[string]*object.Builtin{
		"regex_match":     {Fn: regexMatch},
		"regex_find":      {Fn: regexFind},
		"regex_find_all":  {Fn: regexFindAll},
		"regex_replace":   {Fn: regexReplace},
		"regex_split":     {Fn: regexSplit},
		"regex_compile":   {Fn: regexCompile},
		"regex_test":      {Fn: regexTest},
	})
}

// regexMatch tests if string matches pattern
func regexMatch(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_match: access to native module regex denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("regex_match: expected 2 arguments, got %d", len(args))
	}
	
	pattern, ok1 := args[0].(*object.String)
	text, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("regex_match: arguments must be strings")
	}
	
	matched, err := regexp.MatchString(pattern.Value, text.Value)
	if err != nil {
		return object.NewError("regex_match: %s", err.Error())
	}
	
	return object.NewBoolean(matched)
}

// regexFind finds first match of pattern in string
func regexFind(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_find: access to native module regex denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("regex_find: expected 2 arguments, got %d", len(args))
	}
	
	pattern, ok1 := args[0].(*object.String)
	text, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("regex_find: arguments must be strings")
	}
	
	re, err := regexp.Compile(pattern.Value)
	if err != nil {
		return object.NewError("regex_find: %s", err.Error())
	}
	
	match := re.FindString(text.Value)
	if match == "" {
		return object.NULL
	}
	
	return object.NewString(match)
}

// regexFindAll finds all matches of pattern in string
func regexFindAll(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_find_all: access to native module regex denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("regex_find_all: expected 2-3 arguments, got %d", len(args))
	}
	
	pattern, ok1 := args[0].(*object.String)
	text, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("regex_find_all: first two arguments must be strings")
	}
	
	limit := -1 // Find all by default
	if len(args) == 3 {
		if limitObj, ok := args[2].(*object.Integer); ok {
			limit = int(limitObj.Value)
		} else {
			return object.NewError("regex_find_all: third argument must be integer")
		}
	}
	
	re, err := regexp.Compile(pattern.Value)
	if err != nil {
		return object.NewError("regex_find_all: %s", err.Error())
	}
	
	matches := re.FindAllString(text.Value, limit)
	elements := make([]object.Object, len(matches))
	for i, match := range matches {
		elements[i] = object.NewString(match)
	}
	
	return object.NewArray(elements)
}

// regexReplace replaces matches of pattern with replacement
func regexReplace(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_replace: access to native module regex denied by policy")
	}
	if len(args) != 3 {
		return object.NewError("regex_replace: expected 3 arguments, got %d", len(args))
	}
	
	pattern, ok1 := args[0].(*object.String)
	text, ok2 := args[1].(*object.String)
	replacement, ok3 := args[2].(*object.String)
	if !ok1 || !ok2 || !ok3 {
		return object.NewError("regex_replace: arguments must be strings")
	}
	
	re, err := regexp.Compile(pattern.Value)
	if err != nil {
		return object.NewError("regex_replace: %s", err.Error())
	}
	
	result := re.ReplaceAllString(text.Value, replacement.Value)
	return object.NewString(result)
}

// regexSplit splits string by pattern
func regexSplit(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_split: access to native module regex denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("regex_split: expected 2-3 arguments, got %d", len(args))
	}
	
	pattern, ok1 := args[0].(*object.String)
	text, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("regex_split: first two arguments must be strings")
	}
	
	limit := -1 // Split all by default
	if len(args) == 3 {
		if limitObj, ok := args[2].(*object.Integer); ok {
			limit = int(limitObj.Value)
		} else {
			return object.NewError("regex_split: third argument must be integer")
		}
	}
	
	re, err := regexp.Compile(pattern.Value)
	if err != nil {
		return object.NewError("regex_split: %s", err.Error())
	}
	
	parts := re.Split(text.Value, limit)
	elements := make([]object.Object, len(parts))
	for i, part := range parts {
		elements[i] = object.NewString(part)
	}
	
	return object.NewArray(elements)
}

// regexCompile compiles pattern and returns validation result
func regexCompile(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_compile: access to native module regex denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("regex_compile: expected 1 argument, got %d", len(args))
	}
	
	pattern, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("regex_compile: argument must be string, got %s", args[0].Type())
	}
	
	_, err := regexp.Compile(pattern.Value)
	if err != nil {
		return object.NewError("regex_compile: %s", err.Error())
	}
	
	return object.TRUE
}

// regexTest tests if pattern is valid (alias for compile but returns boolean)
func regexTest(args ...object.Object) object.Object {
	if !ModuleAllowed("regex") {
		return object.NewError("regex_test: access to native module regex denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("regex_test: expected 1 argument, got %d", len(args))
	}
	
	pattern, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("regex_test: argument must be string, got %s", args[0].Type())
	}
	
	_, err := regexp.Compile(pattern.Value)
	return object.NewBoolean(err == nil)
}

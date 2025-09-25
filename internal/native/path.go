package native

import (
	"darix/object"
	"path/filepath"
	"strings"
)

func init() {
	Register("path", map[string]*object.Builtin{
		"path_join":      {Fn: pathJoin},
		"path_split":     {Fn: pathSplit},
		"path_dir":       {Fn: pathDir},
		"path_base":      {Fn: pathBase},
		"path_ext":       {Fn: pathExt},
		"path_clean":     {Fn: pathClean},
		"path_abs":       {Fn: pathAbs},
		"path_rel":       {Fn: pathRel},
		"path_match":     {Fn: pathMatch},
		"path_is_abs":    {Fn: pathIsAbs},
		"path_normalize": {Fn: pathNormalize},
	})
}

// pathJoin joins path elements with appropriate separator
func pathJoin(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_join: access to native module path denied by policy")
	}
	if len(args) < 1 {
		return object.NewError("path_join: expected at least 1 argument, got %d", len(args))
	}
	
	elements := make([]string, len(args))
	for i, arg := range args {
		if str, ok := arg.(*object.String); ok {
			elements[i] = str.Value
		} else {
			return object.NewError("path_join: all arguments must be strings, got %s at position %d", arg.Type(), i)
		}
	}
	
	result := filepath.Join(elements...)
	return object.NewString(result)
}

// pathSplit splits path into directory and file components
func pathSplit(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_split: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_split: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_split: argument must be string, got %s", args[0].Type())
	}
	
	dir, file := filepath.Split(path.Value)
	elements := []object.Object{
		object.NewString(dir),
		object.NewString(file),
	}
	
	return object.NewArray(elements)
}

// pathDir returns directory component of path
func pathDir(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_dir: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_dir: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_dir: argument must be string, got %s", args[0].Type())
	}
	
	dir := filepath.Dir(path.Value)
	return object.NewString(dir)
}

// pathBase returns base component of path
func pathBase(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_base: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_base: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_base: argument must be string, got %s", args[0].Type())
	}
	
	base := filepath.Base(path.Value)
	return object.NewString(base)
}

// pathExt returns file extension
func pathExt(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_ext: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_ext: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_ext: argument must be string, got %s", args[0].Type())
	}
	
	ext := filepath.Ext(path.Value)
	return object.NewString(ext)
}

// pathClean cleans up path
func pathClean(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_clean: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_clean: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_clean: argument must be string, got %s", args[0].Type())
	}
	
	cleaned := filepath.Clean(path.Value)
	return object.NewString(cleaned)
}

// pathAbs returns absolute path
func pathAbs(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_abs: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_abs: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_abs: argument must be string, got %s", args[0].Type())
	}
	
	abs, err := filepath.Abs(path.Value)
	if err != nil {
		return object.NewError("path_abs: %s", err.Error())
	}
	
	return object.NewString(abs)
}

// pathRel returns relative path
func pathRel(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_rel: access to native module path denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("path_rel: expected 2 arguments, got %d", len(args))
	}
	
	basePath, ok1 := args[0].(*object.String)
	targetPath, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("path_rel: arguments must be strings")
	}
	
	rel, err := filepath.Rel(basePath.Value, targetPath.Value)
	if err != nil {
		return object.NewError("path_rel: %s", err.Error())
	}
	
	return object.NewString(rel)
}

// pathMatch matches path against pattern
func pathMatch(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_match: access to native module path denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("path_match: expected 2 arguments, got %d", len(args))
	}
	
	pattern, ok1 := args[0].(*object.String)
	path, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("path_match: arguments must be strings")
	}
	
	matched, err := filepath.Match(pattern.Value, path.Value)
	if err != nil {
		return object.NewError("path_match: %s", err.Error())
	}
	
	return object.NewBoolean(matched)
}

// pathIsAbs checks if path is absolute
func pathIsAbs(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_is_abs: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_is_abs: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_is_abs: argument must be string, got %s", args[0].Type())
	}
	
	isAbs := filepath.IsAbs(path.Value)
	return object.NewBoolean(isAbs)
}

// pathNormalize normalizes path separators
func pathNormalize(args ...object.Object) object.Object {
	if !ModuleAllowed("path") {
		return object.NewError("path_normalize: access to native module path denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("path_normalize: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("path_normalize: argument must be string, got %s", args[0].Type())
	}
	
	// Convert all separators to forward slashes for cross-platform compatibility
	normalized := strings.ReplaceAll(path.Value, "\\", "/")
	normalized = filepath.Clean(normalized)
	
	return object.NewString(normalized)
}

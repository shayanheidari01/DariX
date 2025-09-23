package native

import (
	"darix/object"
	"os"
	"io/ioutil"
)

func init() {
	Register("fs", map[string]*object.Builtin{
		"fs_read": {Fn: fsRead},
		"fs_write": {Fn: fsWrite},
		"fs_exists": {Fn: fsExists},
	})
}

func fsRead(args ...object.Object) object.Object {
	if len(args) != 1 {
		return object.NewError("fs_read: expected 1 argument, got %d", len(args))
	}
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("fs_read: path must be string, got %s", args[0].Type())
	}
	data, err := ioutil.ReadFile(path.Value)
	if err != nil {
		return object.NewError("fs_read: %s", err)
	}
	return object.NewString(string(data))
}

func fsWrite(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("fs_write: expected 2 arguments, got %d", len(args))
	}
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("fs_write: path must be string, got %s", args[0].Type())
	}
	data, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("fs_write: data must be string, got %s", args[1].Type())
	}
	if err := ioutil.WriteFile(path.Value, []byte(data.Value), 0644); err != nil {
		return object.NewError("fs_write: %s", err)
	}
	return object.TRUE
}

func fsExists(args ...object.Object) object.Object {
	if len(args) != 1 {
		return object.NewError("fs_exists: expected 1 argument, got %d", len(args))
	}
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("fs_exists: path must be string, got %s", args[0].Type())
	}
	_, err := os.Stat(path.Value)
	if err == nil {
		return object.TRUE
	}
	if os.IsNotExist(err) {
		return object.FALSE
	}
	return object.NewError("fs_exists: %s", err)
}

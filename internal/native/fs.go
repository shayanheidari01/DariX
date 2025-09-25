package native

import (
	"darix/object"
	"os"
)

func init() {
	Register("fs", map[string]*object.Builtin{
		"fs_read":   {Fn: fsRead},
		"fs_write":  {Fn: fsWrite},
		"fs_exists": {Fn: fsExists},
	})
}

func requireArgs(name string, args []object.Object, expected int) object.Object {
	if len(args) != expected {
		return object.NewError("%s: expected %d argument, got %d", name, expected, len(args))
	}
	return nil
}

func requireStringArg(name, label string, value object.Object) (*object.String, object.Object) {
	str, ok := value.(*object.String)
	if !ok {
		return nil, object.NewError("%s: %s must be string, got %s", name, label, value.Type())
	}
	return str, nil
}

func ensureModuleAllowed(name, module string) object.Object {
	if !ModuleAllowed(module) {
		return object.NewError("%s: access to native module %s denied by policy", name, module)
	}
	return nil
}

func ensureWritable(name string) object.Object {
	if GetPolicy().FSReadOnly {
		return object.NewError("%s: filesystem is read-only by policy", name)
	}
	return nil
}

func resolveFSPath(name, path string) (string, object.Object) {
	abs, ok := sanitizePath(path)
	if !ok {
		return "", object.NewError("%s: access outside allowed root", name)
	}
	return abs, nil
}

func fsRead(args ...object.Object) object.Object {
	const name = "fs_read"
	if errObj := requireArgs(name, args, 1); errObj != nil {
		return errObj
	}
	path, errObj := requireStringArg(name, "path", args[0])
	if errObj != nil {
		return errObj
	}
	if errObj := ensureModuleAllowed(name, "fs"); errObj != nil {
		return errObj
	}
	abs, errObj := resolveFSPath(name, path.Value)
	if errObj != nil {
		return errObj
	}
	data, err := os.ReadFile(abs)
	if err != nil {
		return object.NewError("%s: %s", name, err)
	}
	return object.NewString(string(data))
}

func fsWrite(args ...object.Object) object.Object {
	const name = "fs_write"
	if errObj := requireArgs(name, args, 2); errObj != nil {
		return errObj
	}
	path, errObj := requireStringArg(name, "path", args[0])
	if errObj != nil {
		return errObj
	}
	data, errObj := requireStringArg(name, "data", args[1])
	if errObj != nil {
		return errObj
	}
	if errObj := ensureModuleAllowed(name, "fs"); errObj != nil {
		return errObj
	}
	if errObj := ensureWritable(name); errObj != nil {
		return errObj
	}
	abs, errObj := resolveFSPath(name, path.Value)
	if errObj != nil {
		return errObj
	}
	if err := os.WriteFile(abs, []byte(data.Value), 0o644); err != nil {
		return object.NewError("%s: %s", name, err)
	}
	return object.TRUE
}

func fsExists(args ...object.Object) object.Object {
	const name = "fs_exists"
	if errObj := requireArgs(name, args, 1); errObj != nil {
		return errObj
	}
	path, errObj := requireStringArg(name, "path", args[0])
	if errObj != nil {
		return errObj
	}
	if errObj := ensureModuleAllowed(name, "fs"); errObj != nil {
		return errObj
	}
	abs, errObj := resolveFSPath(name, path.Value)
	if errObj != nil {
		return errObj
	}
	_, err := os.Stat(abs)
	if err == nil {
		return object.TRUE
	}
	if os.IsNotExist(err) {
		return object.FALSE
	}
	return object.NewError("%s: %s", name, err)
}

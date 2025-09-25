package native

import (
	"darix/object"
	"os"
	"os/exec"
	"runtime"
	"strings"
)

func init() {
	Register("os", map[string]*object.Builtin{
		"os_getenv":    {Fn: osGetenv},
		"os_setenv":    {Fn: osSetenv},
		"os_unsetenv":  {Fn: osUnsetenv},
		"os_getcwd":    {Fn: osGetcwd},
		"os_chdir":     {Fn: osChdir},
		"os_mkdir":     {Fn: osMkdir},
		"os_rmdir":     {Fn: osRmdir},
		"os_remove":    {Fn: osRemove},
		"os_rename":    {Fn: osRename},
		"os_exec":      {Fn: osExec},
		"os_platform":  {Fn: osPlatform},
		"os_arch":      {Fn: osArch},
		"os_hostname":  {Fn: osHostname},
		"os_getpid":    {Fn: osGetpid},
		"os_exit":      {Fn: osExit},
	})
}

// osGetenv gets environment variable
func osGetenv(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_getenv: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_getenv: expected 1 argument, got %d", len(args))
	}
	
	key, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_getenv: argument must be string, got %s", args[0].Type())
	}
	
	value := os.Getenv(key.Value)
	return object.NewString(value)
}

// osSetenv sets environment variable
func osSetenv(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_setenv: access to native module os denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("os_setenv: expected 2 arguments, got %d", len(args))
	}
	
	key, ok1 := args[0].(*object.String)
	value, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("os_setenv: arguments must be strings")
	}
	
	if err := os.Setenv(key.Value, value.Value); err != nil {
		return object.NewError("os_setenv: %s", err.Error())
	}
	
	return object.TRUE
}

// osUnsetenv unsets environment variable
func osUnsetenv(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_unsetenv: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_unsetenv: expected 1 argument, got %d", len(args))
	}
	
	key, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_unsetenv: argument must be string, got %s", args[0].Type())
	}
	
	if err := os.Unsetenv(key.Value); err != nil {
		return object.NewError("os_unsetenv: %s", err.Error())
	}
	
	return object.TRUE
}

// osGetcwd gets current working directory
func osGetcwd(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_getcwd: access to native module os denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("os_getcwd: expected 0 arguments, got %d", len(args))
	}
	
	cwd, err := os.Getwd()
	if err != nil {
		return object.NewError("os_getcwd: %s", err.Error())
	}
	
	return object.NewString(cwd)
}

// osChdir changes current working directory
func osChdir(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_chdir: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_chdir: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_chdir: argument must be string, got %s", args[0].Type())
	}
	
	// Check filesystem access policy
	abs, allowed := sanitizePath(path.Value)
	if !allowed {
		return object.NewError("os_chdir: access outside allowed root")
	}
	
	if err := os.Chdir(abs); err != nil {
		return object.NewError("os_chdir: %s", err.Error())
	}
	
	return object.TRUE
}

// osMkdir creates directory
func osMkdir(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_mkdir: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_mkdir: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_mkdir: argument must be string, got %s", args[0].Type())
	}
	
	// Check filesystem write policy
	if GetPolicy().FSReadOnly {
		return object.NewError("os_mkdir: filesystem is read-only by policy")
	}
	
	// Check filesystem access policy
	abs, allowed := sanitizePath(path.Value)
	if !allowed {
		return object.NewError("os_mkdir: access outside allowed root")
	}
	
	if err := os.MkdirAll(abs, 0755); err != nil {
		return object.NewError("os_mkdir: %s", err.Error())
	}
	
	return object.TRUE
}

// osRmdir removes directory
func osRmdir(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_rmdir: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_rmdir: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_rmdir: argument must be string, got %s", args[0].Type())
	}
	
	// Check filesystem write policy
	if GetPolicy().FSReadOnly {
		return object.NewError("os_rmdir: filesystem is read-only by policy")
	}
	
	// Check filesystem access policy
	abs, allowed := sanitizePath(path.Value)
	if !allowed {
		return object.NewError("os_rmdir: access outside allowed root")
	}
	
	if err := os.Remove(abs); err != nil {
		return object.NewError("os_rmdir: %s", err.Error())
	}
	
	return object.TRUE
}

// osRemove removes file or directory
func osRemove(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_remove: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_remove: expected 1 argument, got %d", len(args))
	}
	
	path, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_remove: argument must be string, got %s", args[0].Type())
	}
	
	// Check filesystem write policy
	if GetPolicy().FSReadOnly {
		return object.NewError("os_remove: filesystem is read-only by policy")
	}
	
	// Check filesystem access policy
	abs, allowed := sanitizePath(path.Value)
	if !allowed {
		return object.NewError("os_remove: access outside allowed root")
	}
	
	if err := os.RemoveAll(abs); err != nil {
		return object.NewError("os_remove: %s", err.Error())
	}
	
	return object.TRUE
}

// osRename renames/moves file or directory
func osRename(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_rename: access to native module os denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("os_rename: expected 2 arguments, got %d", len(args))
	}
	
	oldPath, ok1 := args[0].(*object.String)
	newPath, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("os_rename: arguments must be strings")
	}
	
	// Check filesystem write policy
	if GetPolicy().FSReadOnly {
		return object.NewError("os_rename: filesystem is read-only by policy")
	}
	
	// Check filesystem access policy for both paths
	oldAbs, oldAllowed := sanitizePath(oldPath.Value)
	newAbs, newAllowed := sanitizePath(newPath.Value)
	if !oldAllowed || !newAllowed {
		return object.NewError("os_rename: access outside allowed root")
	}
	
	if err := os.Rename(oldAbs, newAbs); err != nil {
		return object.NewError("os_rename: %s", err.Error())
	}
	
	return object.TRUE
}

// osExec executes system command
func osExec(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_exec: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_exec: expected 1 argument, got %d", len(args))
	}
	
	command, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("os_exec: argument must be string, got %s", args[0].Type())
	}
	
	// Split command into parts
	parts := strings.Fields(command.Value)
	if len(parts) == 0 {
		return object.NewError("os_exec: empty command")
	}
	
	var cmd *exec.Cmd
	if len(parts) == 1 {
		cmd = exec.Command(parts[0])
	} else {
		cmd = exec.Command(parts[0], parts[1:]...)
	}
	
	output, err := cmd.CombinedOutput()
	if err != nil {
		return object.NewError("os_exec: %s", err.Error())
	}
	
	return object.NewString(string(output))
}

// osPlatform returns operating system platform
func osPlatform(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_platform: access to native module os denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("os_platform: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewString(runtime.GOOS)
}

// osArch returns system architecture
func osArch(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_arch: access to native module os denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("os_arch: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewString(runtime.GOARCH)
}

// osHostname returns system hostname
func osHostname(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_hostname: access to native module os denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("os_hostname: expected 0 arguments, got %d", len(args))
	}
	
	hostname, err := os.Hostname()
	if err != nil {
		return object.NewError("os_hostname: %s", err.Error())
	}
	
	return object.NewString(hostname)
}

// osGetpid returns process ID
func osGetpid(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_getpid: access to native module os denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("os_getpid: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewInteger(int64(os.Getpid()))
}

// osExit exits the program
func osExit(args ...object.Object) object.Object {
	if !ModuleAllowed("os") {
		return object.NewError("os_exit: access to native module os denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("os_exit: expected 1 argument, got %d", len(args))
	}
	
	code, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("os_exit: argument must be integer, got %s", args[0].Type())
	}
	
	os.Exit(int(code.Value))
	return object.NULL // Never reached
}

package native

import "darix/object"

type Module struct {
    Name      string
    Functions map[string]*object.Builtin
}

var registry = map[string]*Module{}

// Register adds a native module to the global registry.
func Register(name string, funcs map[string]*object.Builtin) {
    registry[name] = &Module{Name: name, Functions: funcs}
}

// Get retrieves a native module by name.
func Get(name string) (*Module, bool) {
    m, ok := registry[name]
    return m, ok
}

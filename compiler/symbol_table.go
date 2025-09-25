package compiler

type SymbolScope string

const (
	GlobalScope SymbolScope = "GLOBAL"
	LocalScope  SymbolScope = "LOCAL"
)

type Symbol struct {
	Name  string
	Scope SymbolScope
	Index int
}

type SymbolTable struct {
	store          map[string]Symbol
	numDefinitions int
	Outer          *SymbolTable
}

func NewSymbolTable() *SymbolTable {
	return &SymbolTable{store: make(map[string]Symbol)}
}

func NewEnclosedSymbolTable(outer *SymbolTable) *SymbolTable {
	st := NewSymbolTable()
	st.Outer = outer
	return st
}

func (st *SymbolTable) Define(name string) Symbol {
	scope := GlobalScope
	if st.Outer != nil {
		scope = LocalScope
	}
	s := Symbol{Name: name, Scope: scope, Index: st.numDefinitions}
	st.store[name] = s
	st.numDefinitions++
	return s
}

func (st *SymbolTable) Resolve(name string) (Symbol, bool) {
	if s, ok := st.store[name]; ok {
		return s, true
	}
	if st.Outer != nil {
		return st.Outer.Resolve(name)
	}
	return Symbol{}, false
}

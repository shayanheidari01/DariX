package native

import (
	"path/filepath"
	"runtime"
	"strings"
)

// CapabilityPolicy controls access to native modules and host resources.
// If AllowAllNative is true, all native modules are importable unless explicitly denied
// by AllowGoModules map (when non-nil, presence of key=false means deny).
// If AllowAllNative is false, only modules present with value=true in AllowGoModules are allowed.
// FSRoot, when non-empty, restricts filesystem access to the subtree under FSRoot.
// FSReadOnly controls whether write operations are permitted.
// InjectToGlobal controls whether native module functions are injected into the global env
// (legacy behavior). In sandboxed mode this should typically be false.
//
// NOTE: This policy is stored globally for simplicity. If you need per-VM/per-interpreter
// policies, wire a setter before executing user code.

type CapabilityPolicy struct {
	AllowAllNative bool
	AllowGoModules map[string]bool // e.g., {"fs":true, "ffi":true}

	FSRoot      string
	FSReadOnly  bool
	InjectToGlobal bool
}

func DefaultAllowAll() *CapabilityPolicy {
	return &CapabilityPolicy{
		AllowAllNative: true,
		AllowGoModules: nil,
		FSRoot:         "",
		FSReadOnly:     false,
		InjectToGlobal: true,
	}
}

var currentPolicy = DefaultAllowAll()

func SetPolicy(p *CapabilityPolicy) {
	if p == nil {
		p = DefaultAllowAll()
	}
	currentPolicy = p
}

func GetPolicy() *CapabilityPolicy { return currentPolicy }

// ModuleAllowed returns whether a native module (e.g., "fs", "ffi") is permitted.
func ModuleAllowed(name string) bool {
	p := currentPolicy
	if p.AllowAllNative {
		// If map is nil -> all allowed; if present and name explicitly false -> deny
		if p.AllowGoModules == nil {
			return true
		}
		if allowed, ok := p.AllowGoModules[name]; ok {
			return allowed
		}
		return true
	}
	// Allow only modules explicitly enabled
	if p.AllowGoModules == nil { return false }
	return p.AllowGoModules[name]
}

// sanitizePath ensures the given path is within FSRoot when set.
// Returns cleaned absolute path and whether it is within the allowed root.
func sanitizePath(path string) (string, bool) {
	p := currentPolicy
	clean := filepath.Clean(path)
	abs := clean
	if !filepath.IsAbs(abs) {
		// Work around Windows drive issues: make it absolute using current working dir
		if a, err := filepath.Abs(abs); err == nil { abs = a }
	}
	if p.FSRoot == "" {
		return abs, true
	}
	root := p.FSRoot
	rootAbs := filepath.Clean(root)
	if !filepath.IsAbs(rootAbs) {
		if ra, err := filepath.Abs(rootAbs); err == nil { rootAbs = ra }
	}
	if runtime.GOOS == "windows" {
		// Case-insensitive prefix check for Windows
		absLower := strings.ToLower(abs)
		rootLower := strings.ToLower(rootAbs)
		if absLower == rootLower || strings.HasPrefix(absLower, rootLower+string(filepath.Separator)) {
			return abs, true
		}
		return abs, false
	}
	if abs == rootAbs || strings.HasPrefix(abs, rootAbs+string(filepath.Separator)) {
		return abs, true
	}
	return abs, false
}

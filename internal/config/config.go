package config

import (
    "fmt"
    "os"
    "strconv"
    "strings"
)

// Config captures global CLI and runtime settings that need to be shared
// between the interpreter, VM, REPL, and tooling. The struct is intentionally
// small; modules with specialized configuration should embed or extend it.
type Config struct {
    Backend       string
    CPUInstructionBudget int
    FSRoot        string
    FSReadOnly    bool
    AllowModules  []string
    DenyModules   []string
    InjectToGlobal bool
}

// Defaults returns a Config populated with safe default values.
func Defaults() Config {
    return Config{
        Backend:       "auto",
        CPUInstructionBudget: 0,
        FSRoot:        "",
        FSReadOnly:    false,
        AllowModules:  nil,
        DenyModules:   nil,
        InjectToGlobal: true,
    }
}

// Loader supports layered configuration: start with defaults, apply env vars,
// then apply CLI flags. It deliberately avoids external dependencies to keep
// binary size low.
type Loader struct {
    base Config
}

// NewLoader creates a loader seeded with Defaults().
func NewLoader() Loader {
    return Loader{base: Defaults()}
}

// WithBase allows callers to seed the loader with a custom base config.
func (l Loader) WithBase(cfg Config) Loader {
    l.base = cfg
    return l
}

// FromEnv overlays known environment variables, returning the merged config.
func (l Loader) FromEnv() Config {
    cfg := l.base

    if v := os.Getenv("DARIX_BACKEND"); v != "" {
        cfg.Backend = v
    }
    if v := os.Getenv("DARIX_CPU_BUDGET"); v != "" {
        if n, err := strconv.Atoi(v); err == nil && n >= 0 {
            cfg.CPUInstructionBudget = n
        }
    }
    if v := os.Getenv("DARIX_FS_ROOT"); v != "" {
        cfg.FSRoot = v
    }
    if v := os.Getenv("DARIX_FS_RO"); v != "" {
        cfg.FSReadOnly = parseBool(v, cfg.FSReadOnly)
    }
    if v := os.Getenv("DARIX_ALLOW_MODULES"); v != "" {
        cfg.AllowModules = splitCSV(v)
    }
    if v := os.Getenv("DARIX_DENY_MODULES"); v != "" {
        cfg.DenyModules = splitCSV(v)
    }
    if v := os.Getenv("DARIX_INJECT_TO_GLOBAL"); v != "" {
        cfg.InjectToGlobal = parseBool(v, cfg.InjectToGlobal)
    }

    return cfg
}

// MergeCLI applies CLI flags on top of an existing config. Callers provide
// values as option functions, keeping flag parsing decoupled from this package.
type Option func(*Config)

// Apply merges the supplied options onto the loader's base config.
func (l Loader) Apply(opts ...Option) Config {
    cfg := l.base
    for _, opt := range opts {
        opt(&cfg)
    }
    return cfg
}

// Helpers -----------------------------------------------------------------

func parseBool(input string, fallback bool) bool {
    switch strings.ToLower(strings.TrimSpace(input)) {
    case "1", "true", "yes", "on":
        return true
    case "0", "false", "no", "off":
        return false
    default:
        return fallback
    }
}

func splitCSV(input string) []string {
    if input == "" {
        return nil
    }
    parts := strings.Split(input, ",")
    result := make([]string, 0, len(parts))
    for _, part := range parts {
        if trimmed := strings.TrimSpace(part); trimmed != "" {
            result = append(result, trimmed)
        }
    }
    return result
}

// CLI Options --------------------------------------------------------------

// WithBackend sets the preferred execution backend (auto|vm|interp).
func WithBackend(backend string) Option {
    return func(cfg *Config) {
        if backend != "" {
            cfg.Backend = backend
        }
    }
}

// WithCPUInstructionBudget sets the VM instruction budget.
func WithCPUInstructionBudget(n int) Option {
    return func(cfg *Config) {
        if n >= 0 {
            cfg.CPUInstructionBudget = n
        }
    }
}

// WithFSRoot sets the virtual filesystem root.
func WithFSRoot(root string) Option {
    return func(cfg *Config) {
        cfg.FSRoot = root
    }
}

// WithFSReadOnly toggles read-only mode for the virtual FS.
func WithFSReadOnly(ro bool) Option {
    return func(cfg *Config) {
        cfg.FSReadOnly = ro
    }
}

// WithModulePolicy applies allow/deny module lists.
func WithModulePolicy(allow, deny []string) Option {
    return func(cfg *Config) {
        cfg.AllowModules = append([]string(nil), allow...)
        cfg.DenyModules = append([]string(nil), deny...)
    }
}

// WithInjectToGlobal controls whether native modules are injected globally.
func WithInjectToGlobal(flag bool) Option {
    return func(cfg *Config) {
        cfg.InjectToGlobal = flag
    }
}

// Validate ensures the configuration is coherent before use.
func (c Config) Validate() error {
    switch c.Backend {
    case "auto", "vm", "interp":
    default:
        return fmt.Errorf("invalid backend: %s", c.Backend)
    }
    if c.CPUInstructionBudget < 0 {
        return fmt.Errorf("negative CPU instruction budget: %d", c.CPUInstructionBudget)
    }
    return nil
}

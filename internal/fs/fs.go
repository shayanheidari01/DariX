package fs

import (
    "errors"
    "io"
    "os"
    "path/filepath"
    "strings"

    "darix/internal/native"
)

var (
    // ErrAccessDenied is returned when the requested operation exceeds the
    // configured capability policy.
    ErrAccessDenied = errors.New("fs: access denied by capability policy")
    // ErrReadOnly is returned when attempting to modify a read-only filesystem.
    ErrReadOnly = errors.New("fs: write attempted on read-only filesystem")
)

// Service provides capability-aware filesystem access. Callers should obtain a
// Service via NewService and reuse it for the lifetime of their execution
// context.
type Service struct {
    policy *native.CapabilityPolicy
}

// NewService creates a new Service bound to the supplied capability policy. A
// nil policy defaults to the permissive configuration.
func NewService(policy *native.CapabilityPolicy) *Service {
    if policy == nil {
        policy = native.DefaultAllowAll()
    }
    return &Service{policy: policy}
}

// ReadFile reads the content of the given path after verifying capability
// restrictions.
func (s *Service) ReadFile(path string) ([]byte, error) {
    full, err := s.resolve(path)
    if err != nil {
        return nil, err
    }
    file, err := os.Open(full)
    if err != nil {
        return nil, err
    }
    defer file.Close()
    return io.ReadAll(file)
}

// WriteFile writes data to the specified path. It obeys read-only policy flags
// and validates that the destination remains within the sandbox root.
func (s *Service) WriteFile(path string, data []byte, perm os.FileMode) error {
    if s.policy.FSReadOnly {
        return ErrReadOnly
    }
    full, err := s.resolve(path)
    if err != nil {
        return err
    }
    return os.WriteFile(full, data, perm)
}

// Open exposes a lower-level API analogous to os.OpenFile while guarding policy
// constraints.
func (s *Service) Open(path string, flag int, perm os.FileMode) (*os.File, error) {
    if s.policy.FSReadOnly && (flag&(os.O_WRONLY|os.O_RDWR|os.O_APPEND|os.O_TRUNC|os.O_CREATE) != 0) {
        return nil, ErrReadOnly
    }
    full, err := s.resolve(path)
    if err != nil {
        return nil, err
    }
    return os.OpenFile(full, flag, perm)
}

// resolve expands the provided path against the capability policy root.
func (s *Service) resolve(path string) (string, error) {
    cleaned := filepath.Clean(path)
    if filepath.IsAbs(cleaned) {
        cleaned = cleaned[1:]
    }

    root := strings.TrimSpace(s.policy.FSRoot)
    if root == "" {
        return cleaned, nil
    }

    full := filepath.Join(root, cleaned)
    if !strings.HasPrefix(full, root) {
        return "", ErrAccessDenied
    }
    return full, nil
}

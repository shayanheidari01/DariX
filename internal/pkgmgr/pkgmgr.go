package pkgmgr

import (
    "archive/zip"
    "errors"
    "fmt"
    "io"
    "net/http"
    urlpkg "net/url"
    "os"
    "path/filepath"
    "runtime"
    "strings"
)

// ModulesRoot determines the directory where DariX modules are stored.
// Order of preference:
// 1) Environment variable DARIX_MODULES
// 2) User config dir (e.g., %AppData%/darix/modules on Windows, ~/.config/darix/modules on Linux)
// 3) Fallback to .darix_modules in current working directory
func ModulesRoot() string {
    if env := strings.TrimSpace(os.Getenv("DARIX_MODULES")); env != "" {
        return env
    }
    if dir, err := os.UserConfigDir(); err == nil && dir != "" {
        return filepath.Join(dir, "darix", "modules")
    }
    // Fallback
    return ".darix_modules"
}

// Install is a convenience wrapper that accepts either
//  - GitHub spec: "owner/repo" or "owner/repo@ref"
//  - GitHub URL: "https://github.com/owner/repo(.git)" or "https://github.com/owner/repo/tree/<ref>"
// It normalizes the input and calls the standard installer.
func Install(specOrURL string) (string, error) {
    s := strings.TrimSpace(specOrURL)
    if s == "" {
        return "", errors.New("empty spec")
    }
    if strings.HasPrefix(s, "http://") || strings.HasPrefix(s, "https://") {
        if owner, repo, ref, err := parseGitHubURL(s); err == nil {
            return InstallGitHub(owner + "/" + repo + "@" + ref)
        } else {
            return "", err
        }
    }
    return InstallGitHub(s)
}

// parseGitHubURL parses common GitHub URL formats and extracts owner, repo, and ref.
// Supported forms:
//   https://github.com/owner/repo
//   https://github.com/owner/repo.git
//   https://github.com/owner/repo/tree/<ref>
func parseGitHubURL(raw string) (owner, repo, ref string, err error) {
    u, err := urlpkg.Parse(raw)
    if err != nil {
        return "", "", "", err
    }
    if !strings.EqualFold(u.Host, "github.com") {
        return "", "", "", fmt.Errorf("unsupported host: %s", u.Host)
    }
    // split and clean path segments
    p := strings.TrimPrefix(u.Path, "/")
    parts := strings.Split(p, "/")
    if len(parts) < 2 {
        return "", "", "", fmt.Errorf("invalid GitHub URL path: %s", u.Path)
    }
    owner = parts[0]
    repo = parts[1]
    // strip optional .git suffix on repo
    repo = strings.TrimSuffix(repo, ".git")
    // detect /tree/<ref>
    if len(parts) >= 4 && parts[2] == "tree" {
        ref = parts[3]
    }
    if ref == "" {
        ref = "main"
    }
    return owner, repo, ref, nil
}

// ensureDir makes sure a directory exists.
func ensureDir(dir string) error {
    if err := os.MkdirAll(dir, 0o755); err != nil {
        return err
    }
    return nil
}

// InstallGitHub downloads a GitHub repository as a zip archive and extracts it
// into the modules directory. The spec format is "owner/repo" or "owner/repo@ref".
// Returns the installed module path inside ModulesRoot.
func InstallGitHub(spec string) (string, error) {
    owner, repo, ref, err := parseGitHubSpec(spec)
    if err != nil {
        return "", err
    }

    root := ModulesRoot()
    if err := ensureDir(root); err != nil {
        return "", err
    }

    // Target path: <root>/<owner>/<repo>
    target := filepath.Join(root, owner, repo)
    if err := ensureDir(filepath.Dir(target)); err != nil {
        return "", err
    }

    // Download archive
    url := fmt.Sprintf("https://codeload.github.com/%s/%s/zip/%s", owner, repo, ref)
    tmpZip, err := os.CreateTemp("", "darix-gh-*.zip")
    if err != nil {
        return "", err
    }
    defer func() { _ = os.Remove(tmpZip.Name()) }()

    if err := httpDownload(url, tmpZip); err != nil {
        return "", fmt.Errorf("download failed: %w", err)
    }

    if _, err := tmpZip.Seek(0, io.SeekStart); err != nil {
        return "", err
    }

    // Extract, stripping the leading top-level folder (repo-ref/)
    if err := unzipStripTopDir(tmpZip.Name(), target); err != nil {
        return "", fmt.Errorf("extract failed: %w", err)
    }

    return target, nil
}

func parseGitHubSpec(spec string) (owner, repo, ref string, err error) {
    if spec == "" {
        return "", "", "", errors.New("empty spec")
    }
    at := strings.SplitN(spec, "@", 2)
    path := at[0]
    if len(at) == 2 {
        ref = strings.TrimSpace(at[1])
    }
    if ref == "" {
        // Choose a reasonable default branch name
        if runtime.GOOS == "windows" {
            ref = "main" // still fine
        } else {
            ref = "main"
        }
    }
    parts := strings.Split(path, "/")
    if len(parts) != 2 {
        return "", "", "", fmt.Errorf("invalid GitHub spec %q, expected owner/repo or owner/repo@ref", spec)
    }
    owner = strings.TrimSpace(parts[0])
    repo = strings.TrimSpace(parts[1])
    if owner == "" || repo == "" {
        return "", "", "", fmt.Errorf("invalid GitHub spec %q", spec)
    }
    return owner, repo, ref, nil
}

func httpDownload(url string, w io.Writer) error {
    resp, err := http.Get(url)
    if err != nil {
        return err
    }
    defer resp.Body.Close()
    if resp.StatusCode < 200 || resp.StatusCode >= 300 {
        return fmt.Errorf("unexpected HTTP status %d", resp.StatusCode)
    }

    _, err = io.Copy(w, resp.Body)
    return err
}

// unzipStripTopDir extracts zip file contents into dest, removing the leading
// directory component usually present in GitHub archives (repo-ref/...).
func unzipStripTopDir(zipPath, dest string) error {
    r, err := zip.OpenReader(zipPath)
    if err != nil {
        return err
    }
    defer r.Close()

    if err := ensureDir(dest); err != nil {
        return err
    }

    // Detect common leading directory prefix
    var prefix string
    if len(r.File) > 0 {
        name := r.File[0].Name
        if idx := strings.IndexByte(name, '/'); idx > 0 {
            prefix = name[:idx+1]
        }
    }

    for _, f := range r.File {
        name := f.Name
        if prefix != "" && strings.HasPrefix(name, prefix) {
            name = strings.TrimPrefix(name, prefix)
        }
        if name == "" {
            continue
        }
        outPath := filepath.Join(dest, filepath.FromSlash(name))

        if f.FileInfo().IsDir() {
            if err := ensureDir(outPath); err != nil {
                return err
            }
            continue
        }

        if err := ensureDir(filepath.Dir(outPath)); err != nil {
            return err
        }

        rc, err := f.Open()
        if err != nil {
            return err
        }
        func() {
            defer rc.Close()
            out, err := os.OpenFile(outPath, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0o644)
            if err != nil {
                rc.Close()
                panic(err)
            }
            defer out.Close()
            if _, err := io.Copy(out, rc); err != nil {
                panic(err)
            }
        }()
    }

    return nil
}

// ResolveGitHubImport converts a gh: import path to a local file path.
// Format: gh:owner/repo[/subpath]
// Returns absolute/relative path under ModulesRoot.
func ResolveGitHubImport(importPath string) (string, error) {
    path := strings.TrimPrefix(importPath, "gh:")
    path = strings.TrimPrefix(path, "/")
    if path == "" {
        return "", errors.New("empty gh: path")
    }
    // Expect owner/repo...
    parts := strings.Split(path, "/")
    if len(parts) < 2 {
        return "", fmt.Errorf("invalid gh: path %q, expected gh:owner/repo[/path]", importPath)
    }
    owner := parts[0]
    repo := parts[1]
    sub := strings.Join(parts[2:], "/")

    base := filepath.Join(ModulesRoot(), owner, repo)
    if sub == "" {
        // Default entry point: main.dax in repo root
        return filepath.Join(base, "main.dax"), nil
    }
    return filepath.Join(base, filepath.FromSlash(sub)), nil
}

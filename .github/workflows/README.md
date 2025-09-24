# GitHub Actions Workflows

This directory contains the CI/CD workflows for the DariX programming language project.

## Workflows Overview

### 1. `ci.yml` - Continuous Integration
**Triggers:** Push to `main`/`develop` branches, Pull Requests

**Purpose:** 
- Automated testing and quality assurance
- Multi-platform testing (Ubuntu, Windows, macOS)
- Multiple Go versions (1.21, 1.22)
- Code linting with golangci-lint

**Jobs:**
- **test**: Runs Go tests and DariX test suite on multiple platforms
- **lint**: Code quality checks and linting

**When it runs:**
- Every push to main/develop branches
- Every pull request to main/develop branches

### 2. `release.yml` - Release Automation
**Triggers:** Git tags matching `v*.*.*` pattern, Manual dispatch

**Purpose:**
- Automated release creation
- Multi-platform binary builds
- GitHub release with artifacts

**Jobs:**
- **build**: Creates binaries for multiple platforms (Linux, Windows, macOS, Android)
- **release**: Creates GitHub release with built artifacts

**When it runs:**
- When you create a git tag like `v1.0.0`, `v1.2.3`, etc.
- Manual trigger via GitHub Actions UI

## Workflow Separation Strategy

### Why Two Separate Workflows?

1. **Different Purposes:**
   - `ci.yml`: Testing and validation
   - `release.yml`: Production releases

2. **Different Triggers:**
   - `ci.yml`: Every push/PR (frequent)
   - `release.yml`: Only on version tags (infrequent)

3. **Resource Optimization:**
   - CI runs lightweight tests frequently
   - Release builds heavy artifacts only when needed

4. **Clear Separation of Concerns:**
   - Development workflow vs. Release workflow
   - No conflicts or duplicate builds

## How to Create a Release

### Method 1: Git Tags (Recommended)
```bash
# Create and push a version tag
git tag v1.0.0
git push origin v1.0.0
```

### Method 2: Manual Trigger
1. Go to GitHub Actions tab
2. Select "Release DariX" workflow
3. Click "Run workflow"
4. Choose branch and run

## Workflow Dependencies

```
Push to main/develop
├── ci.yml (always runs)
│   ├── test (multi-platform)
│   └── lint
│
Tag v*.*.* 
└── release.yml (only on tags)
    ├── build (multi-platform binaries)
    └── release (GitHub release)
```

## Platform Support

### CI Testing Platforms:
- Ubuntu Latest
- Windows Latest  
- macOS Latest

### Release Build Platforms:
- Linux AMD64
- Windows AMD64
- macOS AMD64
- Android ARM64 (for Termux)

## Environment Variables

No special environment variables required. All workflows use:
- `GITHUB_TOKEN`: Automatically provided by GitHub
- Standard GitHub context variables

## Troubleshooting

### Common Issues:

1. **Release not triggered:**
   - Ensure tag follows `v*.*.*` pattern
   - Check if tag was pushed: `git push origin --tags`

2. **Build failures:**
   - Check Go version compatibility
   - Verify all tests pass locally first

3. **Permission errors:**
   - Workflows have `contents: write` permission for releases
   - No additional setup required

### Debug Steps:

1. Check workflow run logs in GitHub Actions tab
2. Verify trigger conditions (branch, tag pattern)
3. Test locally with same Go version
4. Check for any dependency issues

## Maintenance

### Updating Go Versions:
Edit the `go-version` matrix in both workflows when new Go versions are released.

### Adding New Platforms:
Add new entries to the `matrix.include` section in `release.yml`.

### Modifying Triggers:
Update the `on:` section in respective workflow files.

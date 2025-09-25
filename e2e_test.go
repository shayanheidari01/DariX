package main

import (
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
)

// TestDariXEndToEndSuite runs the official DariX test runner via the CLI to
// ensure the language features continue to work together end-to-end.
func TestDariXEndToEndSuite(t *testing.T) {
	scriptPath := filepath.Join("tests", "test_runner.dax")

	cmd := exec.Command("go", "run", ".", "run", scriptPath)
	cmd.Env = os.Environ()

	var stdout, stderr bytes.Buffer
	cmd.Stdout = &stdout
	cmd.Stderr = &stderr

	if err := cmd.Run(); err != nil {
		t.Fatalf("darix test runner failed: %v\nstdout:\n%s\nstderr:\n%s", err, stdout.String(), stderr.String())
	}

	if stdout.Len() == 0 {
		t.Fatalf("darix test runner produced no output")
	}
}

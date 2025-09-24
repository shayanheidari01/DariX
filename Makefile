# DariX Programming Language Makefile

# Variables
BINARY_NAME=darix
BINARY_UNIX=$(BINARY_NAME)
BINARY_WINDOWS=$(BINARY_NAME).exe
VERSION=$(shell git describe --tags --always --dirty 2>/dev/null || echo "dev")
BUILD_TIME=$(shell date +%FT%T%z)
LDFLAGS=-ldflags "-X darix/internal/version.Version=$(VERSION) -X darix/internal/version.BuildTime=$(BUILD_TIME)"

# Default target
.PHONY: all
all: clean build test

# Build for current platform
.PHONY: build
build:
	@echo "🚀 Building DariX..."
	go build $(LDFLAGS) -o $(BINARY_NAME) .
	@echo "✅ Build completed!"

# Build for all platforms
.PHONY: build-all
build-all: build-linux build-windows build-darwin

# Build for Linux
.PHONY: build-linux
build-linux:
	@echo "🐧 Building for Linux..."
	GOOS=linux GOARCH=amd64 go build $(LDFLAGS) -o build/$(BINARY_NAME)-linux-amd64 .

# Build for Windows
.PHONY: build-windows
build-windows:
	@echo "🪟 Building for Windows..."
	GOOS=windows GOARCH=amd64 go build $(LDFLAGS) -o build/$(BINARY_NAME)-windows-amd64.exe .

# Build for macOS
.PHONY: build-darwin
build-darwin:
	@echo "🍎 Building for macOS..."
	GOOS=darwin GOARCH=amd64 go build $(LDFLAGS) -o build/$(BINARY_NAME)-darwin-amd64 .

# Run tests
.PHONY: test
test:
	@echo "🧪 Running tests..."
	@if [ -f $(BINARY_NAME) ]; then \
		./$(BINARY_NAME) run tests/test_runner.dax; \
	elif [ -f $(BINARY_WINDOWS) ]; then \
		./$(BINARY_WINDOWS) run tests/test_runner.dax; \
	else \
		echo "❌ Binary not found. Run 'make build' first."; \
		exit 1; \
	fi

# Run comprehensive test
.PHONY: test-comprehensive
test-comprehensive:
	@echo "🧪 Running comprehensive tests..."
	@if [ -f $(BINARY_NAME) ]; then \
		./$(BINARY_NAME) run examples/comprehensive_test.dax; \
	elif [ -f $(BINARY_WINDOWS) ]; then \
		./$(BINARY_WINDOWS) run examples/comprehensive_test.dax; \
	else \
		echo "❌ Binary not found. Run 'make build' first."; \
		exit 1; \
	fi

# Start REPL
.PHONY: repl
repl:
	@if [ -f $(BINARY_NAME) ]; then \
		./$(BINARY_NAME) repl; \
	elif [ -f $(BINARY_WINDOWS) ]; then \
		./$(BINARY_WINDOWS) repl; \
	else \
		echo "❌ Binary not found. Run 'make build' first."; \
		exit 1; \
	fi

# Clean build artifacts
.PHONY: clean
clean:
	@echo "🧹 Cleaning..."
	rm -f $(BINARY_NAME) $(BINARY_WINDOWS)
	rm -rf build/

# Install dependencies
.PHONY: deps
deps:
	@echo "📦 Installing dependencies..."
	go mod tidy
	go mod download

# Format code
.PHONY: fmt
fmt:
	@echo "🎨 Formatting code..."
	go fmt ./...

# Lint code
.PHONY: lint
lint:
	@echo "🔍 Linting code..."
	@if command -v golangci-lint >/dev/null 2>&1; then \
		golangci-lint run; \
	else \
		echo "⚠️  golangci-lint not found. Install it with: go install github.com/golangci/golangci-lint/cmd/golangci-lint@latest"; \
	fi

# Run benchmarks
.PHONY: bench
bench:
	@echo "⚡ Running benchmarks..."
	go test -bench=. ./...

# Create release
.PHONY: release
release: clean build-all
	@echo "📦 Creating release..."
	mkdir -p release
	cp build/* release/
	cp README.md LICENSE release/
	@echo "✅ Release created in release/ directory"

# Development setup
.PHONY: dev-setup
dev-setup: deps
	@echo "🛠️  Setting up development environment..."
	@if ! command -v golangci-lint >/dev/null 2>&1; then \
		echo "Installing golangci-lint..."; \
		go install github.com/golangci/golangci-lint/cmd/golangci-lint@latest; \
	fi
	@echo "✅ Development environment ready!"

# Show help
.PHONY: help
help:
	@echo "DariX Programming Language - Available targets:"
	@echo ""
	@echo "  build           Build for current platform"
	@echo "  build-all       Build for all platforms (Linux, Windows, macOS)"
	@echo "  build-linux     Build for Linux"
	@echo "  build-windows   Build for Windows"
	@echo "  build-darwin    Build for macOS"
	@echo "  test            Run test suite"
	@echo "  test-comprehensive  Run comprehensive tests"
	@echo "  repl            Start interactive REPL"
	@echo "  clean           Clean build artifacts"
	@echo "  deps            Install dependencies"
	@echo "  fmt             Format code"
	@echo "  lint            Lint code"
	@echo "  bench           Run benchmarks"
	@echo "  release         Create release build"
	@echo "  dev-setup       Setup development environment"
	@echo "  help            Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make build && make test    # Build and test"
	@echo "  make repl                  # Start REPL"
	@echo "  make release               # Create release build"

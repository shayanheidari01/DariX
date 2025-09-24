#!/bin/bash

# DariX Build Script
# اسکریپت ساخت DariX

echo "🚀 Building DariX Programming Language"
echo "======================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Check if Go is installed
if ! command -v go &> /dev/null; then
    print_error "Go is not installed. Please install Go 1.21+ to continue."
    exit 1
fi

print_info "Go version: $(go version)"

# Clean previous builds
print_info "Cleaning previous builds..."
rm -f darix darix.exe

# Build for current platform
print_info "Building DariX for current platform..."
if go build -o darix .; then
    print_success "Build completed successfully!"
else
    print_error "Build failed!"
    exit 1
fi

# Make executable on Unix systems
if [[ "$OSTYPE" != "msys" && "$OSTYPE" != "win32" ]]; then
    chmod +x darix
fi

# Run tests
print_info "Running comprehensive tests..."
if ./darix run tests/test_runner.dax > /dev/null 2>&1; then
    print_success "All tests passed!"
else
    print_warning "Some tests failed. Run './darix run tests/test_runner.dax' for details."
fi

# Display build information
print_info "Build information:"
echo "  📁 Executable: $(pwd)/darix$(if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then echo '.exe'; fi)"
echo "  📊 Size: $(du -h darix$(if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then echo '.exe'; fi) | cut -f1)"
echo "  🎯 Version: $(./darix version 2>/dev/null || echo 'Unknown')"

print_success "DariX build completed successfully!"
echo ""
echo "Usage examples:"
echo "  ./darix repl                    # Start interactive REPL"
echo "  ./darix run examples/hello_world.dax  # Run a DariX file"
echo "  ./darix run tests/test_runner.dax      # Run test suite"
echo "  ./darix --help                  # Show help"

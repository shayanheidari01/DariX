#!/bin/bash

# DariX Installation Script for Termux (Android)
# This script will install the DariX programming language interpreter on Termux

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print functions
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running on Termux
check_termux() {
    if [ -n "$ANDROID_ROOT" ] || [ -n "$PREFIX" ]; then
        print_info "Detected Termux environment"
    else
        print_warning "This script is designed for Termux on Android."
        read -p "Continue anyway? (y/N): " confirm
        if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Check if Go is installed
check_go() {
    if ! command -v go &> /dev/null; then
        print_error "Go is not installed. Please install Go and try again."
        print_info "In Termux, you can install Go with:"
        print_info "  pkg install golang"
        exit 1
    fi
    
    # Check Go version
    GO_VERSION=$(go version | grep -o 'go[0-9]\+\.[0-9]\+' | cut -d 'o' -f 2)
    MAJOR_VERSION=$(echo "$GO_VERSION" | cut -d '.' -f 1)
    MINOR_VERSION=$(echo "$GO_VERSION" | cut -d '.' -f 2)
    
    if [ "$MAJOR_VERSION" -lt 1 ] || [ "$MAJOR_VERSION" -eq 1 ] && [ "$MINOR_VERSION" -lt 16 ]; then
        print_warning "Go version 1.16 or higher is recommended. Your version is $GO_VERSION."
    else
        print_info "Found Go version $GO_VERSION"
    fi
}

# Build DariX interpreter
build_darix() {
    print_info "Building DariX interpreter..."
    
    # Check if main.go exists
    if [ ! -f "main.go" ]; then
        print_error "main.go not found. Please run this script from the DariX project root directory."
        exit 1
    fi
    
    # Build the interpreter
    # In Termux, we might need specific build flags
    if go build -o darix main.go; then
        print_info "Successfully built DariX interpreter"
    else
        print_error "Failed to build DariX interpreter"
        exit 1
    fi
}

# Install DariX in Termux
install_termux() {
    local install_path="$PREFIX/bin"
    
    print_info "Installing DariX to $install_path..."
    
    # In Termux, PREFIX is typically /data/data/com.termux/files/usr
    if cp darix "$install_path/"; then
        print_info "Successfully installed DariX to $install_path"
        print_info "You can now use 'darix' command from anywhere in Termux"
    else
        print_error "Failed to install DariX to $install_path"
        return 1
    fi
}

# Show usage instructions
show_usage() {
    echo
    print_info "DariX Installation Complete!"
    echo
    print_info "Usage:"
    echo "  To run a DariX script:"
    echo "    darix path/to/script.drx"
    echo
    print_info "  To start the REPL:"
    echo "    darix"
    echo
    print_info "Example:"
    echo "  Create a file named hello.drx with content:"
    echo "    print(\"Hello, DariX!\");"
    echo "  Then run:"
    echo "    darix hello.drx"
    echo
}

# Main execution
print_info "DariX Installation Script for Termux"
print_info "==================================="

# Check if running on Termux
check_termux

# Check if script is run from the correct directory
if [ ! -f "main.go" ] || [ ! -d "lexer" ] || [ ! -d "parser" ]; then
    print_error "This script must be run from the DariX project root directory."
    print_error "Please navigate to the directory containing main.go and run this script again."
    exit 1
fi

# Run installation steps
check_go
build_darix
install_termux

# Show usage instructions
show_usage

print_info "Thank you for installing DariX on Termux!"
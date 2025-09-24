#!/bin/bash

# DariX Programming Language Installation Script
# Supports Linux and Termux (Android)
# Author: DariX Team
# Version: 1.0.0

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
REPO_URL="https://github.com/shayanheidari01/DariX.git"
INSTALL_DIR="/usr/local/bin"
TERMUX_INSTALL_DIR="$PREFIX/bin"
GO_VERSION="1.21.0"

# Global variables
PLATFORM=""
ARCH=""
INSTALL_PATH=""
IS_TERMUX=false
TEMP_DIR=""

# Print functions
print_header() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                    DariX Installation Script                 ║"
    echo "║                   Programming Language                       ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${CYAN}[STEP]${NC} $1"
}

# Check if running in Termux
check_termux() {
    if [[ -n "$TERMUX_VERSION" ]] || [[ -d "/data/data/com.termux" ]] || [[ "$PREFIX" == *"com.termux"* ]]; then
        IS_TERMUX=true
        INSTALL_PATH="$TERMUX_INSTALL_DIR"
        print_info "Detected Termux environment"
    else
        INSTALL_PATH="$INSTALL_DIR"
        print_info "Detected standard Linux environment"
    fi
}

# Detect platform
detect_platform() {
    print_step "Detecting platform..."
    
    local uname_output=$(uname -s)
    case "$uname_output" in
        Linux*)
            if $IS_TERMUX; then
                PLATFORM="android"
                print_info "Platform: Android (Termux)"
            else
                PLATFORM="linux"
                print_info "Platform: Linux"
            fi
            ;;
        Darwin*)
            PLATFORM="darwin"
            print_info "Platform: macOS"
            ;;
        CYGWIN*|MINGW*|MSYS*)
            print_error "Windows is not supported by this script. Please download the Windows binary manually."
            exit 1
            ;;
        *)
            print_error "Unsupported platform: $uname_output"
            exit 1
            ;;
    esac
}

# Detect architecture
detect_architecture() {
    print_step "Detecting architecture..."
    
    local uname_arch=$(uname -m)
    case "$uname_arch" in
        x86_64|amd64)
            ARCH="amd64"
            ;;
        aarch64|arm64)
            ARCH="arm64"
            ;;
        armv7l|armv6l)
            if $IS_TERMUX; then
                ARCH="arm64"  # Most modern Android devices use arm64
            else
                print_error "ARM 32-bit is not supported. Please use a 64-bit system."
                exit 1
            fi
            ;;
        *)
            print_error "Unsupported architecture: $uname_arch"
            exit 1
            ;;
    esac
    
    print_info "Architecture: $ARCH"
}

# Check dependencies
check_dependencies() {
    print_step "Checking dependencies..."
    
    local missing_deps=()
    
    # Check for git
    if ! command -v git >/dev/null 2>&1; then
        missing_deps+=("git")
    fi
    
    # Check for curl or wget
    if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
        missing_deps+=("curl or wget")
    fi
    
    # Check for tar
    if ! command -v tar >/dev/null 2>&1; then
        missing_deps+=("tar")
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        
        if $IS_TERMUX; then
            print_info "Install missing dependencies with: pkg install ${missing_deps[*]}"
        else
            print_info "Install missing dependencies with your package manager:"
            print_info "  Ubuntu/Debian: sudo apt update && sudo apt install ${missing_deps[*]}"
            print_info "  CentOS/RHEL: sudo yum install ${missing_deps[*]}"
            print_info "  Fedora: sudo dnf install ${missing_deps[*]}"
            print_info "  Arch: sudo pacman -S ${missing_deps[*]}"
        fi
        exit 1
    fi
    
    print_success "All dependencies are available"
}

# Check if Go is installed
check_go_installation() {
    print_step "Checking Go installation..."
    
    if command -v go >/dev/null 2>&1; then
        local go_version=$(go version | cut -d' ' -f3 | sed 's/go//')
        print_success "Go is already installed: $go_version"
        return 0
    else
        print_warning "Go is not installed"
        return 1
    fi
}

# Install Go
install_go() {
    print_step "Installing Go $GO_VERSION..."
    
    local go_arch=""
    case "$ARCH" in
        amd64) go_arch="amd64" ;;
        arm64) go_arch="arm64" ;;
        *) 
            print_error "Unsupported architecture for Go installation: $ARCH"
            exit 1
            ;;
    esac
    
    local go_os=""
    if $IS_TERMUX; then
        go_os="linux"
    else
        case "$PLATFORM" in
            linux) go_os="linux" ;;
            darwin) go_os="darwin" ;;
            *)
                print_error "Unsupported platform for Go installation: $PLATFORM"
                exit 1
                ;;
        esac
    fi
    
    local go_filename="go${GO_VERSION}.${go_os}-${go_arch}.tar.gz"
    local go_url="https://golang.org/dl/${go_filename}"
    local go_temp_file="$TEMP_DIR/$go_filename"
    
    print_info "Downloading Go from: $go_url"
    
    if command -v curl >/dev/null 2>&1; then
        if ! curl -L -o "$go_temp_file" "$go_url"; then
            print_error "Failed to download Go"
            exit 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if ! wget -O "$go_temp_file" "$go_url"; then
            print_error "Failed to download Go"
            exit 1
        fi
    fi
    
    # Install Go
    if $IS_TERMUX; then
        local go_install_dir="$PREFIX/lib/go"
        print_info "Installing Go to $go_install_dir"
        
        # Remove existing Go installation
        rm -rf "$go_install_dir"
        
        # Extract Go
        mkdir -p "$PREFIX/lib"
        tar -C "$PREFIX/lib" -xzf "$go_temp_file"
        mv "$PREFIX/lib/go" "$go_install_dir"
        
        # Add Go to PATH
        if ! grep -q "$go_install_dir/bin" "$PREFIX/etc/bash.bashrc" 2>/dev/null; then
            echo "export PATH=\"$go_install_dir/bin:\$PATH\"" >> "$PREFIX/etc/bash.bashrc"
        fi
        export PATH="$go_install_dir/bin:$PATH"
        
    else
        local go_install_dir="/usr/local"
        print_info "Installing Go to $go_install_dir"
        
        # Remove existing Go installation
        sudo rm -rf "$go_install_dir/go"
        
        # Extract Go
        sudo tar -C "$go_install_dir" -xzf "$go_temp_file"
        
        # Add Go to PATH
        if ! grep -q "/usr/local/go/bin" /etc/profile 2>/dev/null; then
            echo "export PATH=\"/usr/local/go/bin:\$PATH\"" | sudo tee -a /etc/profile
        fi
        export PATH="/usr/local/go/bin:$PATH"
    fi
    
    # Verify Go installation
    if command -v go >/dev/null 2>&1; then
        local installed_version=$(go version | cut -d' ' -f3 | sed 's/go//')
        print_success "Go installed successfully: $installed_version"
    else
        print_error "Go installation failed"
        exit 1
    fi
}

# Clone repository
clone_repository() {
    print_step "Cloning DariX repository..."
    
    local repo_dir="$TEMP_DIR/DariX"
    
    if ! git clone "$REPO_URL" "$repo_dir"; then
        print_error "Failed to clone repository"
        exit 1
    fi
    
    print_success "Repository cloned successfully"
    echo "$repo_dir"
}

# Build DariX
build_darix() {
    local repo_dir="$1"
    print_step "Building DariX from source..."
    
    cd "$repo_dir"
    
    # Build the binary
    if ! go build -o darix main.go; then
        print_error "Failed to build DariX"
        exit 1
    fi
    
    print_success "DariX built successfully"
    
    # Install binary
    install_binary "$repo_dir/darix"
}

# Install binary
install_binary() {
    local source_file="$1"
    local target_file="$INSTALL_PATH/darix"
    
    print_step "Installing DariX to $target_file..."
    
    # Create install directory if it doesn't exist
    if [[ ! -d "$INSTALL_PATH" ]]; then
        if $IS_TERMUX; then
            mkdir -p "$INSTALL_PATH"
        else
            if ! sudo mkdir -p "$INSTALL_PATH"; then
                print_error "Failed to create installation directory: $INSTALL_PATH"
                exit 1
            fi
        fi
    fi
    
    # Install binary
    if $IS_TERMUX; then
        if ! cp "$source_file" "$target_file"; then
            print_error "Failed to install binary to $target_file"
            exit 1
        fi
    else
        if ! sudo cp "$source_file" "$target_file"; then
            print_error "Failed to install binary to $target_file"
            print_info "You may need to run this script with sudo privileges"
            exit 1
        fi
    fi
    
    # Ensure binary is executable
    if $IS_TERMUX; then
        chmod +x "$target_file"
    else
        sudo chmod +x "$target_file"
    fi
    
    print_success "DariX installed successfully to $target_file"
}

# Verify installation
verify_installation() {
    print_step "Verifying installation..."
    
    if ! command -v darix >/dev/null 2>&1; then
        print_warning "darix command not found in PATH"
        print_info "You may need to add $INSTALL_PATH to your PATH"
        print_info "Add this line to your ~/.bashrc or ~/.zshrc:"
        print_info "export PATH=\"$INSTALL_PATH:\$PATH\""
        return 1
    fi
    
    local version_output
    if version_output=$(darix --version 2>/dev/null || darix -v 2>/dev/null || echo "DariX installed"); then
        print_success "Installation verified: $version_output"
        return 0
    else
        print_warning "Could not verify installation"
        return 1
    fi
}

# Create test file
create_test_file() {
    print_step "Creating test file..."
    
    local test_file="darix_test.dax"
    cat > "$test_file" << 'EOF'
// DariX Test Program
print("Hello! Welcome to DariX Programming Language");

var name = "DariX";
var version = "0.1.1";

print("Language:", name);
print("Version:", version);

// Test basic operations
var a = 10;
var b = 20;
var sum = a + b;

print("Sum of", a, "and", b, "is:", sum);

// Test function
func greet(name) {
    return "Hello, " + name + "!";
}

print(greet("World"));
print("DariX installation test completed successfully!");
EOF
    
    print_success "Test file created: $test_file"
}

# Run test
run_test() {
    print_step "Running installation test..."
    
    local test_file="darix_test.dax"
    create_test_file
    
    if command -v darix >/dev/null 2>&1; then
        print_info "Running: darix run $test_file"
        if darix run "$test_file"; then
            print_success "Test completed successfully!"
        else
            print_warning "Test failed, but DariX is installed"
        fi
    else
        print_warning "Cannot run test - darix not in PATH"
        print_info "Try running: $INSTALL_PATH/darix run $test_file"
    fi
    
    # Cleanup test file
    rm -f "$test_file"
}

# Print usage information
print_usage_info() {
    echo
    print_info "DariX has been installed successfully!"
    echo
    echo -e "${CYAN}Usage Examples:${NC}"
    echo "  darix run script.dax          # Run a DariX script"
    echo "  darix repl                    # Start interactive REPL"
    echo "  darix eval \"print('Hello')\"   # Evaluate expression"
    echo "  darix --help                  # Show help"
    echo
    echo -e "${CYAN}Getting Started:${NC}"
    echo "  1. Create a file with .dax extension"
    echo "  2. Write your DariX code"
    echo "  3. Run with: darix run yourfile.dax"
    echo
    echo -e "${CYAN}Documentation:${NC}"
    echo "  GitHub: $REPO_URL"
    echo "  README: $REPO_URL/blob/main/README.md"
    echo
}

# Main installation function
main() {
    print_header
    
    # Check if user wants help
    if [[ "$1" == "--help" ]] || [[ "$1" == "-h" ]]; then
        echo "DariX Installation Script"
        echo
        echo "Usage: $0 [options]"
        echo
        echo "Options:"
        echo "  --help, -h     Show this help message"
        echo "  --test         Run installation test after install"
        echo "  --no-verify    Skip installation verification"
        echo
        echo "This script automatically detects your platform and architecture"
        echo "and installs the appropriate DariX binary."
        echo
        exit 0
    fi
    
    local run_test=false
    local skip_verify=false
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --test)
                run_test=true
                shift
                ;;
            --no-verify)
                skip_verify=true
                shift
                ;;
            *)
                print_warning "Unknown option: $1"
                shift
                ;;
        esac
    done
    
    # Create temporary directory
    TEMP_DIR=$(mktemp -d)
    trap "rm -rf $TEMP_DIR" EXIT
    
    # Installation steps
    check_termux
    detect_platform
    detect_architecture
    check_dependencies
    
    # Install Go if not present
    if ! check_go_installation; then
        install_go
    fi
    
    # Clone and build DariX
    local repo_dir=$(clone_repository)
    build_darix "$repo_dir"
    
    # Verification and testing
    if [[ "$skip_verify" != true ]]; then
        if verify_installation; then
            print_success "Installation completed successfully!"
        else
            print_warning "Installation completed with warnings"
        fi
    fi
    
    if [[ "$run_test" == true ]]; then
        run_test
    fi
    
    print_usage_info
    
    print_success "DariX installation completed!"
}

# Run main function with all arguments
main "$@"

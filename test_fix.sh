#!/bin/bash

# Test script to verify the fix
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

print_step() {
    echo -e "${CYAN}[STEP]${NC} $1" >&2
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" >&2
}

test_function() {
    print_step "Testing function..."
    local test_dir="/tmp/test_directory"
    print_success "Function completed successfully"
    echo "$test_dir"
}

# Test the capture
echo "Testing directory capture..."
captured_dir=$(test_function)
echo "Captured directory: '$captured_dir'"

if [[ "$captured_dir" == "/tmp/test_directory" ]]; then
    echo "✅ Fix works! Directory captured correctly without ANSI codes."
else
    echo "❌ Fix failed! Captured: '$captured_dir'"
fi

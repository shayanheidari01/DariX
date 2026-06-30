# Building DariX (C++)

## Requirements

- **C++17** compatible compiler:
  - GCC 7+ (Linux, MinGW/MSYS2 on Windows)
  - Clang 5+ (macOS, Linux)
  - MSVC 2017+ (Windows)
- **CMake 3.16+**
- **Windows**: Winsock2 (`ws2_32.lib`) for networking

## Quick Build

```bash
cd cpp-src
cmake -S . -B build
cmake --build build
```

The executable will be at `build/darix.exe` (Windows) or `build/darix` (Linux/macOS).

## Platform-Specific Instructions

### Windows (MSYS2/MinGW)
```bash
cd cpp-src
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

### Windows (Visual Studio)
```bash
cd cpp-src
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Linux
```bash
cd cpp-src
cmake -S . -B build
cmake --build build
```

### macOS
```bash
cd cpp-src
cmake -S . -B build -G "Xcode"
cmake --build build
```

## Optional Dependencies

### OpenSSL (for crypto module)
If OpenSSL is installed, the crypto module will use it for stronger hashing algorithms. Without it, the module uses pure C++ implementations.

```bash
# Ubuntu/Debian
sudo apt install libssl-dev

# macOS
brew install openssl
```

### libcurl (for HTTP client)
If libcurl is installed, it may be used for HTTP operations. Without it, raw sockets are used.

```bash
# Ubuntu/Debian
sudo apt install libcurl4-openssl-dev
```

## Install

```bash
# Build and install to /usr/local/bin (Linux/macOS)
cd cpp-src
cmake -S . -B build
cmake --build build
sudo cmake --install build

# Or copy manually
cp build/darix /usr/local/bin/
```

## Build Options

```bash
# Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Debug build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Static linking
cmake -S . -B build -DCMAKE_EXE_LINKER_FLAGS="-static"
```

## Cross-Compilation

```bash
# ARM64 on x86_64 Linux
cmake -S . -B build -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++

# Windows cross-compilation from Linux
cmake -S . -B build -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
```

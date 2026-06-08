cmake_minimum_required(VERSION 3.20)
project(DariX_JIT VERSION 2.0.0 LANGUAGES CXX)

# C++20 for improved performance and modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined")

# Enable warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# Project structure
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
)

set(HEADERS
    include/token.h
    include/lexer.h
    include/ast.h
    include/parser.h
    include/object.h
    include/environment.h
    include/interpreter.h
    include/builtins.h
    include/compiler.h
    include/jit.h
)

# Create executable
add_executable(darix ${SOURCES})

# Enable LTO for Release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_output)
if(lto_supported)
    set_property(TARGET darix PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Testing
enable_testing()
add_executable(darix_tests tests/test_main.cpp)
target_link_libraries(darix_tests darix)
add_test(NAME unit_tests COMMAND darix_tests)

# Installation
install(TARGETS darix DESTINATION bin)

# Print configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

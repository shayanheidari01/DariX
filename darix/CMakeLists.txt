cmake_minimum_required(VERSION 3.20)
project(DariX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Source files
set(DARIX_SOURCES
    src/main.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
    src/semantic/analyzer.cpp
    src/interpreter/interpreter.cpp
    src/runtime/environment.cpp
    src/runtime/value.cpp
    src/runtime/gc.cpp
    src/stdlib/core.cpp
)

# Main executable
add_executable(darix ${DARIX_SOURCES})
target_include_directories(darix PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Test executable
set(TEST_SOURCES
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_interpreter.cpp
)
add_executable(darix_tests ${TEST_SOURCES} ${DARIX_SOURCES})
target_include_directories(darix_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Enable testing
enable_testing()
add_test(NAME DariXTests COMMAND darix_tests)

# Install
install(TARGETS darix DESTINATION bin)

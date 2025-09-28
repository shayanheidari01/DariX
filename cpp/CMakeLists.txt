cmake_minimum_required(VERSION 3.16)
project(DariX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# LLVM configuration for JIT
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# LLVM libraries for JIT
llvm_map_components_to_libnames(llvm_libs support core irreader executionengine orcjit native)

# Project structure
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/interpreter.cpp
    src/object.cpp
    src/environment.cpp
    src/builtins.cpp
)

# Create executable
add_executable(darix ${SOURCES})

# Link LLVM libraries
target_link_libraries(darix ${llvm_libs})

# Enable testing
enable_testing()

# Test executable
add_executable(darix_tests tests/test_main.cpp)
target_link_libraries(darix_tests ${llvm_libs})

# Add tests
add_test(NAME unit_tests COMMAND darix_tests)

# Installation
install(TARGETS darix DESTINATION bin)

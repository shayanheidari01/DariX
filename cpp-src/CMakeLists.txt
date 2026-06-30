cmake_minimum_required(VERSION 3.16)
project(DariX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(darix ${SOURCES})
target_include_directories(darix PRIVATE include)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(darix PRIVATE pthread)
endif()

if(WIN32)
    target_compile_definitions(darix PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Optional: HTTP client (libcurl)
find_package(CURL QUIET)
if(CURL_FOUND)
    target_link_libraries(darix PRIVATE CURL::libcurl)
    target_compile_definitions(darix PRIVATE HAS_CURL)
endif()

# Optional: OpenSSL for crypto
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    target_link_libraries(darix PRIVATE OpenSSL::Crypto)
    target_compile_definitions(darix PRIVATE HAS_OPENSSL)
endif()

# Install
install(TARGETS darix RUNTIME DESTINATION bin)

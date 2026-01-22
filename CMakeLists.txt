cmake_minimum_required(VERSION 3.12)
project(federated VERSION 0.1.0 LANGUAGES CXX)

# C++17 standard, no exceptions, warnings as errors
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
add_library(federated_core
    src/core/error.cpp
    src/core/registry.cpp
)

target_include_directories(federated_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Test executable
add_executable(federated_tests
    tests/test_runner.cpp
    tests/main.cpp
    tests/core/test_buffer.cpp
    tests/core/test_error.cpp
    tests/core/test_packet.cpp
    tests/core/test_registry.cpp
)

target_link_libraries(federated_tests
    federated_core
)

# Enable testing
enable_testing()
add_test(NAME AllTests COMMAND federated_tests)

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

# Transport library
add_library(federated_transport
    src/transport/loopback.cpp
    src/transport/file.cpp
)

target_include_directories(federated_transport PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(federated_transport
    federated_core
)

# Framer library
add_library(federated_framer
    src/framer/raw.cpp
    src/framer/length_prefix.cpp
)

target_include_directories(federated_framer PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(federated_framer
    federated_core
)

# Crypto library
add_library(federated_crypto
    src/crypto/none.cpp
)

target_include_directories(federated_crypto PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(federated_crypto
    federated_core
)

# Service library (email subsystem)
add_library(federated_service
    src/service/mail.cpp
    src/service/smtp.cpp
    src/service/imap.cpp
)

target_include_directories(federated_service PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(federated_service
    federated_core
)

# Tool library (CLI framework)
add_library(federated_tool
    src/tool/cli.cpp
    src/tool/cli_commands.cpp
)

target_include_directories(federated_tool PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(federated_tool
    federated_core
    federated_service
)

# Test executable
add_executable(federated_tests
    tests/test_runner.cpp
    tests/main.cpp
    tests/core/test_buffer.cpp
    tests/core/test_error.cpp
    tests/core/test_packet.cpp
    tests/core/test_registry.cpp
    tests/transport/test_loopback.cpp
    tests/transport/test_file.cpp
    tests/framer/test_raw.cpp
    tests/framer/test_length_prefix.cpp
    tests/crypto/test_none.cpp
    tests/service/test_mail.cpp
    tests/service/test_smtp.cpp
    tests/service/test_imap.cpp
    tests/scenarios/scenario_file_transfer.cpp
    tests/scenarios/scenario_end_to_end_crypto.cpp
    tests/scenarios/scenario_store_and_forward.cpp
)

target_link_libraries(federated_tests
    federated_core
    federated_transport
    federated_framer
    federated_crypto
    federated_service
)

# Enable testing
enable_testing()
add_test(NAME AllTests COMMAND federated_tests)

# Examples
add_executable(simple_message examples/simple_message.cpp)
target_link_libraries(simple_message
    federated_core
    federated_transport
    federated_framer
    federated_crypto
)

# CLI executable
add_executable(federated src/tool/cli_main.cpp)
target_link_libraries(federated
    federated_core
    federated_transport
    federated_framer
    federated_crypto
    federated_service
    federated_tool
)

# Copilot Instructions for Federated Project

This file provides coding standards, architectural principles, and development guidelines that all AI assistants (including GitHub Copilot) should follow when contributing to the Federated project.

---

## Core Principles (Non-Negotiable)

### 1. **Zero External Dependencies**
- **ONLY C++17 standard library** - No external libraries, frameworks, or dependencies
- No OpenSSL, libcrypto, libcurl, libevent, Boost, etc.
- Only libc and OS syscalls are permitted
- Implement everything from first principles using standard C++17

**Example violations:**
```cpp
// ❌ WRONG - External dependency
#include <openssl/ssl.h>
#include <boost/asio.hpp>

// ✅ CORRECT - Standard library only
#include <cstdio>
#include <sys/socket.h>  // OS syscalls OK
```

### 2. **Test-First Development (TDD)**
- **Every feature starts with a failing test**
- Write the test BEFORE implementing the feature
- Tests must cover all code paths and edge cases
- Maintain 100% test coverage for critical paths

**Development workflow:**
1. Read and understand `TODO.md` for what needs to be built
2. Write test cases that define expected behavior
3. Run tests (they should fail)
4. Implement the minimal code to make tests pass
5. Refactor while keeping tests green
6. Update `TODO.md` to mark items complete

**Test organization:**
```
tests/
  core/          - Core primitive tests
  transport/     - Transport layer tests  
  service/       - Service tests
  e2e/           - End-to-end integration tests
  scenarios/     - User scenario tests
```

### 3. **RFC Compliance with Heavy Documentation**
- All protocol implementations MUST reference the relevant RFC
- Include RFC number, title, and URL at the top of each file
- Document which sections of the RFC are implemented
- Add inline comments referencing specific RFC sections

**Required documentation format:**
```cpp
/**
 * SMTP Protocol Implementation
 * 
 * RFC 5321 - Simple Mail Transfer Protocol
 * https://datatracker.ietf.org/doc/html/rfc5321
 * 
 * State Machine (Section 4.1.4):
 * INIT → GREETED → MAIL → RCPT → DATA → DONE
 * 
 * Commands Implemented:
 * - HELO/EHLO: Initiate session (Section 4.1.1.1)
 * - MAIL FROM: Specify sender (Section 4.1.1.2)
 * - RCPT TO: Specify recipient (Section 4.1.1.3)
 * - DATA: Begin message transmission (Section 4.1.1.4)
 */
```

### 4. **Flat and Atomic File Organization**
- **No deep inheritance hierarchies** - Prefer composition
- **One concept per file** - Keep files focused and small
- **Flat directory structure** - Mirror include/ in src/ and tests/
- **Atomic modules** - Each component should be independently testable

**File structure:**
```
include/federated/transport/tcp.h      - Public interface
src/transport/tcp.cpp                  - Implementation
tests/transport/test_tcp.cpp           - Unit tests
tests/scenarios/scenario_tcp_transfer.cpp  - Integration tests
```

### 5. **Explicit Ownership and Error Handling**
- **No exceptions** - All errors returned as explicit error codes
- **No hidden allocations** - Caller owns memory
- **No implicit state changes** - All state transitions visible
- Use `core::ErrorCode` enum for all error conditions

**Correct error handling:**
```cpp
// ✅ CORRECT - Explicit error codes
core::ErrorCode send_message(const Buffer& msg) {
    if (msg.size == 0) {
        return core::ERR_FORMAT;
    }
    // ... implementation
    return core::OK;
}

// ❌ WRONG - Exceptions
void send_message(const Buffer& msg) {
    if (msg.size == 0) {
        throw std::invalid_argument("Empty message");
    }
}
```

### 6. **TODO.md is the Source of Truth**
- Always check `TODO.md` before implementing new features
- Features not in `TODO.md` should be discussed first
- Update `TODO.md` when completing tasks
- Follow the phase structure defined in `TODO.md`

---

## Code Style Guidelines

### Naming Conventions
```cpp
// Namespaces: lowercase
namespace federated { namespace core { } }

// Structs/Classes: PascalCase
struct Buffer { };
struct SmtpSession { };

// Functions: snake_case
ErrorCode parse_command(const Buffer& input);

// Variables: snake_case
int message_count = 0;
const char* smtp_host = "localhost";

// Constants: UPPER_SNAKE_CASE
const size_t MAX_BUFFER_SIZE = 4096;

// Enum values: UPPER_SNAKE_CASE
enum ErrorCode {
    OK = 0,
    ERR_IO,
    ERR_FORMAT
};
```

### Header Structure
```cpp
#ifndef FEDERATED_MODULE_FILE_H
#define FEDERATED_MODULE_FILE_H

/**
 * Brief description of what this module does
 * 
 * RFC References (if applicable):
 * - RFC XXXX: Title
 *   https://datatracker.ietf.org/doc/html/rfcXXXX
 *   Description of what it defines
 * 
 * Implementation notes:
 * - List key implementation details
 * - State machine diagrams if applicable
 */

#include "../core/buffer.h"  // Relative includes within project

namespace federated {
namespace module {

// Interface definitions

} // namespace module
} // namespace federated

#endif // FEDERATED_MODULE_FILE_H
```

### Implementation Structure
```cpp
/**
 * Implementation file description
 * 
 * RFC XXXX references and implementation notes
 */

#include "federated/module/file.h"
#include <cstring>  // System includes after project includes

namespace federated {
namespace module {

// Static/internal helpers first
static void internal_helper() {
    // Implementation
}

// Public interface implementations
ErrorCode public_function() {
    // Implementation with error checking
    if (error_condition) {
        return ERR_FORMAT;
    }
    return OK;
}

} // namespace module
} // namespace federated
```

---

## Testing Standards

### Test File Structure
```cpp
#include "../test_runner.h"
#include "../../include/federated/module/feature.h"

using namespace federated::module;
using namespace federated::core;

// Test: Brief description
TEST(feature_basic_functionality) {
    // Setup
    Feature f;
    
    // Execute
    ErrorCode err = f.do_something();
    
    // Verify
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(f.is_valid());
}

// Test: Edge case
TEST(feature_error_handling) {
    Feature f;
    ErrorCode err = f.do_something_invalid();
    TEST_ASSERT_NE(err, OK);
}

// Test: RFC compliance
TEST(feature_rfc_compliance) {
    // Test that implementation follows RFC specification
    // Reference specific RFC sections being tested
}
```

### E2E Test Structure
```bash
#!/bin/bash
# E2E test for feature

source "$(dirname "$0")/lib.sh"

test_feature_scenario() {
    log_info "Testing feature scenario"
    
    start_service feature --port 8080
    
    # Perform test operations
    local result=$(curl localhost:8080)
    assert_contains "$result" "expected"
    
    stop_service feature
}

run_test "Feature Scenario" test_feature_scenario
```

---

## Common Patterns

### State Machines
State machines MUST be explicit enums with validation:
```cpp
enum SmtpState {
    SMTP_INIT,
    SMTP_GREETED,
    SMTP_MAIL,
    SMTP_RCPT,
    SMTP_DATA,
    SMTP_DONE,
    SMTP_ERROR
};

// State transitions must be validated
ErrorCode process_command(Session& session, const Command& cmd) {
    if (session.state != EXPECTED_STATE) {
        return ERR_FORMAT;  // Invalid state transition
    }
    session.state = NEXT_STATE;
    return OK;
}
```

### Buffer Management
```cpp
// Caller owns memory - no hidden allocations
struct Buffer {
    uint8_t* data;  // Pointer to memory
    size_t   size;  // Size of data
    
    Buffer() : data(nullptr), size(0) {}
    Buffer(uint8_t* d, size_t s) : data(d), size(s) {}
};

// Usage
uint8_t storage[1024];
Buffer buf(storage, sizeof(storage));
```

### Command Parsing
```cpp
// Simple, explicit parsing - no regex, no complex libraries
bool parse_command(const Buffer& line, Command& out) {
    size_t i = 0;
    // Parse character by character with explicit bounds checking
    while (i < line.size && line.data[i] != ' ') {
        // ... parsing logic
    }
    return true;
}
```

---

## Security Guidelines

### Buffer Safety
- Always check bounds before array access
- Use `strncpy` with explicit size limits
- Prefer `memcpy` with explicit lengths over `strcpy`
- Validate all input sizes

```cpp
// ✅ CORRECT
void safe_copy(char* dest, const char* src, size_t dest_size) {
    size_t src_len = strlen(src);
    if (src_len >= dest_size) src_len = dest_size - 1;
    memcpy(dest, src, src_len);
    dest[src_len] = '\0';
}

// ❌ WRONG
void unsafe_copy(char* dest, const char* src) {
    strcpy(dest, src);  // No bounds checking
}
```

### Input Validation
- Validate all external input
- Check for malformed data
- Fail safely with explicit error codes
- Log security-relevant events

---

## Architecture Layers

The project follows a strict layered architecture:

```
┌─────────────────────────────────────┐
│ Services (HTTP, SMTP, IMAP, etc.)   │
├─────────────────────────────────────┤
│ Message Router                      │
├─────────────────────────────────────┤
│ Onion Router (optional)             │
├─────────────────────────────────────┤
│ Crypto (optional)                   │
├─────────────────────────────────────┤
│ Framer                              │
├─────────────────────────────────────┤
│ Transport                           │
├─────────────────────────────────────┤
│ Core Primitives                     │
└─────────────────────────────────────┘
```

**Rules:**
- Lower layers MUST NOT depend on upper layers
- Each layer has a clear, minimal interface
- Layers communicate through explicit function calls
- No hidden side effects between layers

---

## Documentation Requirements

### Every Public Function Must Have:
```cpp
/**
 * Brief description of what the function does
 * 
 * RFC Reference (if applicable): Section X.Y.Z
 * 
 * @param input Description of input parameter
 * @param output Description of output parameter
 * @return Error code: OK on success, ERR_* on failure
 * 
 * Example usage:
 *   Buffer buf(data, size);
 *   ErrorCode err = function(buf, result);
 *   if (err != OK) { handle_error(); }
 */
ErrorCode function(const Buffer& input, Result& output);
```

### Every File Must Have:
1. File-level comment with purpose and RFC references
2. Copyright/license header (if applicable)
3. Include guards for headers
4. Namespace documentation
5. Clear separation of public vs private APIs

---

## CLI and Service Implementation

### Service Registration Pattern
```cpp
// Service definition
ServiceDef my_service;
my_service.name = "myservice";
my_service.description = "My Service (RFC XXXX)";
my_service.start = my_service_start;
my_service.stop = my_service_stop;
my_service.status = my_service_status;

// Options with environment variable mapping
my_service.options[0].name = "port";
my_service.options[0].env_var = "FEDERATED_MY_PORT";
my_service.options[0].default_value = "8080";

register_service(my_service);
```

### Logging Pattern
```cpp
#include "federated/tool/log.h"

// Use structured logging with appropriate levels
LOG_HEADER("Starting My Service");
LOG_INFO("Configuration: port=%d", port);
LOG_WARN("Using default configuration");
LOG_ERROR("Failed to bind: %s", error_message(err));
```

---

## What NOT to Do

### ❌ Don't Add External Dependencies
```cpp
// NEVER do this
#include <boost/asio.hpp>
#include <curl/curl.h>
#include <openssl/ssl.h>
```

### ❌ Don't Use Exceptions
```cpp
// NEVER do this
throw std::runtime_error("Error occurred");

// DO THIS instead
return core::ERR_INTERNAL;
```

### ❌ Don't Hide Memory Allocation
```cpp
// NEVER do this
Buffer* allocate_buffer() {
    return new Buffer();  // Hidden allocation
}

// DO THIS instead
struct Buffer {
    uint8_t* data;  // Caller provides memory
    size_t size;
};
```

### ❌ Don't Create Deep Inheritance
```cpp
// NEVER do this
class Base { };
class Derived : public Base { };
class MoreDerived : public Derived { };

// DO THIS instead - flat composition
struct Component1 { };
struct Component2 { };
struct Composite {
    Component1 c1;
    Component2 c2;
};
```

### ❌ Don't Skip Tests
```cpp
// NEVER commit code without tests
// Every function must have corresponding test cases
```

---

## Pre-Commit Checklist

Before committing code, verify:

- [ ] All tests pass (`./federated_tests`)
- [ ] E2E tests pass (`./tests/e2e/run_all.sh`)
- [ ] Build is clean with no warnings
- [ ] RFC references added to new protocol code
- [ ] `TODO.md` updated if completing tasks
- [ ] No external dependencies introduced
- [ ] No exceptions used
- [ ] All errors explicitly handled
- [ ] Memory ownership is clear
- [ ] Code follows naming conventions
- [ ] Documentation comments added
- [ ] Security guidelines followed

---

## Examples of Good Code

### Good: Simple, Explicit, Testable
```cpp
// smtp.h
struct SmtpCommand {
    char cmd[16];
    char arg[512];
};

bool parse_smtp_command(const Buffer& line, SmtpCommand& out);

// smtp.cpp
bool parse_smtp_command(const Buffer& line, SmtpCommand& out) {
    if (line.size == 0) return false;
    
    size_t i = 0;
    size_t j = 0;
    
    // Parse command
    while (i < line.size && line.data[i] != ' ') {
        if (j < 15) out.cmd[j++] = line.data[i];
        i++;
    }
    out.cmd[j] = '\0';
    
    return true;
}

// test_smtp.cpp
TEST(smtp_parse_command) {
    uint8_t data[] = "HELO example.com\r\n";
    Buffer buf(data, sizeof(data) - 1);
    SmtpCommand cmd;
    
    bool result = parse_smtp_command(buf, cmd);
    
    TEST_ASSERT(result);
    TEST_ASSERT_STR_EQ(cmd.cmd, "HELO");
}
```

---

## Getting Help

If you're unsure about:
- Architecture decisions → Check `README.md` and `docs/`
- What to implement next → Check `TODO.md`
- How to implement protocols → Check RFC references
- Test patterns → Look at existing tests in `tests/`
- Error handling → Check `include/federated/core/error.h`

---

## Summary

**Remember the core tenets:**
1. Zero dependencies (C++17 only)
2. Test-first development
3. RFC compliance with heavy comments
4. Flat, atomic file structure
5. Explicit ownership and error handling
6. Follow TODO.md for feature priorities

**When in doubt:**
- Keep it simple
- Make it explicit
- Test it thoroughly
- Document it completely
- Reference the RFC

This is a **resilient communication toolkit** designed to survive anything. The code must be equally resilient - simple, explicit, testable, and dependency-free.

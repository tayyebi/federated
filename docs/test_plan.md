# Federated Test Plan

**Version:** 1.0  
**Last Updated:** 2026-01-22

---

## 1. Testing Philosophy

The Federated project follows a **test-first development** methodology where:
1. Tests are written before implementation
2. Tests define the contract and expected behavior
3. All code must have 100% test coverage
4. Tests serve as executable documentation

### Test Pyramid

```
         /\
        /  \     E2E & User Scenarios (Few, slow, high value)
       /----\
      /      \   Integration Tests (Some, medium speed)
     /--------\
    /          \ Unit Tests (Many, fast, focused)
   /____________\
```

---

## 2. Test Categories

### 2.1 Unit Tests

**Purpose:** Test individual components in isolation

**Characteristics:**
- Fast (<1ms per test)
- No external dependencies
- Mock/stub collaborators
- One assertion per logical test
- Test both success and failure paths

**Coverage:** 100% of all implemented components

**Current Status:** 33 tests, 113 assertions, 100% coverage

**Example:**
```cpp
TEST(buffer_create_with_data) {
    uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Buffer buf(data, 10);
    
    TEST_ASSERT(buf.data == data);
    TEST_ASSERT_EQ(buf.size, 10);
    TEST_ASSERT_EQ(buf.data[0], 1);
    TEST_ASSERT_EQ(buf.data[9], 10);
}
```

### 2.2 User Scenario Tests

**Purpose:** Test realistic user workflows end-to-end

**Characteristics:**
- Slower (10-1000ms per test)
- May use real components (not mocks)
- Test complete workflows
- Validate user-facing behavior
- Cross-component integration

**Priority:** **HIGH** - Currently missing, must be implemented

**Planned Scenarios:**

1. **File Transfer Scenario**
   - Send file from point A to point B
   - Use real transport (loopback)
   - Use real framer (length-prefix)
   - Verify file integrity

2. **Multi-Transport Failover**
   - Attempt primary transport (fails)
   - Fall back to secondary transport (succeeds)
   - Verify message delivery

3. **End-to-End Encryption**
   - Encrypt message at sender
   - Send via transport
   - Decrypt at receiver
   - Verify plaintext matches

4. **HTTP Static File Server**
   - Start HTTP server
   - Request static file via HTTP GET
   - Verify correct response
   - Verify headers and status code

5. **Store-and-Forward Messaging**
   - Write message to file transport
   - Read message from file transport
   - Verify message integrity
   - Test offline scenarios

6. **Onion Routing (Future)**
   - Route message through multiple hops
   - Verify each hop cannot see end-to-end content
   - Verify final destination receives message

7. **P2P Mesh Communication (Future)**
   - Multiple nodes discover each other
   - Route messages across mesh
   - Handle node failures

8. **Federation Sync (Future)**
   - Two nodes synchronize data
   - Handle conflicts
   - Verify eventual consistency

### 2.3 Integration Tests

**Purpose:** Test multiple components working together

**Characteristics:**
- Medium speed (1-100ms per test)
- Test component interactions
- May use test doubles for external systems
- Validate contracts between layers

**Planned Integration Tests:**

1. **Transport + Framer**
   ```cpp
   TEST(loopback_with_length_prefix_framing) {
       // Setup
       Transport* transport = LoopbackTransport::get_instance();
       Framer* framer = LengthPrefixFramer::get_instance();
       
       // Send
       Buffer msg = create_message("Hello");
       Buffer framed;
       framer->encode(msg, framed);
       transport->send(framed);
       
       // Receive
       Buffer received;
       transport->recv(received);
       Buffer decoded;
       framer->decode(received, decoded);
       
       // Verify
       TEST_ASSERT_EQ(decoded.size, msg.size);
       TEST_ASSERT_EQ(memcmp(decoded.data, msg.data, msg.size), 0);
   }
   ```

2. **Transport + Framer + Crypto**
   - Full stack with encryption
   - Verify end-to-end integrity

3. **Service + Transport**
   - HTTP server over TCP
   - DNS cache over UDP
   - SMTP over TCP

4. **Multiple Transports**
   - Send same message via multiple transports
   - Verify all deliver correctly

### 2.4 Performance Tests

**Purpose:** Measure and benchmark performance

**Characteristics:**
- Measure throughput (messages/sec)
- Measure latency (ms per operation)
- Measure resource usage (memory, CPU)
- Compare against baseline
- Detect regressions

**Planned Benchmarks:**

1. **Transport Throughput**
   - Messages per second
   - Bytes per second
   - Various message sizes (10B, 1KB, 10KB, 100KB, 1MB)

2. **Framer Overhead**
   - Encoding latency
   - Decoding latency
   - Memory overhead

3. **Crypto Performance**
   - Encryption throughput
   - Decryption throughput
   - Key generation time

4. **End-to-End Latency**
   - Full stack latency
   - Per-component breakdown

### 2.5 Stress Tests

**Purpose:** Test behavior under extreme conditions

**Characteristics:**
- High load
- Long duration
- Resource exhaustion
- Error injection
- Recovery testing

**Planned Stress Tests:**

1. **High Message Volume**
   - Send 1M+ messages
   - Verify no memory leaks
   - Verify all messages delivered

2. **Large Messages**
   - Test maximum message sizes
   - Verify buffer handling
   - Test fragmentation

3. **Connection Churn**
   - Rapid connect/disconnect cycles
   - Verify resource cleanup

4. **Error Injection**
   - Simulate network failures
   - Simulate corrupted data
   - Verify error handling and recovery

### 2.6 Security Tests

**Purpose:** Validate security properties

**Characteristics:**
- Test attack scenarios
- Verify access controls
- Test crypto correctness
- Vulnerability scanning

**Planned Security Tests:**

1. **Buffer Overflow Protection**
   - Attempt oversized messages
   - Verify bounds checking

2. **Crypto Validation**
   - Test known-answer tests (KATs)
   - Verify randomness
   - Test key handling

3. **Authentication**
   - Test unauthorized access
   - Test credential validation

4. **Denial of Service**
   - Test resource limits
   - Test rate limiting

---

## 3. Test Infrastructure

### 3.1 Test Runner

**Current Implementation:**
- Custom test framework (zero dependencies)
- Automatic test registration
- TAP-like output format
- Exit code indicates pass/fail

**Features:**
```cpp
TEST(test_name) {
    // Test body
    TEST_ASSERT(condition);
    TEST_ASSERT_EQ(a, b);
    TEST_ASSERT_NE(a, b);
    TEST_ASSERT_STR_EQ(str1, str2);
}
```

### 3.2 Test Execution

**Build System Integration:**
```bash
# Build tests
cmake ..
make

# Run all tests
./federated_tests

# Run via CTest
ctest --output-on-failure

# Run with valgrind (memory checks)
valgrind --leak-check=full ./federated_tests
```

### 3.3 Continuous Integration

**GitHub Actions Workflow:**
- Build on Linux (Ubuntu)
- Build on macOS
- Run all tests
- Check code coverage
- Run security scans (CodeQL)

**Failure Policy:**
- Any test failure blocks merge
- Any build warning blocks merge
- Any security alert blocks merge

---

## 4. Test Organization

### 4.1 Directory Structure

```
tests/
├── test_runner.h         # Test framework
├── test_runner.cpp       # Test runner implementation
├── main.cpp              # Test executable entry point
├── core/                 # Unit tests for core primitives
│   ├── test_buffer.cpp
│   ├── test_packet.cpp
│   ├── test_error.cpp
│   └── test_registry.cpp
├── transport/            # Unit tests for transports
│   ├── test_loopback.cpp
│   ├── test_file.cpp
│   ├── test_tcp.cpp      # TODO
│   └── test_udp.cpp      # TODO
├── framer/               # Unit tests for framers
│   ├── test_raw.cpp
│   ├── test_length_prefix.cpp
│   └── test_crc.cpp      # TODO
├── crypto/               # Unit tests for crypto
│   ├── test_none.cpp
│   └── test_xor.cpp      # TODO
├── scenarios/            # User scenario tests (NEW)
│   ├── scenario_file_transfer.cpp
│   ├── scenario_http_server.cpp
│   └── scenario_end_to_end_crypto.cpp
├── integration/          # Integration tests (NEW)
│   ├── test_transport_framer.cpp
│   └── test_full_stack.cpp
└── performance/          # Performance tests (NEW)
    ├── bench_transport.cpp
    └── bench_crypto.cpp
```

### 4.2 Test Naming Conventions

**Unit Tests:**
- Format: `test_<component>_<scenario>.cpp`
- Example: `test_buffer_create_empty.cpp`

**User Scenario Tests:**
- Format: `scenario_<workflow>.cpp`
- Example: `scenario_file_transfer.cpp`

**Integration Tests:**
- Format: `test_<component1>_<component2>.cpp`
- Example: `test_transport_framer.cpp`

**Performance Tests:**
- Format: `bench_<component>.cpp`
- Example: `bench_crypto.cpp`

---

## 5. Current Test Coverage

### 5.1 Implemented Tests

| Component | Tests | Assertions | Coverage |
|-----------|-------|------------|----------|
| Buffer | 3 | 9 | 100% |
| Packet | 2 | 6 | 100% |
| Error | 3 | 10 | 100% |
| Registry | 4 | 16 | 100% |
| Loopback Transport | 5 | 15 | 100% |
| File Transport | 3 | 15 | 100% |
| Raw Framer | 4 | 13 | 100% |
| Length-Prefix Framer | 5 | 17 | 100% |
| None Crypto | 4 | 12 | 100% |
| **Total** | **33** | **113** | **100%** |

### 5.2 Test Gap Analysis

**Missing Tests:**

1. **User Scenario Tests** - 0 of 8 planned (CRITICAL)
2. **Integration Tests** - 0 of 4 planned (HIGH)
3. **Performance Tests** - 0 of 4 planned (MEDIUM)
4. **Stress Tests** - 0 of 4 planned (MEDIUM)
5. **Security Tests** - 0 of 4 planned (HIGH)

---

## 6. Immediate Test Plan (Sprint 1)

### 6.1 Create User Scenario Tests

**Priority:** CRITICAL

**Tasks:**
1. Create `tests/scenarios/` directory
2. Implement scenario test framework
3. Implement 3 initial scenarios:
   - File transfer with framing
   - Multi-transport communication
   - Store-and-forward messaging

**Acceptance Criteria:**
- At least 3 scenario tests passing
- Scenarios test real workflows
- Scenarios use real components (not mocks)
- Scenarios validate end-to-end behavior

### 6.2 Add to Build System

**Tasks:**
1. Update CMakeLists.txt to include scenario tests
2. Create separate test executable or integrate with existing
3. Add scenario tests to CI pipeline

### 6.3 Documentation

**Tasks:**
1. Document scenario test writing guidelines
2. Add examples to test plan
3. Update STATUS.md with scenario test metrics

---

## 7. Test Writing Guidelines

### 7.1 Unit Test Guidelines

**DO:**
- Test one thing per test
- Use descriptive test names
- Test both success and failure paths
- Test boundary conditions
- Make tests independent (no shared state)
- Make tests deterministic (no randomness)
- Make tests fast (<1ms ideal)

**DON'T:**
- Test implementation details
- Use real external dependencies
- Share state between tests
- Make tests flaky
- Skip error testing

### 7.2 Scenario Test Guidelines

**DO:**
- Test realistic user workflows
- Use real components where possible
- Test end-to-end behavior
- Validate user-visible outcomes
- Include setup and teardown
- Document the scenario being tested

**DON'T:**
- Test internal implementation
- Mock too many components
- Make scenarios too complex
- Ignore cleanup

### 7.3 Test Code Quality

**Standards:**
- Same code quality as production code
- Clear, readable code
- Minimal duplication (use helpers)
- Good error messages
- Proper resource cleanup

---

## 8. Test Metrics and Goals

### 8.1 Current Metrics

- **Test Count:** 33
- **Assertion Count:** 113
- **Code Coverage:** 100%
- **Test Runtime:** <0.1s
- **Failure Rate:** 0%

### 8.2 Target Metrics (End of Sprint 1)

- **Test Count:** 50+ (add 17+ scenario/integration tests)
- **Assertion Count:** 200+
- **Code Coverage:** 100% (maintain)
- **Test Runtime:** <1s (allow for scenario tests)
- **Failure Rate:** 0% (maintain)

### 8.3 Long-Term Goals

- **Test Count:** 200+ (comprehensive coverage)
- **Code Coverage:** 100% (always)
- **Test Runtime:** <5s (all tests)
- **Performance Baselines:** Established for all components
- **Continuous Monitoring:** Automated performance regression detection

---

## 9. Test Maintenance

### 9.1 When to Update Tests

**Always update tests when:**
- Adding new features
- Changing existing behavior
- Fixing bugs
- Refactoring code

**Test Update Workflow:**
1. Write/update test first (test-first)
2. Test fails (red)
3. Implement feature/fix
4. Test passes (green)
5. Refactor if needed
6. All tests still pass

### 9.2 Test Review Process

**All test code must be reviewed for:**
- Correctness of assertions
- Completeness of coverage
- Code quality
- Performance impact
- Flakiness potential

---

## 10. Appendix

### 10.1 Test Framework Reference

```cpp
// Assertion macros
TEST_ASSERT(condition)
TEST_ASSERT_EQ(a, b)
TEST_ASSERT_NE(a, b)
TEST_ASSERT_STR_EQ(str1, str2)

// Test registration
TEST(test_name) {
    // Test body
}

// Running tests
int main() {
    return federated::test::run_all_tests();
}
```

### 10.2 Example Test Template

```cpp
#include "../test_runner.h"
#include "../../include/federated/component/component.h"

using namespace federated::component;

TEST(component_basic_functionality) {
    // Arrange
    Component c;
    
    // Act
    auto result = c.do_something();
    
    // Assert
    TEST_ASSERT(result.success);
    TEST_ASSERT_EQ(result.value, expected_value);
}

TEST(component_error_handling) {
    // Arrange
    Component c;
    
    // Act
    auto result = c.do_invalid_operation();
    
    // Assert
    TEST_ASSERT(!result.success);
    TEST_ASSERT_EQ(result.error, ERR_INVALID);
}
```

---

**Document Owner:** Federated Test Team  
**Review Frequency:** After each sprint  
**Next Review:** End of Sprint 1

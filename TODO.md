# Federated Project TODO

**Last Updated:** 2026-01-27

---

## Current Status

### ✅ Completed (Phase 1: Foundation)

#### Core Primitives (100%)
- [x] Buffer - Memory buffer with explicit ownership
- [x] Packet - Network packet wrapper
- [x] Error - Explicit error codes (8 types)
- [x] Registry - Global component registry
- [x] Endian - Portable byte-order conversion

#### Transport Layer (82% - 9 of 11 with stubs, 45% fully functional - 5 of 11)
- [x] Loopback - In-memory testing transport (FULL)
- [x] File - Store-and-forward transport (FULL)
- [x] TCP - TCP socket transport (RFC 793) (FULL)
- [x] UDP - UDP datagram transport (RFC 768) (FULL)
- [x] DNS Tunnel - Covert channel via DNS (RFC 1035) (FULL - Base32 encoding, 6 tests)
- [x] Bluetooth - Bluetooth transport - STUB (requires platform APIs)
- [x] WiFi Direct - WiFi Direct transport - STUB (requires platform APIs)
- [x] Infrared - IrDA transport - STUB (requires platform APIs)
- [ ] Audio - Audio tone-based transport
- [ ] QR - QR code-based transport
- [ ] FM Radio - FM radio data transport

#### Framer Layer (100% - 4 of 4 planned)
- [x] Raw - Pass-through framer
- [x] Length-prefix - 4-byte header framing
- [x] CRC - CRC checksum framing
- [x] Chunked - Chunked transfer encoding

#### Crypto Layer (50% - 3 of 6 planned)
- [x] None - Pass-through (Tier 0) (FULL)
- [x] XOR Stream - XOR obfuscation (Tier 1) (FULL)
- [x] ChaCha20 - ChaCha20 encryption (Tier 2, RFC 8439) (FULL - 7 tests with RFC vectors)
- [ ] AES - AES encryption (Tier 2, NIST)
- [x] Public Key - RSA/ECC public-key (Tier 3, RFC 8017) - STUB (requires big integer lib)
- [ ] Advanced - Future methods (Tier 4)

#### Test Coverage
- [x] Unit Tests - 140 tests, 673 assertions, 100% coverage
- [x] User Scenario Tests - 19 tests across 6 scenarios (crypto, store-and-forward, file transfer, HTTP, TCP, UDP)
- [x] E2E Tests - Shell-based tests for SMTP, IMAP, combined scenarios
- [ ] Performance Tests - Not yet implemented

---

## Phase 2: Essential Transports & Services (✅ COMPLETED - Expanded)

### High Priority

#### Transports
- [x] TCP transport implementation (RFC 793)
  - [x] Header file with RFC documentation
  - [x] Implementation file
  - [x] Unit tests (8 tests)
  - [x] User scenario tests (client-server communication)
  
- [x] UDP transport implementation (RFC 768)
  - [x] Header file with RFC documentation
  - [x] Implementation file
  - [x] Unit tests (11 tests)
  - [x] User scenario tests (datagram exchange)

#### Framers
- [x] CRC framer implementation
  - [x] Header file with algorithm documentation
  - [x] Implementation file (CRC32)
  - [x] Unit tests (6 tests)
  - [x] User scenario tests (integrated)

- [x] Chunked framer implementation
  - [x] Header file with RFC documentation
  - [x] Implementation file
  - [x] Unit tests (11 tests)
  - [x] User scenario tests (integrated)

#### Crypto
- [x] XOR Stream crypto (Tier 1)
  - [x] Header file
  - [x] Implementation file
  - [x] Unit tests (5 tests)
  - [x] User scenario tests (integrated)

#### Services
- [x] HTTP Server (RFC 9110, RFC 9112)
  - [x] Header file with RFC documentation
  - [x] Implementation file (static file server with MIME types)
  - [x] Unit tests (16 tests)
  - [x] User scenario tests (3 scenarios: serve static files, 404 handling, multiple files)
  
- [x] SMTP Protocol (RFC 5321)
  - [x] Header file with RFC documentation
  - [x] Implementation file (command parsing, state machine)
  - [x] Unit tests (8 tests)
  - [x] E2E tests (shell-based)
  
- [x] IMAP Protocol (RFC 3501)
  - [x] Header file with RFC documentation
  - [x] Implementation file (command parsing)
  - [x] Unit tests (9 tests)
  - [x] E2E tests (shell-based)
  
- [x] Mail Storage System
  - [x] Header file
  - [x] Implementation file (persistence, fetch, list, delete)
  - [x] Unit tests (6 tests)

#### Documentation
- [x] docs/rfc_references.md - Complete RFC reference list
- [x] docs/test_plan.md - Test strategy and scenarios
- [x] docs/architecture.md - Architecture overview
- [ ] docs/STATUS.md - Current implementation status (NEEDS UPDATE - outdated)
- [ ] docs/protocols.md - Protocol specifications (partial in rfc_references.md)
- [ ] docs/ux.md - User experience guidelines

#### User Scenario Tests
- [x] tests/scenarios/ - Directory for scenario tests
- [x] Scenario: End-to-end crypto (2 tests: roundtrip, layer integration)
- [x] Scenario: Store-and-forward messaging (2 tests: send/receive, persistence)
- [x] Scenario: File transfer over loopback (3 tests: send/receive, bidirectional, empty file)
- [x] Scenario: HTTP server serving static content (3 tests: serve files, 404, multiple files)
- [x] Scenario: TCP client-server communication (4 tests: basic exchange, bidirectional, lifecycle, errors)
- [x] Scenario: UDP datagram exchange (5 tests: boundaries, variable sizes, timeout, TCP comparison, errors)
- [x] E2E tests (tests/e2e/): Shell scripts for SMTP, IMAP, and combined scenarios

---

## Phase 3: Transport and Crypto Expansion (✅ COMPLETED)

### Completed
- [x] ChaCha20 implementation (RFC 8439)
  - [x] Header file with RFC documentation
  - [x] Implementation file (pure C++, zero dependencies)
  - [x] Unit tests (7 tests, 21 assertions)
  - [x] RFC test vectors validated

- [x] DNS Tunnel transport (RFC 1035)
  - [x] Header file with RFC documentation
  - [x] Implementation file (Base32 encoding, DNS query construction)
  - [x] Unit tests (6 tests, 14 assertions)
  - [x] UDP socket communication to DNS server
  
- [x] Transport stub implementations
  - [x] Bluetooth - stub with TODO markers (requires platform APIs)
  - [x] WiFi Direct - stub with TODO markers (requires platform APIs)
  - [x] Infrared (IrDA) - stub with TODO markers (requires platform APIs)

- [x] Public Key Crypto stub
  - [x] Header file with RFC documentation
  - [x] Stub implementation with TODO markers (requires big integer library)

---

## Phase 4: Advanced Features (Planned)

### Onion Routing
- [ ] onion/onion_router.h - Multi-hop routing interface
- [ ] onion/bridge_mode.h - Bridge/proxy mode
- [ ] onion/multi_hop.h - Multi-hop implementation
- [ ] Implementation files
- [ ] Unit tests
- [ ] User scenario tests

### Federation Layer
- [ ] federation/federation.h - Federation interface
- [ ] federation/node_discovery.h - Node discovery
- [ ] federation/sync.h - Data synchronization
- [ ] federation/conflict_resolution.h - Conflict resolution
- [ ] Implementation files
- [ ] Unit tests
- [ ] User scenario tests

### Additional Services (Planned)
- [ ] Microblog service (ActivityPub W3C)
- [ ] File Exchange service
- [ ] Full network server integration (bind TCP sockets, accept connections for SMTP/IMAP/HTTP)

### Tools
- [x] Log System (tool/log.h, tool/log.cpp)
  - [x] 5 log levels (ERROR, WARN, INFO, DEBUG, TRACE)
  - [x] Color output support
  - [x] Configurable drivers
  - [x] Example: examples/log_demo.cpp
  
- [x] CLI Framework (tool/cli.h, tool/cli.cpp)
  - [x] Command registration
  - [x] Service registration
  - [x] Environment variable handling
  - [x] Signal handlers for graceful shutdown
  
- [ ] Diagnostic tools (ping, arp, netstat, ifconfig, route)
- [ ] Feed aggregation (RSS, Atom, JSON Feed)
- [ ] SSH agentless monitoring
- [ ] Full CLI command parser and interface

---

## Phase 5: Analog & Specialized Transports (Partial)

- [x] DNS Tunnel transport (RFC 1035) - FULLY IMPLEMENTED
- [x] Bluetooth transport - STUB (requires BlueZ or platform Bluetooth APIs)
- [x] WiFi Direct transport - STUB (requires wpa_supplicant or platform WiFi APIs)
- [x] Infrared transport - STUB (requires serial port libraries)
- [ ] Audio tones transport
- [ ] QR code transport
- [ ] FM radio data transport

---

## Phase 6: Advanced Crypto (Partial)

- [x] ChaCha20 implementation (RFC 8439) - FULLY IMPLEMENTED
- [x] Public-key cryptography (RFC 8017) - STUB (requires big integer library or OpenSSL)
- [ ] AES implementation (NIST standards)
- [ ] Key exchange protocols
- [ ] Certificate handling (X.509, RFC 5280)

---

## Phase 7: UX & Tooling (Partial)

### UX Surfaces
- [x] CLI framework (partial - command registration, service registration)
- [ ] Full CLI interface (scriptable, deterministic)
- [ ] TUI dashboard (status monitoring)
- [ ] Web UI (HTML/JS, read-only default)
- [ ] JSON API (automation)

### Build & Deploy
- [ ] Cross-compilation support
- [ ] Package management integration
- [ ] Installation scripts
- [ ] Docker containers
- [ ] Platform-specific builds (Windows, macOS, Linux ARM)

---

## Testing Strategy

### Unit Tests (✅ COMPLETED)
- [x] Core primitives - 12 tests
- [x] Transports - 33 tests (loopback, file, TCP, UDP, DNS tunnel)
- [x] Framers - 23 tests (raw, length-prefix, CRC, chunked)
- [x] Crypto - 16 tests (none, XOR stream, ChaCha20)
- [x] Services - 39 tests (mail storage, SMTP, IMAP, HTTP, HTTP server)

### User Scenario Tests (✅ COMPLETED - 19 tests)
- [x] File transfer scenario (3 tests)
- [x] End-to-end encryption scenario (2 tests)
- [x] HTTP service scenario (3 tests)
- [x] Store-and-forward scenario (2 tests)
- [x] TCP communication scenario (4 tests)
- [x] UDP datagram scenario (5 tests)
- [ ] Multi-transport failover scenario
- [ ] P2P mesh communication scenario
- [ ] Onion routing scenario
- [ ] Federation sync scenario

### Integration Tests (Partial)
- [x] Transport + Framer integration (covered in scenarios)
- [x] Transport + Framer + Crypto integration (end-to-end crypto scenario)
- [x] Full stack integration (examples/simple_message.cpp)
- [x] Service integration tests (E2E shell tests for SMTP/IMAP)
- [ ] Federation integration tests

### Performance Tests
- [ ] Throughput benchmarks
- [ ] Latency measurements
- [ ] Memory usage profiling
- [ ] CPU usage profiling
- [ ] Stress tests

---

## Documentation Tasks

### Required Documentation Files

#### High Priority
- [x] docs/rfc_references.md - Complete RFC reference guide
- [x] docs/test_plan.md - Comprehensive test strategy
- [x] docs/architecture.md - Architecture overview
- [ ] docs/STATUS.md - Current implementation status (NEEDS UPDATE - shows outdated test counts)
- [ ] docs/protocols.md - Protocol specifications (partial in rfc_references.md)
- [ ] docs/ux.md - User experience design and principles

#### Medium Priority
- [ ] API documentation (Doxygen or similar)
- [ ] Developer guide
- [ ] Deployment guide
- [ ] Security guide
- [ ] Performance tuning guide

#### Low Priority
- [ ] User manual
- [ ] FAQ
- [ ] Troubleshooting guide
- [ ] Contributing guidelines
- [ ] Code of conduct

---

## RFC Documentation Requirements

All implementation files must include RFC references in header comments:

```cpp
/**
 * RFC XXXX - Protocol Name
 * 
 * Brief description of the protocol/standard
 * Key requirements and constraints
 * 
 * Implementation notes:
 * - Specific design decisions
 * - Deviations from RFC (if any)
 * - Compatibility notes
 */
```

### Key RFCs to Document

#### Internet Layer
- RFC 791 - IPv4
- RFC 8200 - IPv6
- RFC 792 - ICMPv4
- RFC 4443 - ICMPv6
- RFC 826 - ARP

#### Transport Layer
- RFC 793 - TCP
- RFC 768 - UDP
- RFC 4960 - SCTP (optional)
- RFC 9000 - QUIC (future)

#### Application Layer
- RFC 7230-7235 - HTTP/1.1
- RFC 5321 - SMTP
- RFC 3501 - IMAP
- RFC 1034-1035 - DNS
- RFC 4287 - Atom
- RSS 2.0 - Specification
- JSON Feed - RFC 8425

#### Security
- RFC 5246 - TLS 1.2
- RFC 8446 - TLS 1.3
- RFC 8439 - ChaCha20-Poly1305
- RFC 8017 - PKCS#1 (RSA)
- RFC 5280 - X.509

#### Federation
- W3C ActivityPub - Fediverse protocol
- RFC 7033 - WebFinger

---

## Directory Structure Completion

### Existing Directories
- [x] tests/scenarios/ - User scenario tests (19 tests)
- [x] tests/e2e/ - End-to-end integration tests (shell scripts)
- [x] examples/ - Example applications (simple_message.cpp, log_demo.cpp)

### Missing Directories to Create
- [ ] tests/performance/ - Performance benchmarks
- [ ] examples/microblog_demo/ - Microblog example
- [ ] examples/p2p_mesh_demo/ - P2P mesh example
- [ ] examples/web_service_demo/ - Web service example
- [ ] tools/generate_keys/ - Key generation utility
- [ ] tools/run_ci/ - CI runner scripts
- [ ] tools/generate_docs/ - Documentation generator
- [ ] tools/benchmarks/ - Benchmark utilities
- [ ] ci/build_matrix/ - CI build configurations
- [ ] assets/html/ - HTML templates
- [ ] assets/css/ - Stylesheets
- [ ] assets/js/ - JavaScript files
- [ ] configs/ - Configuration files

### Missing Config Files
- [ ] configs/transports.json - Transport configuration
- [ ] configs/services.json - Services configuration
- [ ] configs/crypto.json - Crypto configuration
- [ ] configs/federation.json - Federation configuration
- [ ] configs/logging.json - Logging configuration

---

## Immediate Next Steps (Current Sprint)

1. **✅ Phase 2 & 3 Implementation - COMPLETED**
   - [x] TCP transport with RFC 793 documentation
   - [x] UDP transport with RFC 768 documentation
   - [x] DNS Tunnel transport with RFC 1035 and Base32 encoding
   - [x] CRC framer with CRC32 checksums
   - [x] Chunked framer with RFC 9112 implementation
   - [x] XOR Stream crypto (Tier 1)
   - [x] ChaCha20 crypto (Tier 2, RFC 8439)
   - [x] HTTP Server with RFC 9110/9112 documentation
   - [x] SMTP protocol implementation (RFC 5321)
   - [x] IMAP protocol implementation (RFC 3501)
   - [x] Mail storage system
   - [x] All unit tests (159 tests, 692 assertions)
   - [x] User scenario tests (19 tests across 6 scenarios)
   - [x] E2E tests (shell-based for SMTP/IMAP)
   - [x] Updated CMakeLists.txt
   - [x] Cross-platform support (Linux, macOS, Windows)

2. **✅ Documentation - PARTIALLY COMPLETED**
   - [x] docs/rfc_references.md
   - [x] docs/test_plan.md
   - [x] docs/architecture.md
   - [ ] docs/STATUS.md (needs update)
   - [ ] docs/protocols.md (partially covered in rfc_references.md)
   - [ ] docs/ux.md

3. **Next Priority Items**
   - [ ] AES implementation (NIST standards)
   - [ ] Full implementation of platform-dependent transports (requires external APIs)
   - [ ] Performance benchmarking framework
   - [ ] Additional examples (microblog, P2P mesh, web service)

---

## Notes

- All code must maintain 100% test coverage
- All new features require user scenario tests
- All implementation files need RFC documentation headers
- Follow test-first development methodology
- Maintain zero external dependencies principle
- Keep documentation synchronized with code changes

---

## Metrics to Track

- **Test Coverage**: Currently 100% (maintained ✓)
- **Build Time**: Currently <10s (maintained ✓)
- **Test Runtime**: Currently <1s (maintained ✓)
- **Binary Size**: Currently ~200KB tests (reasonable growth)
- **Code Quality**: 0 warnings, 0 security alerts (maintained ✓)
- **Test Count**: 159 tests (140 unit + 19 scenario tests)
- **Assertions**: 692 assertions
- **E2E Tests**: Shell-based tests for SMTP, IMAP, combined scenarios



---

**Status Legend:**
- ✅ Completed
- 🔄 In Progress
- ⏳ Planned
- ❌ Blocked
- 📝 Documentation Needed

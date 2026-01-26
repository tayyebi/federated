# Federated Project TODO

**Last Updated:** 2026-01-26

---

## Current Status

### ✅ Completed (Phase 1: Foundation)

#### Core Primitives (100%)
- [x] Buffer - Memory buffer with explicit ownership
- [x] Packet - Network packet wrapper
- [x] Error - Explicit error codes (8 types)
- [x] Registry - Global component registry
- [x] Endian - Portable byte-order conversion

#### Transport Layer (40% - 4 of 10 planned)
- [x] Loopback - In-memory testing transport
- [x] File - Store-and-forward transport
- [x] TCP - TCP socket transport (RFC 793)
- [x] UDP - UDP datagram transport (RFC 768)
- [ ] DNS Tunnel - Covert channel via DNS (RFC 1035)
- [ ] Bluetooth - Bluetooth transport
- [ ] WiFi Direct - WiFi Direct transport
- [ ] Audio - Audio tone-based transport
- [ ] QR - QR code-based transport
- [ ] Infrared - IrDA transport
- [ ] FM Radio - FM radio data transport

#### Framer Layer (100% - 4 of 4 planned)
- [x] Raw - Pass-through framer
- [x] Length-prefix - 4-byte header framing
- [x] CRC - CRC checksum framing
- [x] Chunked - Chunked transfer encoding

#### Crypto Layer (33% - 2 of 6 planned)
- [x] None - Pass-through (Tier 0)
- [x] XOR Stream - XOR obfuscation (Tier 1)
- [ ] ChaCha20 - ChaCha20 encryption (Tier 2, RFC 8439)
- [ ] AES - AES encryption (Tier 2, NIST)
- [ ] Public Key - RSA/ECC public-key (Tier 3, RFC 8017)
- [ ] Advanced - Future methods (Tier 4)

#### Test Coverage
- [x] Unit Tests - 146 tests, 657 assertions, 100% coverage
- [x] User Scenario Tests - Implemented (11 scenarios)
- [ ] Integration Tests - Not yet implemented
- [ ] Performance Tests - Not yet implemented

---

## Phase 2: Essential Transports & Services (✅ COMPLETED)

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
  - [x] Implementation file (basic static server)
  - [x] Unit tests (17 tests)
  - [x] User scenario tests (serve static files)

#### Documentation
- [ ] docs/protocols.md - RFC mapping and protocol details
- [ ] docs/ux.md - User experience guidelines
- [ ] docs/test_plan.md - Test strategy and scenarios
- [ ] docs/rfc_references.md - Complete RFC reference list

#### User Scenario Tests
- [x] tests/scenarios/ - Directory for scenario tests
- [x] Scenario: Send file over loopback with framing
- [x] Scenario: Multi-hop message routing
- [x] Scenario: HTTP server serving static content
- [x] Scenario: Store-and-forward messaging via file transport
- [x] Scenario: Encrypted communication end-to-end
- [x] Scenario: TCP client-server communication
- [x] Scenario: UDP datagram exchange

---

## Phase 3: Advanced Features (Planned)

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

### Additional Services
- [ ] SMTP service (RFC 5321)
- [ ] IMAP service (RFC 3501)
- [ ] Microblog service (ActivityPub W3C)
- [ ] File Exchange service

### Tools
- [ ] Diagnostic tools (ping, arp, netstat, ifconfig, route)
- [ ] Feed aggregation (RSS, Atom, JSON Feed)
- [ ] SSH agentless monitoring
- [ ] CLI interface and command parser

---

## Phase 4: Analog & Specialized Transports (Planned)

- [ ] DNS Tunnel transport (RFC 1035 extensions)
- [ ] Bluetooth transport
- [ ] WiFi Direct transport
- [ ] Audio tones transport
- [ ] QR code transport
- [ ] Infrared transport
- [ ] FM radio data transport

---

## Phase 5: Advanced Crypto (Planned)

- [ ] ChaCha20 implementation (RFC 8439)
- [ ] AES implementation (NIST standards)
- [ ] Public-key cryptography (RFC 8017)
- [ ] Key exchange protocols
- [ ] Certificate handling (X.509, RFC 5280)

---

## Phase 6: UX & Tooling (Planned)

### UX Surfaces
- [ ] CLI interface (scriptable, deterministic)
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

### Unit Tests
- [x] Core primitives - 12 tests
- [x] Transports (loopback, file) - 8 tests
- [x] Framers (raw, length-prefix) - 9 tests
- [x] Crypto (none) - 4 tests
- [ ] Additional transports (TCP, UDP, etc.) - TBD
- [ ] Additional framers (CRC, chunked) - TBD
- [ ] Additional crypto (XOR, ChaCha20, AES) - TBD

### User Scenario Tests (New)
- [ ] File transfer scenario
- [ ] Multi-transport failover scenario
- [ ] End-to-end encryption scenario
- [ ] HTTP service scenario
- [ ] P2P mesh communication scenario
- [ ] Store-and-forward scenario
- [ ] Onion routing scenario
- [ ] Federation sync scenario

### Integration Tests
- [ ] Transport + Framer integration
- [ ] Transport + Framer + Crypto integration
- [ ] Full stack integration (all layers)
- [ ] Service integration tests
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
- [ ] docs/protocols.md - Protocol specifications and RFC mappings
- [ ] docs/ux.md - User experience design and principles
- [ ] docs/test_plan.md - Comprehensive test strategy
- [ ] docs/rfc_references.md - Complete RFC reference guide

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

### Missing Directories to Create
- [ ] tests/scenarios/ - User scenario tests
- [ ] tests/integration/ - Integration tests
- [ ] tests/performance/ - Performance benchmarks
- [ ] examples/send_file/ - File transfer example
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

## Immediate Next Steps (Sprint 2)

1. **✅ Phase 2 Implementation - COMPLETED**
   - [x] TCP transport with RFC 793 documentation
   - [x] UDP transport with RFC 768 documentation
   - [x] CRC framer with CRC32 checksums
   - [x] Chunked framer with RFC 9112 implementation
   - [x] XOR Stream crypto (Tier 1)
   - [x] HTTP Server with RFC 9110/9112 documentation
   - [x] All unit tests (146 tests, 657 assertions)
   - [x] User scenario tests (11 scenarios)
   - [x] Updated CMakeLists.txt
   - [x] Cross-platform support (Linux, macOS, Windows)

2. **Create Missing Documentation** (Next Sprint)
   - [ ] docs/protocols.md
   - [ ] docs/ux.md
   - [ ] docs/test_plan.md
   - [ ] docs/rfc_references.md

3. **Implement Advanced Crypto** (Next Sprint)
   - [ ] ChaCha20 implementation (RFC 8439)
   - [ ] AES implementation (NIST standards)

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
- **Test Count**: 146 tests (up from 77)
- **Assertions**: 657 assertions (up from 313)

---

**Status Legend:**
- ✅ Completed
- 🔄 In Progress
- ⏳ Planned
- ❌ Blocked
- 📝 Documentation Needed

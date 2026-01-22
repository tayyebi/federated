# Federated Project TODO

**Last Updated:** 2026-01-22

---

## Current Status

### ✅ Completed (Phase 1: Foundation)

#### Core Primitives (100%)
- [x] Buffer - Memory buffer with explicit ownership
- [x] Packet - Network packet wrapper
- [x] Error - Explicit error codes (8 types)
- [x] Registry - Global component registry
- [x] Endian - Portable byte-order conversion

#### Transport Layer (20% - 2 of 10 planned)
- [x] Loopback - In-memory testing transport
- [x] File - Store-and-forward transport
- [ ] TCP - TCP socket transport (RFC 793)
- [ ] UDP - UDP datagram transport (RFC 768)
- [ ] DNS Tunnel - Covert channel via DNS (RFC 1035)
- [ ] Bluetooth - Bluetooth transport
- [ ] WiFi Direct - WiFi Direct transport
- [ ] Audio - Audio tone-based transport
- [ ] QR - QR code-based transport
- [ ] Infrared - IrDA transport
- [ ] FM Radio - FM radio data transport

#### Framer Layer (50% - 2 of 4 planned)
- [x] Raw - Pass-through framer
- [x] Length-prefix - 4-byte header framing
- [ ] CRC - CRC checksum framing
- [ ] Chunked - Chunked transfer encoding

#### Crypto Layer (17% - 1 of 6 planned)
- [x] None - Pass-through (Tier 0)
- [ ] XOR Stream - XOR obfuscation (Tier 1)
- [ ] ChaCha20 - ChaCha20 encryption (Tier 2, RFC 8439)
- [ ] AES - AES encryption (Tier 2, NIST)
- [ ] Public Key - RSA/ECC public-key (Tier 3, RFC 8017)
- [ ] Advanced - Future methods (Tier 4)

#### Test Coverage
- [x] Unit Tests - 33 tests, 113 assertions, 100% coverage
- [ ] User Scenario Tests - Not yet implemented
- [ ] Integration Tests - Not yet implemented
- [ ] Performance Tests - Not yet implemented

---

## Phase 2: Essential Transports & Services (In Progress)

### High Priority

#### Transports
- [ ] TCP transport implementation (RFC 793)
  - [ ] Header file with RFC documentation
  - [ ] Implementation file
  - [ ] Unit tests
  - [ ] User scenario tests (client-server communication)
  
- [ ] UDP transport implementation (RFC 768)
  - [ ] Header file with RFC documentation
  - [ ] Implementation file
  - [ ] Unit tests
  - [ ] User scenario tests (datagram exchange)

#### Framers
- [ ] CRC framer implementation
  - [ ] Header file with algorithm documentation
  - [ ] Implementation file (CRC32)
  - [ ] Unit tests
  - [ ] User scenario tests

- [ ] Chunked framer implementation
  - [ ] Header file with RFC documentation
  - [ ] Implementation file
  - [ ] Unit tests
  - [ ] User scenario tests

#### Crypto
- [ ] XOR Stream crypto (Tier 1)
  - [ ] Header file
  - [ ] Implementation file
  - [ ] Unit tests
  - [ ] User scenario tests

#### Services
- [ ] HTTP Server (RFC 7230-7235)
  - [ ] Header file with RFC documentation
  - [ ] Implementation file (basic static server)
  - [ ] Unit tests
  - [ ] User scenario tests (serve static files)

- [ ] DNS Cache (RFC 1034-1035)
  - [ ] Header file with RFC documentation
  - [ ] Implementation file
  - [ ] Unit tests
  - [ ] User scenario tests (cache queries)

#### Documentation
- [ ] docs/protocols.md - RFC mapping and protocol details
- [ ] docs/ux.md - User experience guidelines
- [ ] docs/test_plan.md - Test strategy and scenarios
- [ ] docs/rfc_references.md - Complete RFC reference list

#### User Scenario Tests
- [ ] tests/scenarios/ - Directory for scenario tests
- [ ] Scenario: Send file over loopback with framing
- [ ] Scenario: Multi-hop message routing
- [ ] Scenario: HTTP server serving static content
- [ ] Scenario: Store-and-forward messaging via file transport
- [ ] Scenario: Encrypted communication end-to-end

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

## Immediate Next Steps (Sprint 1)

1. **Create User Scenario Tests** (This Sprint)
   - [ ] Create tests/scenarios/ directory
   - [ ] Implement scenario: File transfer with framing
   - [ ] Implement scenario: Multi-transport communication
   - [ ] Implement scenario: End-to-end with crypto
   - [ ] Add scenario tests to CMakeLists.txt

2. **Create Missing Documentation** (This Sprint)
   - [ ] docs/protocols.md
   - [ ] docs/ux.md
   - [ ] docs/test_plan.md
   - [ ] docs/rfc_references.md

3. **Implement TCP Transport** (Next Sprint)
   - [ ] include/federated/transport/tcp.h with RFC 793 documentation
   - [ ] src/transport/tcp.cpp
   - [ ] tests/transport/test_tcp.cpp
   - [ ] tests/scenarios/tcp_client_server.cpp

4. **Implement UDP Transport** (Next Sprint)
   - [ ] include/federated/transport/udp.h with RFC 768 documentation
   - [ ] src/transport/udp.cpp
   - [ ] tests/transport/test_udp.cpp
   - [ ] tests/scenarios/udp_datagram.cpp

5. **Implement CRC Framer** (Next Sprint)
   - [ ] include/federated/framer/crc.h
   - [ ] src/framer/crc.cpp
   - [ ] tests/framer/test_crc.cpp

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

- **Test Coverage**: Currently 100% (must maintain)
- **Build Time**: Currently <5s (keep under 10s)
- **Test Runtime**: Currently <0.1s (keep under 1s)
- **Binary Size**: Currently 86KB tests (monitor growth)
- **Code Quality**: 0 warnings, 0 security alerts (must maintain)

---

**Status Legend:**
- ✅ Completed
- 🔄 In Progress
- ⏳ Planned
- ❌ Blocked
- 📝 Documentation Needed

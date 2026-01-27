# Phase 4 Roadmap: Advanced Features and Platform Integration

**Status:** PLANNED  
**Target:** Q1 2026  
**Last Updated:** 2026-01-27

---

## Executive Summary

Phase 4 focuses on implementing advanced cryptography (AES), completing critical covert transports (DNS Tunnel), and enabling platform-specific transport implementations (Bluetooth, WiFi Direct, Infrared). This phase addresses the remaining stub implementations while maintaining the project's core principles of zero dependencies and test-first development.

**Key Deliverables:**
- ✅ AES-128/256 encryption (CBC and GCM modes)
- ✅ DNS Tunnel fully operational with rate limiting
- ✅ Platform abstraction layer for cross-platform transports
- ✅ At least one platform-specific transport (Bluetooth on Linux)

---

## Priority 1: HIGH - Critical for Core Functionality

### Task 1.1: Implement AES Encryption (Tier 2 Crypto)

**Rationale:** Core security feature that completes the cryptography layer for secure communications.

**Current Status:** Not implemented (currently have None, XOR, ChaCha20)

**RFC Compliance:**
- NIST FIPS 197 - Advanced Encryption Standard (AES)
- RFC 3602 - The AES-CBC Cipher Algorithm and Its Use with IPsec

**Implementation Requirements:**

1. **Core AES Algorithm**
   - 128-bit and 256-bit key support
   - S-Box and inverse S-Box lookup tables
   - MixColumns and InvMixColumns operations
   - Key expansion algorithm
   - Constant-time operations (side-channel resistance)

2. **Cipher Modes**
   - **CBC Mode** (Cipher Block Chaining)
     - IV generation and handling
     - PKCS#7 padding
     - Block chaining implementation
   - **GCM Mode** (Galois/Counter Mode) - *Optional for Phase 4*
     - Authenticated encryption
     - GHASH computation
     - Counter mode encryption

3. **Interface Design**
   ```cpp
   // include/federated/crypto/aes.h
   namespace federated {
   namespace crypto {
   
   /**
    * AES Encryption (Tier 2)
    * 
    * NIST FIPS 197 - Advanced Encryption Standard
    * RFC 3602 - AES-CBC Cipher Algorithm
    * 
    * Supported key sizes: 128-bit, 256-bit
    * Supported modes: CBC (GCM optional)
    * 
    * Key format (for CBC):
    * - Bytes 0-15/31: AES key (128 or 256 bit)
    * - Bytes 16-31/32-47: IV (16 bytes)
    */
   struct AESCrypto {
       static Crypto* get_instance();
   };
   
   } // namespace crypto
   } // namespace federated
   ```

4. **Test Requirements**
   - NIST test vectors (FIPS 197 Appendix C)
   - RFC 3602 test vectors
   - Known Answer Tests (KAT)
   - Multi-Block Message Tests (MMT)
   - Edge cases: single block, multiple blocks, padding
   - Performance benchmarks

**Implementation Approach:**
- Use lookup tables for SubBytes/MixColumns (performance)
- Implement constant-time operations (security)
- Focus on CBC mode first (simpler, widely compatible)
- Defer GCM mode to later if time permits

**Estimated Effort:** 3-5 days

**Success Criteria:**
- ✅ All NIST test vectors pass
- ✅ 100% test coverage for AES operations
- ✅ CBC mode fully functional
- ✅ No CodeQL security alerts
- ✅ Performance: Encrypt 1MB in < 100ms

---

### Task 1.2: Complete DNS Tunnel Transport

**Rationale:** Critical for censored network scenarios where only DNS traffic is allowed.

**Current Status:** Basic implementation exists with send/receive, needs completion

**RFC Compliance:**
- RFC 1035 - Domain Names - Implementation and Specification
- RFC 4648 - The Base16, Base32, and Base64 Data Encodings (Base32 section)

**Implementation Requirements:**

1. **Data Encoding**
   - Base32 encoding for DNS-safe payloads (RFC 4648)
   - Subdomain label splitting (63 char max per label, 253 total)
   - Payload chunking for large messages
   - Query ID tracking for request/response matching

2. **DNS Query Construction**
   - Proper DNS header format (RFC 1035 Section 4.1.1)
   - TXT record queries (type 16)
   - QNAME construction with encoded data
   - Query class: IN (Internet)

3. **Network Operations**
   - UDP socket for DNS queries (port 53)
   - Configurable DNS server (default: 8.8.8.8)
   - Timeout handling and retries
   - Response parsing for TXT records

4. **Anti-Detection Features**
   - Rate limiting (max queries per second)
   - Random query delays (jitter)
   - Domain rotation (multiple base domains)
   - Query pattern randomization

5. **Interface Enhancement**
   ```cpp
   // Configuration
   struct DNSConfig {
       const char* dns_server = "8.8.8.8";
       uint16_t dns_port = 53;
       const char* base_domain = "example.com";
       uint32_t max_qps = 5;  // Max queries per second
       uint32_t timeout_ms = 5000;
   };
   ```

**Test Requirements:**
- Base32 encoding/decoding tests
- DNS query construction validation
- Label splitting edge cases
- Rate limiting verification
- E2E test with actual DNS server
- Large payload chunking tests

**Implementation Approach:**
1. Implement Base32 encoder/decoder first
2. Build DNS query constructor
3. Add UDP socket operations
4. Implement response parser
5. Add rate limiting layer
6. Integration testing with real DNS

**Estimated Effort:** 4-6 days

**Success Criteria:**
- ✅ Base32 encoding matches RFC 4648 test vectors
- ✅ DNS queries properly formatted per RFC 1035
- ✅ Rate limiting enforced (no bursts)
- ✅ Successfully transfers 1KB payload
- ✅ E2E test with public DNS server passes

---

## Priority 2: MEDIUM - Platform-Dependent Transports

### Task 2.1: Bluetooth Transport (Linux First)

**Rationale:** Enables local P2P communication without internet infrastructure.

**Current Status:** Stub implementation with TODO markers

**Platform Strategy:** Start with Linux (BlueZ), then Windows (WinSock), macOS later

**Implementation Requirements:**

1. **Platform Abstraction Layer**
   ```cpp
   // Platform detection
   #if defined(__linux__)
       #define PLATFORM_BLUETOOTH_BLUEZ
   #elif defined(_WIN32)
       #define PLATFORM_BLUETOOTH_WINSOCK
   #elif defined(__APPLE__)
       #define PLATFORM_BLUETOOTH_IOBT
   #else
       #define PLATFORM_BLUETOOTH_UNSUPPORTED
   #endif
   ```

2. **Linux Implementation (BlueZ)**
   - BlueZ API via system calls (no external libs)
   - HCI socket for adapter control
   - RFCOMM socket creation (`AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM`)
   - Device address parsing (MAC format: `XX:XX:XX:XX:XX:XX`)
   - Service discovery via SDP
   - Connection establishment and pairing

3. **Windows Implementation**
   - Winsock Bluetooth extensions
   - RFCOMM via `BTHPROTO_RFCOMM`
   - Device enumeration using `WSALookupServiceBegin`
   - Similar socket API to Linux

4. **Interface Design**
   ```cpp
   struct BluetoothConfig {
       char device_addr[18];  // "XX:XX:XX:XX:XX:XX"
       uint8_t channel = 1;   // RFCOMM channel
       uint32_t timeout_ms = 10000;
   };
   ```

**Test Requirements:**
- Platform-specific test suites (`#ifdef` guards)
- Mock Bluetooth adapter for unit tests
- Integration tests with real hardware (manual)
- Cross-platform compilation tests
- Error handling for unavailable Bluetooth

**Implementation Approach:**
1. Create platform abstraction headers
2. Implement Linux version first (most common dev platform)
3. Add Windows support
4. Create stub for macOS (defer to later)
5. Add platform detection to CMake

**Estimated Effort:** 5-7 days per platform (10-14 days total for Linux + Windows)

**Success Criteria:**
- ✅ Builds on Linux with BlueZ support
- ✅ Builds on Windows with Winsock
- ✅ Compiles (stub) on macOS
- ✅ Transfers data between two Linux machines
- ✅ Graceful degradation on unsupported platforms

---

### Task 2.2: WiFi Direct Transport

**Rationale:** High-bandwidth P2P communication without infrastructure.

**Current Status:** Stub implementation

**Platform Strategy:** Platform abstraction layer, Linux first (wpa_supplicant)

**Implementation Requirements:**

1. **Linux Implementation (wpa_supplicant)**
   - D-Bus interface to wpa_supplicant
   - P2P device discovery
   - Group formation (GO or client)
   - Interface configuration
   - Socket creation over P2P interface

2. **Windows Implementation**
   - Windows WiFi Direct API
   - Peer discovery
   - Connection negotiation
   - Socket over WiFi Direct interface

3. **Network Layer**
   - TCP or UDP socket over P2P interface
   - IP address assignment handling
   - DHCP coordination (GO acts as DHCP server)

**Test Requirements:**
- Platform-specific tests
- Mock wpa_supplicant responses
- Integration tests with real WiFi Direct hardware
- Fallback to regular WiFi if Direct unavailable

**Estimated Effort:** 6-8 days

**Success Criteria:**
- ✅ Linux implementation with wpa_supplicant
- ✅ Windows stub created
- ✅ Successful P2P connection established
- ✅ Data transfer over WiFi Direct link

---

### Task 2.3: Infrared (IrDA) Transport

**Rationale:** Legacy support, niche use cases, low-bandwidth fallback.

**Priority:** LOW (can be deferred if time is limited)

**Current Status:** Stub implementation

**Implementation Requirements:**

1. **Serial Port Access**
   - POSIX: `/dev/ttyS*` or `/dev/ircomm*`
   - Windows: `COM*` ports
   - Termios configuration for IrDA

2. **IrDA Protocol Framing**
   - IrLAP (Link Access Protocol)
   - IrLMP (Link Management Protocol)
   - Simplified implementation for point-to-point

3. **Platform Detection**
   - Check for IrDA device presence
   - Graceful fallback if unsupported

**Test Requirements:**
- Serial port mock for unit tests
- Manual testing with real IrDA hardware (if available)

**Estimated Effort:** 3-4 days

**Success Criteria:**
- ✅ Compiles on all platforms
- ✅ Basic serial communication works
- ✅ IrDA framing implemented
- ✅ Documented limitations and requirements

---

## Priority 3: LOW - Advanced Cryptography

### Task 3.1: Public Key Cryptography

**Rationale:** Enables asymmetric encryption, digital signatures, key exchange.

**Current Status:** Stub placeholder

**Blocker:** Requires big integer arithmetic library or violates zero-dependency principle.

**Options:**

1. **Option A: Custom Big Integer Implementation**
   - **Pros:** Zero dependencies maintained, full control
   - **Cons:** Very high effort (2-3 weeks), potential for bugs
   - **Scope:** Arbitrary precision arithmetic, modular exponentiation, primality testing

2. **Option B: Evaluate Lightweight Crypto Libraries**
   - **Pros:** Battle-tested, faster implementation
   - **Cons:** Violates zero-dependency principle
   - **Candidates:** mbedTLS (considered lightweight), TinyCrypt
   - **Decision:** Requires architecture review

3. **Option C: Defer to Phase 5+**
   - **Pros:** Focus on more critical features first
   - **Cons:** Limits functionality of some advanced features
   - **Recommendation:** Defer pending architecture decision

**Recommendation:** **DEFER to Phase 5+** pending architecture review on dependency policy.

**If Implemented, Requirements:**

- RSA key generation (2048, 4096 bit)
- RSA encryption/decryption (PKCS#1 v1.5, OAEP)
- RSA signatures (PSS, PKCS#1 v1.5)
- ECC support (NIST P-256, Curve25519)
- ECDH key exchange
- Certificate handling (X.509 parsing - optional)

**Estimated Effort:** 15-20 days (if custom implementation)

---

## Documentation Tasks

### Task D.1: Update README.md

**Changes Required:**
1. Add Phase 4 status section
2. Update completion percentages for transport and crypto layers
3. Add AES example to Quick Start (once implemented)
4. Add DNS Tunnel example to Quick Start (once enhanced)
5. Update test count metrics
6. Update "Next Priorities" section

### Task D.2: Create Supporting Documentation (This File)

**Files to Create:**
- ✅ `docs/PHASE4_ROADMAP.md` (this file)
- ✅ `docs/PLATFORM_ABSTRACTION.md` (platform design guide)
- ✅ `docs/TODO_TRACKING.md` (central TODO tracking)

### Task D.3: Update docs/STATUS.md

**Changes Required:**
1. Sync with actual implementation status
2. Update test counts (currently outdated)
3. Add TODO tracking table
4. Update completion percentages
5. Add Phase 4 section

---

## Implementation Guidelines

All Phase 4 implementations MUST follow project standards:

### Test-First Development
- Write failing tests before implementation
- Minimum 100% coverage for critical paths
- Include edge cases and error conditions
- RFC test vectors for cryptography

### Zero Dependencies
- Only C++17 standard library
- Platform APIs via syscalls only (no external libs)
- Custom implementations for all algorithms

### RFC Compliance
- Heavy documentation with RFC references
- Inline comments citing RFC sections
- Exact spec conformance where applicable

### Flat Architecture
- No deep inheritance hierarchies
- Explicit error handling (no exceptions)
- Clear ownership and state management
- Atomic, testable modules

### Cross-Platform Support
- Platform detection via preprocessor macros
- Graceful degradation on unsupported platforms
- Separate compilation units for platform-specific code
- Document platform requirements in README

---

## Success Criteria

### Phase 4 Completion Metrics

1. **Functionality**
   - ✅ AES implementation with NIST test vectors (100% pass)
   - ✅ DNS Tunnel fully functional with rate limiting
   - ✅ Bluetooth transport working on Linux
   - ✅ WiFi Direct transport working on Linux
   - ✅ Infrared stub completed (optional)

2. **Quality**
   - ✅ Zero security vulnerabilities (CodeQL clean)
   - ✅ All tests passing (unit + scenario + E2E)
   - ✅ 100% test coverage maintained
   - ✅ Build time remains < 15 seconds
   - ✅ Zero compiler warnings

3. **Documentation**
   - ✅ README.md updated with Phase 4 status
   - ✅ docs/STATUS.md synchronized
   - ✅ Platform requirements documented
   - ✅ Code examples for new features
   - ✅ RFC references complete

4. **Performance**
   - ✅ AES: Encrypt 1MB in < 100ms
   - ✅ DNS Tunnel: < 10 QPS average
   - ✅ Bluetooth: > 100 KB/s throughput
   - ✅ WiFi Direct: > 1 MB/s throughput

---

## Timeline Estimate

**Conservative Estimate:** 25-35 days (5-7 weeks)

| Task | Effort | Dependencies |
|------|--------|--------------|
| AES Implementation | 3-5 days | None |
| DNS Tunnel Completion | 4-6 days | None |
| Bluetooth (Linux) | 5-7 days | Platform abstraction layer |
| Bluetooth (Windows) | 5-7 days | Linux Bluetooth |
| WiFi Direct (Linux) | 6-8 days | Platform abstraction layer |
| Infrared (stub) | 3-4 days | Platform abstraction layer |
| Documentation Updates | 2-3 days | All implementations |
| Testing & Integration | 3-5 days | All implementations |

**Aggressive Estimate:** 18-22 days (3-4 weeks) if focused

---

## Notes and Considerations

### AES Considerations
- Use constant-time operations to prevent timing attacks
- Lookup tables vs. computation trade-off (use tables for performance)
- Key expansion can be pre-computed and cached
- CBC mode requires proper IV handling (never reuse IVs)
- PKCS#7 padding must handle all block sizes correctly

### DNS Tunnel Considerations
- DNS query rate limits vary by server (respect them)
- Some networks block non-standard DNS servers (allow override)
- TXT record size limits vary (typically 255 bytes per string, 4KB total)
- Base32 encoding increases payload size by ~60%
- Consider implementing DNS over HTTPS (DoH) for added stealth

### Platform Transport Considerations
- Platform APIs may require elevated privileges (document this)
- Bluetooth pairing may require user interaction (handle gracefully)
- WiFi Direct group owner negotiation can be complex
- IrDA is rarely available on modern hardware (low priority)
- Mock implementations for unit testing are essential

### Security Considerations
- All cryptographic implementations must be reviewed
- Side-channel attacks are a real threat (constant-time ops)
- Platform transports may expose MAC addresses (privacy concern)
- DNS queries are visible to ISPs and DNS servers (metadata leakage)
- Rate limiting must be enforced to prevent detection

---

## References

### RFCs and Standards
- RFC 1035 - Domain Names
- RFC 3602 - AES-CBC Cipher Algorithm
- RFC 4648 - Base32/Base64 Data Encodings
- NIST FIPS 197 - Advanced Encryption Standard
- RFC 8439 - ChaCha20 and Poly1305 (reference for comparison)

### Platform Documentation
- BlueZ API Documentation (Linux Bluetooth)
- wpa_supplicant D-Bus Interface (Linux WiFi Direct)
- Windows Bluetooth API Reference
- Windows WiFi Direct API Reference
- POSIX Serial Port Programming Guide

### Project Documentation
- `README.md` - Current status and overview
- `docs/STATUS.md` - Detailed implementation status
- `docs/rfc_references.md` - Complete RFC list
- `docs/test_plan.md` - Testing strategy
- `docs/architecture.md` - System architecture
- `.github/agents/copilot-instructions.md` - Development guidelines

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-27 | 1.0 | Initial Phase 4 roadmap created |

---

**Next Steps:**
1. Review and approve this roadmap
2. Begin with AES implementation (highest priority)
3. Create platform abstraction layer design
4. Start implementing tasks in priority order
5. Update progress in STATUS.md regularly

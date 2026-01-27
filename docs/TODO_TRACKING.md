# TODO Tracking

**Purpose:** Central tracking of all TODO items across the codebase  
**Status:** Active tracking for Phase 4 and beyond  
**Last Updated:** 2026-01-27

---

## Overview

This document tracks all TODO items found in the codebase, organized by component and priority. Each TODO is linked to its location in the source code and associated with a phase and priority level.

---

## Summary Statistics

| Category | Total | High Priority | Medium Priority | Low Priority | Completed |
|----------|-------|---------------|-----------------|--------------|-----------|
| **Transport** | 15 | 5 | 5 | 5 | 0 |
| **Crypto** | 5 | 1 | 0 | 4 | 0 |
| **Services** | 0 | 0 | 0 | 0 | 0 |
| **Tools** | 0 | 0 | 0 | 0 | 0 |
| **Documentation** | 3 | 3 | 0 | 0 | 0 |
| **TOTAL** | 23 | 9 | 5 | 9 | 0 |

---

## High Priority (Phase 4 - Q1 2026)

### CRYPTO-001: Implement AES Encryption
**Status:** 🔴 Not Started  
**Priority:** HIGH  
**Phase:** 4  
**Location:** Not yet created  
**Assignee:** TBD  
**Estimated Effort:** 3-5 days

**Description:**
Implement AES-128 and AES-256 encryption with CBC and GCM modes per NIST FIPS 197.

**Requirements:**
- [ ] AES core algorithm (encryption/decryption)
- [ ] Key expansion for 128 and 256-bit keys
- [ ] CBC mode with PKCS#7 padding
- [ ] GCM mode (optional)
- [ ] Constant-time operations (side-channel resistance)
- [ ] NIST test vectors (100% pass)

**Files to Create:**
- `include/federated/crypto/aes.h`
- `src/crypto/aes.cpp`
- `tests/crypto/test_aes.cpp`

**References:**
- NIST FIPS 197
- RFC 3602 (AES-CBC)
- `docs/PHASE4_ROADMAP.md` - Task 1.1

---

### TRANSPORT-001: Complete DNS Tunnel Implementation
**Status:** 🟡 In Progress (Basic implementation exists)  
**Priority:** HIGH  
**Phase:** 4  
**Location:** `src/transport/dns_tunnel.cpp`, `include/federated/transport/dns_tunnel.h`  
**Assignee:** TBD  
**Estimated Effort:** 4-6 days

**Description:**
Complete DNS Tunnel transport with Base32 encoding, rate limiting, and full RFC 1035 compliance.

**Current Status:**
- ✅ Basic send/receive implemented
- ✅ DNS query construction
- ❌ Base32 encoding (currently simplified)
- ❌ Rate limiting
- ❌ Label splitting (63 char limit)
- ❌ TXT record parsing

**Remaining Work:**
- [ ] Implement proper Base32 encoding per RFC 4648
- [ ] Add subdomain label splitting (63 char per label, 253 total)
- [ ] Implement rate limiting (configurable QPS)
- [ ] Add jitter to query timing (anti-detection)
- [ ] Enhance TXT record parsing
- [ ] Add E2E test with real DNS server
- [ ] Add scenario tests for large payloads

**Files to Modify:**
- `src/transport/dns_tunnel.cpp` (enhance implementation)
- `tests/transport/test_dns_tunnel.cpp` (add comprehensive tests)

**References:**
- RFC 1035 (DNS)
- RFC 4648 (Base32)
- `docs/PHASE4_ROADMAP.md` - Task 1.2

---

### TRANSPORT-002: Implement Bluetooth Transport (Linux)
**Status:** 🔴 Not Started (Stub exists)  
**Priority:** HIGH  
**Phase:** 4  
**Location:** `src/transport/bluetooth.cpp`, `include/federated/transport/bluetooth.h`  
**Assignee:** TBD  
**Estimated Effort:** 5-7 days

**Description:**
Implement Bluetooth transport for Linux using BlueZ stack and RFCOMM sockets.

**TODO Items from Code:**
1. `bluetooth.cpp:9` - Implement Bluetooth connection
2. `bluetooth.cpp:19` - Close Bluetooth connection
3. `bluetooth.cpp:25` - Send data over Bluetooth
4. `bluetooth.cpp:33` - Receive data from Bluetooth
5. `bluetooth.cpp:41` - Check if Bluetooth is connected

**Requirements:**
- [ ] Platform detection and abstraction
- [ ] Linux BlueZ implementation (RFCOMM sockets)
- [ ] Device address parsing (MAC format)
- [ ] Connection establishment
- [ ] Send/receive operations
- [ ] Error handling and retry logic
- [ ] Unit tests with mocks
- [ ] Integration tests (manual with hardware)

**Files to Modify:**
- `src/transport/bluetooth.cpp` (implement Linux version)
- `tests/transport/test_bluetooth.cpp` (add tests)

**References:**
- `docs/PHASE4_ROADMAP.md` - Task 2.1
- `docs/PLATFORM_ABSTRACTION.md` - Linux Bluetooth section

---

### DOC-001: Update docs/STATUS.md
**Status:** 🔴 Not Started  
**Priority:** HIGH  
**Phase:** 4  
**Location:** `docs/STATUS.md`  
**Assignee:** TBD  
**Estimated Effort:** 2-3 hours

**Description:**
Synchronize STATUS.md with current implementation state (currently dated 2026-01-22, outdated).

**Requirements:**
- [ ] Update test counts (currently shows 33 tests, actual: 159)
- [ ] Update assertion counts (currently shows 115, actual: 692)
- [ ] Update transport layer status (show DNS Tunnel as implemented)
- [ ] Update crypto layer status (show ChaCha20 as implemented)
- [ ] Add services section (HTTP, SMTP, IMAP implemented)
- [ ] Update completion percentages
- [ ] Add TODO tracking table
- [ ] Update "Last Updated" date to 2026-01-27

**References:**
- Current README.md (has accurate stats)
- `docs/PHASE4_ROADMAP.md`

---

### DOC-002: Update README.md with Phase 4 Status
**Status:** 🔴 Not Started  
**Priority:** HIGH  
**Phase:** 4  
**Location:** `README.md`  
**Assignee:** TBD  
**Estimated Effort:** 1-2 hours

**Description:**
Add Phase 4 section to README.md and update completion metrics.

**Requirements:**
- [ ] Add Phase 4 status section under "Development Status"
- [ ] Update crypto layer percentage (add AES when implemented)
- [ ] Update transport layer percentage (reflect DNS Tunnel completion)
- [ ] Add AES example to Quick Start (once implemented)
- [ ] Update "Next Priorities" section with Phase 4 tasks
- [ ] Reference PHASE4_ROADMAP.md for details

**References:**
- `docs/PHASE4_ROADMAP.md`

---

### DOC-003: Update Copilot Instructions for Phase 4
**Status:** 🔴 Not Started  
**Priority:** HIGH  
**Phase:** 4  
**Location:** `.github/agents/copilot-instructions.md`  
**Assignee:** TBD  
**Estimated Effort:** 1 hour

**Description:**
Add Phase 4 guidelines to Copilot instructions.

**Requirements:**
- [ ] Add section on platform-specific implementations
- [ ] Add guidelines for #ifdef usage
- [ ] Reference PLATFORM_ABSTRACTION.md
- [ ] Add AES implementation guidelines (constant-time ops)
- [ ] Add covert transport considerations (rate limiting)

**References:**
- `docs/PLATFORM_ABSTRACTION.md`
- `docs/PHASE4_ROADMAP.md`

---

## Medium Priority (Phase 4 - Q2 2026)

### TRANSPORT-003: Implement Bluetooth Transport (Windows)
**Status:** 🔴 Not Started  
**Priority:** MEDIUM  
**Phase:** 4  
**Location:** `src/transport/bluetooth.cpp`  
**Assignee:** TBD  
**Estimated Effort:** 5-7 days

**Description:**
Implement Bluetooth transport for Windows using Winsock2 Bluetooth extensions.

**Requirements:**
- [ ] Windows platform detection
- [ ] Winsock2 initialization
- [ ] RFCOMM socket creation (AF_BTH)
- [ ] Device address handling (uint64 format)
- [ ] Connection establishment
- [ ] Send/receive operations
- [ ] Platform-specific tests

**References:**
- `docs/PLATFORM_ABSTRACTION.md` - Windows Bluetooth section

---

### TRANSPORT-004: Implement WiFi Direct Transport (Linux)
**Status:** 🔴 Not Started (Stub exists)  
**Priority:** MEDIUM  
**Phase:** 4  
**Location:** `src/transport/wifi_direct.cpp`, `include/federated/transport/wifi_direct.h`  
**Assignee:** TBD  
**Estimated Effort:** 6-8 days

**Description:**
Implement WiFi Direct transport using wpa_supplicant on Linux.

**TODO Items from Code:**
1. `wifi_direct.cpp:9` - Implement WiFi Direct connection
2. `wifi_direct.cpp:19` - Close WiFi Direct connection
3. `wifi_direct.cpp:25` - Send data over WiFi Direct
4. `wifi_direct.cpp:33` - Receive data from WiFi Direct
5. `wifi_direct.cpp:41` - Check if WiFi Direct is connected

**Requirements:**
- [ ] wpa_supplicant control interface integration
- [ ] P2P device discovery
- [ ] Group formation (GO or client)
- [ ] Socket creation over P2P interface
- [ ] IP address handling
- [ ] Platform-specific tests

**References:**
- `docs/PHASE4_ROADMAP.md` - Task 2.2
- `docs/PLATFORM_ABSTRACTION.md` - WiFi Direct section

---

### TRANSPORT-005: Implement Infrared Transport
**Status:** 🔴 Not Started (Stub exists)  
**Priority:** MEDIUM  
**Phase:** 4  
**Location:** `src/transport/infrared.cpp`, `include/federated/transport/infrared.h`  
**Assignee:** TBD  
**Estimated Effort:** 3-4 days

**Description:**
Implement Infrared (IrDA) transport using serial port communication.

**TODO Items from Code:**
1. `infrared.cpp:9` - Implement infrared connection
2. `infrared.cpp:19` - Close infrared connection
3. `infrared.cpp:25` - Send data over infrared
4. `infrared.cpp:33` - Receive data from infrared
5. `infrared.cpp:41` - Check if infrared is connected

**Requirements:**
- [ ] Serial port access (POSIX termios / Windows COM)
- [ ] IrDA framing (simplified for P2P)
- [ ] 115200 baud configuration
- [ ] Platform-specific serial port handling
- [ ] Error detection and retry

**References:**
- `docs/PHASE4_ROADMAP.md` - Task 2.3
- `docs/PLATFORM_ABSTRACTION.md` - Serial Port section

---

### TRANSPORT-006: WiFi Direct Windows Stub
**Status:** 🔴 Not Started  
**Priority:** MEDIUM  
**Phase:** 4-5  
**Location:** To be created  
**Assignee:** TBD  
**Estimated Effort:** 6-8 days

**Description:**
Implement or stub WiFi Direct for Windows (may require WinRT/COM).

**Requirements:**
- [ ] Evaluate WinRT compatibility with zero-dependency goal
- [ ] Alternative: Use netsh commands
- [ ] Platform-specific implementation or documented stub

**References:**
- `docs/PLATFORM_ABSTRACTION.md` - Windows WiFi Direct section

---

### TRANSPORT-007: Bluetooth macOS Stub
**Status:** 🔴 Not Started  
**Priority:** MEDIUM  
**Phase:** 5  
**Location:** To be created  
**Assignee:** TBD  
**Estimated Effort:** TBD

**Description:**
Create stub or implementation for Bluetooth on macOS.

**Requirements:**
- [ ] IOBluetooth framework evaluation
- [ ] Platform-specific implementation or stub

---

## Low Priority (Phase 5+ - Future)

### CRYPTO-002: Public Key Cryptography - Big Integer Library
**Status:** 🔴 Blocked  
**Priority:** LOW  
**Phase:** 5+  
**Location:** `src/crypto/public_key.cpp`, `include/federated/crypto/public_key.h`  
**Assignee:** TBD  
**Estimated Effort:** 15-20 days (if custom implementation)

**Description:**
Implement or integrate big integer arithmetic library for RSA/ECC.

**TODO Items from Code:**
1. `public_key.h:32` - Big integer arithmetic
2. `public_key.h:33` - RSA key generation
3. `public_key.h:34` - RSA encryption/decryption
4. `public_key.h:35` - Digital signatures
5. `public_key.h:36` - ECC support

**Blocker:** Requires big integer library, conflicts with zero-dependency principle.

**Options:**
1. Custom big integer implementation (very high effort)
2. Evaluate lightweight libraries (policy decision needed)
3. Defer until policy clarified

**Requirements (if implemented):**
- [ ] Big integer arithmetic (add, multiply, mod, exponentiation)
- [ ] RSA key generation (2048, 4096 bit)
- [ ] RSA PKCS#1 v1.5 encryption/decryption
- [ ] RSA-OAEP (optional)
- [ ] RSA signatures (PSS, PKCS#1 v1.5)
- [ ] ECC curves (P-256, Curve25519)
- [ ] ECDH key exchange

**References:**
- RFC 8017 (RSA)
- `docs/PHASE4_ROADMAP.md` - Task 3.1

---

### TRANSPORT-008: Audio Transport
**Status:** 🔴 Not Started  
**Priority:** LOW  
**Phase:** 6  
**Location:** Not yet created  
**Assignee:** TBD  
**Estimated Effort:** 8-10 days

**Description:**
Implement audio modem transport (FSK/AFSK modulation).

**Requirements:**
- [ ] Audio capture/playback via OS APIs
- [ ] FSK modulation/demodulation
- [ ] Error correction coding
- [ ] Platform-specific audio access

---

### TRANSPORT-009: QR Code Transport
**Status:** 🔴 Not Started  
**Priority:** LOW  
**Phase:** 6  
**Location:** Not yet created  
**Assignee:** TBD  
**Estimated Effort:** 5-7 days

**Description:**
Implement QR code transport (encode data to QR, decode from camera).

**Requirements:**
- [ ] QR code generation (custom or minimal library)
- [ ] QR code parsing
- [ ] Camera access (platform-specific)
- [ ] Frame-by-frame transmission protocol

---

### TRANSPORT-010: FM Radio Transport
**Status:** 🔴 Not Started  
**Priority:** LOW  
**Phase:** 7  
**Location:** Not yet created  
**Assignee:** TBD  
**Estimated Effort:** 10-15 days

**Description:**
Implement FM radio transport (RDS/RBDS data channel).

**Requirements:**
- [ ] Software-defined radio integration or hardware interface
- [ ] RDS encoding/decoding
- [ ] FCC/regulatory compliance
- [ ] Platform-specific radio access

---

## Completed Items

*(None yet - this section will track completed TODOs)*

---

## Tracking by File

### Transport Layer

#### `include/federated/transport/bluetooth.h`
- Line 24: TODO: Platform-specific Bluetooth API integration
- Line 25: TODO: RFCOMM socket setup
- Line 26: TODO: Device discovery and pairing
- Line 27: TODO: Data transmission over Bluetooth

#### `src/transport/bluetooth.cpp`
- Line 9: TODO: Implement Bluetooth connection
- Line 19: TODO: Close Bluetooth connection
- Line 25: TODO: Send data over Bluetooth
- Line 33: TODO: Receive data from Bluetooth
- Line 41: TODO: Check if Bluetooth is connected

#### `include/federated/transport/wifi_direct.h`
- Line 24: TODO: Platform integration (Linux: wpa_supplicant, Windows: WiFi Direct API)
- Line 25: TODO: P2P group formation
- Line 26: TODO: TCP/UDP socket creation over WiFi Direct interface

#### `src/transport/wifi_direct.cpp`
- Line 9: TODO: Implement WiFi Direct connection
- Line 19: TODO: Close WiFi Direct connection
- Line 25: TODO: Send data over WiFi Direct
- Line 33: TODO: Receive data from WiFi Direct
- Line 41: TODO: Check if WiFi Direct is connected

#### `include/federated/transport/infrared.h`
- Line 24: TODO: Serial port communication for IrDA
- Line 25: TODO: IrDA protocol framing
- Line 26: TODO: Platform-specific serial port access

#### `src/transport/infrared.cpp`
- Line 9: TODO: Implement infrared connection
- Line 19: TODO: Close infrared connection
- Line 25: TODO: Send data over infrared
- Line 33: TODO: Receive data from infrared
- Line 41: TODO: Check if infrared is connected

### Crypto Layer

#### `include/federated/crypto/public_key.h`
- Line 32: TODO: Big integer arithmetic
- Line 33: TODO: RSA key generation
- Line 34: TODO: RSA encryption/decryption
- Line 35: TODO: Digital signatures
- Line 36: TODO: ECC support

#### `src/crypto/public_key.cpp`
- (Contains similar TODOs in implementation section)

### Documentation

#### `docs/STATUS.md`
- Needs comprehensive update (see DOC-001)

#### `README.md`
- Needs Phase 4 section (see DOC-002)

#### `.github/agents/copilot-instructions.md`
- Needs Phase 4 guidelines (see DOC-003)

---

## Progress Tracking

### Phase 4 Progress (Target: Q1 2026)

**High Priority (6 items):**
- [ ] CRYPTO-001: AES Implementation (0%)
- [ ] TRANSPORT-001: DNS Tunnel Completion (40% - basic impl exists)
- [ ] TRANSPORT-002: Bluetooth Linux (0%)
- [ ] DOC-001: Update STATUS.md (0%)
- [ ] DOC-002: Update README.md (0%)
- [ ] DOC-003: Update Copilot Instructions (0%)

**Medium Priority (5 items):**
- [ ] TRANSPORT-003: Bluetooth Windows (0%)
- [ ] TRANSPORT-004: WiFi Direct Linux (0%)
- [ ] TRANSPORT-005: Infrared (0%)
- [ ] TRANSPORT-006: WiFi Direct Windows (0%)
- [ ] TRANSPORT-007: Bluetooth macOS (0%)

**Overall Phase 4 Completion: 0%** (0 of 11 items complete)

---

## Review Process

### Adding New TODOs
1. Add TODO comment in code with clear description
2. Update this tracking document with new entry
3. Assign priority and phase
4. Estimate effort
5. Link to relevant documentation

### Completing TODOs
1. Implement the feature with tests
2. Remove TODO comment from code
3. Move item to "Completed Items" section
4. Update progress metrics
5. Update relevant documentation

### Review Cadence
- **Weekly:** Review high-priority items
- **Bi-weekly:** Review medium-priority items
- **Monthly:** Review low-priority and long-term items
- **Per-phase:** Comprehensive review and re-prioritization

---

## Notes

- All estimates are preliminary and subject to change
- Priority levels may be adjusted based on project needs
- Blocked items require policy/architecture decisions
- Platform-specific items depend on platform abstraction layer
- Documentation items should be done in parallel with implementation

---

## References

- `docs/PHASE4_ROADMAP.md` - Detailed Phase 4 implementation plan
- `docs/PLATFORM_ABSTRACTION.md` - Platform-specific implementation guide
- `README.md` - Current project status and overview
- `docs/STATUS.md` - Detailed implementation status (needs update)
- `.github/agents/copilot-instructions.md` - Development guidelines

---

**Last Updated:** 2026-01-27  
**Next Review:** 2026-02-03 (weekly high-priority review)

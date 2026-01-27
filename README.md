# FEDERATED — Universal Resilient Communication Toolkit

**Language:** C++17 (zero external dependencies)  
**Methodology:** Test-First, Flat Design, Explicit Ownership  
**Target:** Resilient communications under normal, degraded, or censored networks

---

## Overview

Federated is a **universal, resilient communication toolkit** designed to operate across **any available medium** — digital, analog, or human-assisted — while remaining auditable, extensible, and survivable.

### Key Features

* Communication during partial or total internet shutdowns
* Acts as a standard internet server when connectivity exists
* Multi-modal operation: P2P, Share-It-style, or classic web service
* Optional cryptography, onion routing, and federation
* Built-in diagnostics, monitoring, feed aggregation, and CLI/Web/TUI interfaces
* Scriptable, automatable, and observable behavior
* Interoperable with standard RFC-compliant servers (web, email, DNS)

---

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running Tests

```bash
./federated_tests
```

Or with CMake:

```bash
ctest
```

---

## Quick Start

### Start HTTP Server
```bash
# Build the project first
./build/federated

# The HTTP server can be started programmatically or via CLI
# Example: Serve static files from a directory
# (Full CLI integration coming in Phase 4)
```

### Use TCP Transport
```cpp
#include "federated/transport/tcp.h"
#include "federated/core/buffer.h"

using namespace federated;

// Get TCP transport instance
transport::Transport* tcp = transport::TCPTransport::get_instance();

// Open connection (creates loopback server for testing)
tcp->open();

// Send data
const char* msg = "Hello, TCP!";
core::Buffer send_buf(
    const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(msg)),
    strlen(msg)
);
tcp->send(send_buf);

// Receive data
uint8_t recv_data[1024];
core::Buffer recv_buf(recv_data, sizeof(recv_data));
tcp->recv(recv_buf);

// Close connection
tcp->close();
```

### Use ChaCha20 Encryption
```cpp
#include "federated/crypto/chacha20.h"
#include "federated/core/buffer.h"

using namespace federated;

// Get ChaCha20 instance
crypto::Crypto* chacha = crypto::ChaCha20Crypto::get_instance();

// Prepare key (32 bytes) + nonce (12 bytes)
uint8_t key_data[44];
// ... fill with key and nonce ...
core::Buffer key(key_data, 44);

// Encrypt
const char* plaintext = "Secret message";
core::Buffer plain_buf(
    const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(plaintext)),
    strlen(plaintext)
);

uint8_t cipher_data[1024];
core::Buffer cipher_buf(cipher_data, sizeof(cipher_data));

chacha->encrypt(plain_buf, key, cipher_buf);

// Decrypt (symmetric)
uint8_t decrypted_data[1024];
core::Buffer decrypted_buf(decrypted_data, sizeof(decrypted_data));

chacha->decrypt(cipher_buf, key, decrypted_buf);
```

### Use CRC Framing
```cpp
#include "federated/framer/crc.h"
#include "federated/core/buffer.h"

using namespace federated;

// Get CRC framer
framer::Framer* crc = framer::CRCFramer::get_instance();

// Encode with CRC checksum
const char* data = "Important data";
core::Buffer input(
    const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data)),
    strlen(data)
);

uint8_t frame_data[1024];
core::Buffer frame_buf(frame_data, sizeof(frame_data));

crc->encode(input, frame_buf);

// Decode and verify
uint8_t decoded_data[1024];
core::Buffer decoded_buf(decoded_data, sizeof(decoded_data));

core::ErrorCode err = crc->decode(frame_buf, decoded_buf);
if (err == core::OK) {
    // CRC valid, data intact
} else {
    // CRC failed, data corrupted
}
```

---

## Architecture

```
[ Transport ] → [ Framer ] → [ Crypto (optional) ] → [ Onion Router (optional) ] → [ Message Router ] → [ Service / Tool ]
```

All layers are independent, modular, and testable.

---

## Project Structure

```
include/federated/   # Public headers
  core/              # Core primitives (Buffer, Packet, Registry, Error)
  transport/         # Transport layer (TCP, UDP, DNS, File, Loopback, etc.)
  framer/            # Frame encoding/decoding
  crypto/            # Cryptography (None, XOR, ChaCha20, AES, Public-key)
  onion/             # Onion routing and bridge mode
  service/           # Services (HTTP, DNS, SMTP, Microblog, File exchange)
  tool/              # Diagnostics and utilities
    diag/            # ping, arp, netstat, ifconfig, route
    feed/            # RSS/JSON feed aggregation
    monitor/         # Agent-less monitoring
    cli/             # CLI interface

src/                 # Implementation (mirrors include)
tests/               # Test suite (mirrors src)
examples/            # Example applications
docs/                # Documentation
```

---

## Design Principles

1. **Flat architecture** — no deep inheritance, no hidden control flows
2. **Zero external runtime dependencies** — only libc and OS syscalls
3. **Test-first development** — every feature starts with a failing test
4. **Explicit state and ownership** — no hidden allocations, all errors returned explicitly
5. **Transport and protocol agnostic** — analog, digital, or hybrid channels
6. **Survivable by design** — operates in censorship, blackouts, or infrastructure collapse
7. **Interoperable** — peers may be standard RFC-compliant servers

---

## Development Status

**Last Updated:** 2026-01-27

### Current Implementation

#### ✅ Phase 1: Foundation (100% Complete)

**Core Primitives**
- Buffer, Packet, Error, Registry, Endian

**Transport Layer** (82% - 9 of 11 with stubs, 45% fully functional)
- ✅ FULL: Loopback, File, TCP (RFC 793), UDP (RFC 768), DNS Tunnel (RFC 1035)
- ⚠️ STUB: Bluetooth, WiFi Direct, Infrared (require platform APIs)
- ⏳ Planned: Audio, QR, FM Radio

**Framer Layer** (100% - 4 of 4)
- ✅ Raw, Length-prefix, CRC, Chunked

**Crypto Layer** (50% - 3 of 6)
- ✅ FULL: None, XOR Stream, ChaCha20 (RFC 8439)
- ⚠️ STUB: Public Key/RSA (requires big integer library)
- ⏳ Planned: AES, Advanced methods

**Services**
- ✅ HTTP Server (RFC 9110/9112) - 16 tests
- ✅ SMTP Protocol (RFC 5321) - 8 tests + E2E
- ✅ IMAP Protocol (RFC 3501) - 9 tests + E2E
- ✅ Mail Storage System - 6 tests

**Tools**
- ✅ Log System (5 levels, colors, drivers)
- ✅ CLI Framework (command/service registration, signals)

#### ✅ Phase 2 & 3: Essential Features (COMPLETED)

All essential transports (TCP, UDP, DNS Tunnel), framers (CRC, Chunked), crypto (ChaCha20), and services (HTTP, SMTP, IMAP) implemented with comprehensive tests.

#### ⏳ Phase 4-7: Advanced Features (Planned)

- Onion routing and federation layer
- Additional transports (Audio, QR, FM Radio)
- Advanced crypto (AES, key exchange, certificates)
- Full CLI/TUI/Web interfaces
- Performance benchmarking

### Test Coverage

- **159 tests** (140 unit + 19 scenario), **692 assertions**
- **100% code coverage** maintained
- E2E shell tests for SMTP, IMAP
- Build time: <10s, Test runtime: <1s
- Zero warnings, zero security alerts

### Documentation

- ✅ Complete: `docs/rfc_references.md`, `docs/test_plan.md`, `docs/architecture.md`
- ⏳ Needs update: `docs/STATUS.md`
- See `docs/` for detailed specifications

### Next Priorities

1. AES implementation (NIST standards)
2. Platform-dependent transport implementations
3. Performance benchmarking framework
4. Additional examples (microblog, P2P mesh, web service)

For detailed roadmap and task tracking, see `.github/copilot-instructions.md` for AI agent guidelines.

---

## License

MIT License - See LICENSE file for details.

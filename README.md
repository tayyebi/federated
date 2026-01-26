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

Currently implementing core primitives and basic transports following test-first methodology.

See `docs/` for detailed architecture and development guidelines.

---

## License

MIT License - See LICENSE file for details.

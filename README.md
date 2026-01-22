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

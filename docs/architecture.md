# Federated Architecture

## Overview

Federated is a universal, resilient communication toolkit designed with a flat, modular architecture.

## Data Flow

```
[ Transport ] → [ Framer ] → [ Crypto (optional) ] → [ Onion Router (optional) ] → [ Message Router ] → [ Service / Tool ]
```

## Core Components

### 1. Core Primitives

Located in `include/federated/core/`:

- **Buffer**: Simple memory buffer with explicit ownership
- **Packet**: Network packet containing a payload
- **Error**: Explicit error codes (no exceptions)
- **Registry**: Global registry for components (only global mutable state)

### 2. Transport Layer

Located in `include/federated/transport/`:

Provides pluggable transport implementations:

- **Loopback**: In-memory transport for testing
- **File**: Read/write to files (store-and-forward, dead drops)
- **TCP**: (Planned) TCP socket transport
- **UDP**: (Planned) UDP datagram transport
- **DNS Tunnel**: (Planned) Covert channel via DNS
- **Audio**: (Planned) Audio tone-based transport
- **QR**: (Planned) QR code-based transport

All transports implement the `Transport` interface and must be:
- Hot-pluggable
- Independently testable
- Support failure simulation

### 3. Framer Layer

Located in `include/federated/framer/`:

Provides frame encoding/decoding:

- **Raw**: Pass-through (no framing)
- **Length-prefix**: 4-byte length header + payload
- **CRC**: (Planned) CRC checksum framing
- **Chunked**: (Planned) Chunked transfer encoding

### 4. Crypto Layer

Located in `include/federated/crypto/`:

Cryptography is optional and tiered:

- **Tier 0 (None)**: No encryption (pass-through)
- **Tier 1**: (Planned) XOR/basic obfuscation
- **Tier 2**: (Planned) ChaCha20 / AES
- **Tier 3**: (Planned) Public-key cryptography
- **Tier 4**: (Planned) Advanced / future methods

### 5. Services

Located in `include/federated/service/`:

Application services (planned):

- HTTP static server
- DNS cache and resolver
- SMTP/IMAP email
- Micro-blogging (Fediverse/ActivityPub)
- File exchange

### 6. Tools

Located in `include/federated/tool/`:

Diagnostic and utility tools (planned):

- **Diagnostics**: ping, arp, netstat, ifconfig, route
- **Feed Aggregation**: RSS 2.0, Atom, JSON Feed
- **Monitoring**: Agent-less SSH-based monitoring
- **CLI**: Command-line interface

## Design Principles

1. **Flat Architecture**: No deep inheritance, no hidden control flows
2. **Explicit Ownership**: Caller owns memory, no hidden allocations
3. **Explicit Errors**: All errors returned explicitly, no exceptions
4. **Zero External Dependencies**: Only libc and OS syscalls
5. **Test-First**: Every feature starts with a failing test
6. **Transport Agnostic**: Works across digital, analog, or hybrid channels
7. **Interoperable**: Can communicate with standard RFC-compliant servers

## Testing Strategy

All components follow test-first development:

1. Write failing tests
2. Implement minimal code to pass tests
3. Refactor if needed
4. Repeat

Current test coverage:
- Core primitives: 100%
- Transports (loopback, file): 100%
- Framers (raw, length-prefix): 100%
- Crypto (none): 100%

## Building

```bash
mkdir build
cd build
cmake ..
make
./federated_tests
```

## Future Development

See the master specification document for planned features including:

- Additional transports (TCP, UDP, DNS, analog channels)
- Additional framers (CRC, chunked)
- Crypto tiers 1-4
- Onion routing and bridge mode
- Federation layer
- Services and tools
- UX surfaces (CLI, TUI, Web)

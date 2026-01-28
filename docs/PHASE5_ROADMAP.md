# Phase 5 Roadmap: Onion Routing and Federation

**Status:** IN PROGRESS  
**Target:** Q2 2026  
**Last Updated:** 2026-01-27

---

## Executive Summary

Phase 5 introduces the **onion routing layer** and **federation protocol**, enabling anonymous multi-hop communication and decentralized peer coordination. This phase builds upon the transport, framer, and crypto layers established in Phases 1-3, creating a foundation for resilient, privacy-preserving distributed systems.

**Key Deliverables:**
- ✅ Onion routing layer with multi-hop circuit construction
- ✅ Federation protocol for peer discovery and coordination
- ✅ Bridge mode support (relay nodes)
- ✅ Directory service for node discovery
- ✅ Comprehensive test coverage

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                   Federation Protocol                        │
│  ┌───────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │ Peer Registry │  │ Discovery    │  │ Peer Messaging  │  │
│  └───────────────┘  └──────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                     Onion Router                             │
│  ┌───────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │ Circuit       │  │ Encryption   │  │ Bridge/Relay    │  │
│  │ Builder       │  │ Layering     │  │ Mode            │  │
│  └───────────────┘  └──────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│              Existing Layers (Crypto, Framer, Transport)    │
└─────────────────────────────────────────────────────────────┘
```

---

## Priority 1: HIGH - Onion Routing Layer

### Task 1.1: Onion Router Core Implementation

**Rationale:** Enables anonymous multi-hop communication by layering encryption across multiple relays.

**Current Status:** Not implemented

**Design Principles:**
- RFC-inspired design (similar to Tor's approach but simplified)
- Zero external dependencies
- Test-first development
- Modular and composable

**Implementation Requirements:**

1. **Core Onion Interface**
   ```cpp
   // include/federated/onion/onion.h
   namespace federated {
   namespace onion {
   
   /**
    * Onion Router
    * 
    * Provides multi-hop anonymous routing through layered encryption.
    * Each hop adds/removes one layer of encryption (like peeling an onion).
    * 
    * Architecture:
    * - Circuit: Path through multiple nodes
    * - Hop: Single node in a circuit
    * - Layer: One encryption layer per hop
    * 
    * References:
    * - Tor Design: https://spec.torproject.org/tor-spec
    * - Onion Routing: Goldschlag, Reed, Syverson (1999)
    */
   class OnionRouter {
   public:
       virtual ~OnionRouter() = default;
       
       // Create a new circuit through specified hops
       virtual core::ErrorCode create_circuit(
           const Node* hops,
           size_t hop_count,
           Circuit** out_circuit
       ) = 0;
       
       // Send data through a circuit (applies encryption layers)
       virtual core::ErrorCode send_onion(
           Circuit* circuit,
           const core::Buffer& plaintext,
           core::Buffer& onion_packet
       ) = 0;
       
       // Receive and decrypt one layer (for relay nodes)
       virtual core::ErrorCode peel_layer(
           const core::Buffer& onion_packet,
           core::Buffer& inner_packet,
           Node** next_hop
       ) = 0;
       
       // Destroy a circuit
       virtual core::ErrorCode destroy_circuit(Circuit* circuit) = 0;
       
       // Bridge mode: act as a relay node
       virtual core::ErrorCode enable_bridge_mode() = 0;
       virtual core::ErrorCode disable_bridge_mode() = 0;
       virtual bool is_bridge_mode() const = 0;
   };
   
   } // namespace onion
   } // namespace federated
   ```

2. **Circuit and Node Structures**
   ```cpp
   // Circuit representation
   struct Circuit {
       uint32_t circuit_id;
       Node* hops;
       size_t hop_count;
       crypto::Crypto** crypto_layers;  // One per hop
       uint8_t* session_keys;           // Symmetric keys for each hop
       transport::Transport* transport;
       bool is_active;
   };
   
   // Node in the network
   struct Node {
       char address[256];        // Transport-specific address
       uint16_t port;
       uint8_t public_key[32];   // For key exchange (future: actual public key crypto)
       uint32_t node_id;
   };
   ```

3. **Encryption Layering**
   - For N hops: plaintext → E_N(...E_2(E_1(plaintext)))
   - Each relay peels one layer: E_N(...E_2(E_1(msg))) → E_N(...E_2(msg))
   - Final destination receives plaintext
   - Use ChaCha20 for each layer (already implemented)

4. **Circuit Construction**
   - Build circuit incrementally: A → B → C
   - Step 1: Establish session with A
   - Step 2: Through A, establish session with B
   - Step 3: Through A→B, establish session with C
   - Each step negotiates a symmetric key (simplified: pre-shared for now)

**Test Requirements:**
- [ ] Circuit creation and destruction
- [ ] Encryption layering (3-hop circuit)
- [ ] Relay node behavior (peel one layer)
- [ ] Bridge mode enable/disable
- [ ] Error handling (node unreachable, circuit failure)
- [ ] Scenario test: End-to-end onion routing

**Estimated Effort:** 5-7 days

**Success Criteria:**
- ✅ 3-hop circuit successfully established
- ✅ Data sent through circuit and received at destination
- ✅ Intermediate nodes cannot read plaintext
- ✅ 100% test coverage
- ✅ No security vulnerabilities

---

### Task 1.2: Bridge and Relay Mode

**Rationale:** Enables nodes to act as relays in the network, extending reach and anonymity.

**Implementation Requirements:**

1. **Bridge Mode Interface**
   - Accept incoming circuit extension requests
   - Forward encrypted packets to next hop
   - Peel one encryption layer per hop
   - No access to plaintext (only sees encrypted next layer)

2. **Relay Protocol**
   ```cpp
   // When receiving onion packet as a relay:
   // 1. Peel one encryption layer
   // 2. Extract next hop address
   // 3. Forward inner packet to next hop
   // 4. Return any response back through circuit
   ```

3. **Circuit Extension**
   - Client extends circuit: A → B, then A→B → C
   - Each extension uses existing circuit to reach next hop
   - Protocol: EXTEND command with next hop info

**Test Requirements:**
- [ ] Bridge mode accepts and forwards packets
- [ ] Relay correctly peels encryption layer
- [ ] Circuit extension through existing circuit
- [ ] Backward compatibility (works with non-bridge nodes)

**Estimated Effort:** 3-4 days

---

## Priority 2: HIGH - Federation Protocol

### Task 2.1: Peer Discovery and Registry

**Rationale:** Enables nodes to discover peers and maintain network topology knowledge.

**Implementation Requirements:**

1. **Federation Interface**
   ```cpp
   // include/federated/federation/federation.h
   namespace federated {
   namespace federation {
   
   /**
    * Federation Protocol
    * 
    * Manages peer discovery, registry, and coordination in a
    * decentralized network. Supports multiple discovery mechanisms.
    * 
    * Discovery Methods:
    * - Bootstrap nodes (hardcoded initial peers)
    * - Local network broadcast
    * - Directory service
    * - Peer exchange (PEX)
    * 
    * References:
    * - BitTorrent DHT: BEP 5
    * - Kademlia: Maymounkov & Mazières (2002)
    */
   class Federation {
   public:
       virtual ~Federation() = default;
       
       // Initialize federation with bootstrap nodes
       virtual core::ErrorCode initialize(
           const char** bootstrap_nodes,
           size_t node_count
       ) = 0;
       
       // Discover peers in the network
       virtual core::ErrorCode discover_peers(
           Peer** peers,
           size_t* peer_count,
           size_t max_peers
       ) = 0;
       
       // Register this node as available
       virtual core::ErrorCode register_node(
           const NodeInfo& info
       ) = 0;
       
       // Send message to a peer
       virtual core::ErrorCode send_to_peer(
           const Peer& peer,
           const core::Buffer& message
       ) = 0;
       
       // Broadcast message to all known peers
       virtual core::ErrorCode broadcast(
           const core::Buffer& message
       ) = 0;
       
       // Get list of known peers
       virtual core::ErrorCode get_peers(
           Peer** peers,
           size_t* peer_count
       ) = 0;
   };
   
   } // namespace federation
   } // namespace federated
   ```

2. **Peer Structure**
   ```cpp
   struct Peer {
       uint32_t peer_id;
       char address[256];
       uint16_t port;
       uint64_t last_seen;       // Timestamp
       uint32_t latency_ms;      // Network latency
       uint8_t capabilities;     // Bitmask: bridge, relay, directory, etc.
       bool is_trusted;
   };
   
   struct NodeInfo {
       char address[256];
       uint16_t port;
       uint8_t capabilities;
       char version[32];
   };
   ```

3. **Discovery Mechanisms**
   - **Bootstrap**: Hardcoded list of initial nodes
   - **Broadcast**: UDP broadcast on local network
   - **Directory**: Query central/federated directory service
   - **PEX**: Peer exchange (ask peers for their peers)

**Test Requirements:**
- [ ] Initialize with bootstrap nodes
- [ ] Discover peers via broadcast
- [ ] Register node in directory
- [ ] Send message to specific peer
- [ ] Broadcast to all peers
- [ ] Peer list management

**Estimated Effort:** 4-5 days

---

### Task 2.2: Directory Service

**Rationale:** Provides a federated directory for node discovery (optional component).

**Implementation Requirements:**

1. **Directory Server**
   - HTTP-based API for node registration and lookup
   - Simple key-value store: node_id → NodeInfo
   - TTL-based expiration (nodes must re-register)
   - Query by capability (find all bridges, relays, etc.)

2. **Directory Client**
   - Register with directory on startup
   - Periodic heartbeat to maintain registration
   - Query directory for peers by capability
   - Fallback if directory unavailable

3. **Directory Protocol** (JSON over HTTP)
   ```
   POST /register
   {
       "node_id": "abc123",
       "address": "192.168.1.100",
       "port": 9050,
       "capabilities": ["relay", "bridge"],
       "version": "0.1.0"
   }
   
   GET /peers?capability=relay&limit=10
   {
       "peers": [
           {"node_id": "...", "address": "...", ...},
           ...
       ]
   }
   ```

**Test Requirements:**
- [ ] Directory server stores and retrieves nodes
- [ ] TTL expiration removes stale nodes
- [ ] Client registers successfully
- [ ] Client queries and receives peer list
- [ ] Graceful handling of directory unavailability

**Estimated Effort:** 3-4 days

---

## Priority 3: MEDIUM - Integration and Scenarios

### Task 3.1: End-to-End Scenarios

**Rationale:** Validate that all layers work together correctly.

**Scenarios to Implement:**

1. **Scenario: Anonymous File Transfer**
   - Node A sends file to Node D through B and C
   - Circuit: A → B → C → D
   - File encrypted at each hop
   - D receives file without B or C knowing content

2. **Scenario: Peer Discovery and Messaging**
   - Node A discovers peers via directory
   - Node A sends message to Node B
   - Node B receives and responds
   - Message routing through federation protocol

3. **Scenario: Bridge Network**
   - 5 nodes: 2 clients, 3 bridges
   - Clients discover bridges
   - Clients create circuits through bridges
   - Data flows through bridge network

4. **Scenario: Federated Directory**
   - Multiple directory servers
   - Nodes register with multiple directories
   - Client queries and aggregates results
   - Handles directory failures gracefully

**Test Requirements:**
- [ ] All scenarios pass
- [ ] No data leakage to intermediate nodes
- [ ] Proper error handling and recovery
- [ ] Performance acceptable (circuit creation < 1s)

**Estimated Effort:** 4-5 days

---

## Priority 4: LOW - Advanced Features (Optional for Phase 5)

### Task 4.1: Advanced Circuit Management

**Features:**
- Circuit pooling (reuse circuits)
- Circuit expiration and renewal
- Circuit failure detection and recovery
- Multi-path routing (parallel circuits)

**Estimated Effort:** 3-4 days

---

### Task 4.2: Traffic Analysis Resistance

**Features:**
- Constant-rate traffic (dummy packets)
- Traffic shaping and timing obfuscation
- Circuit padding
- Cover traffic generation

**Estimated Effort:** 4-5 days

---

## Documentation Tasks

### Task D.1: Update README.md

**Changes Required:**
1. Add Phase 5 status section
2. Add onion routing example to Quick Start
3. Add federation example to Quick Start
4. Update test count metrics
5. Update architecture diagram

---

### Task D.2: Update docs/STATUS.md

**Changes Required:**
1. Add onion routing layer section
2. Add federation protocol section
3. Update test counts
4. Update completion percentages

---

### Task D.3: Create Supporting Documentation

**Files to Create:**
- ✅ `docs/PHASE5_ROADMAP.md` (this file)
- 📝 `docs/ONION_ROUTING.md` (technical deep-dive)
- 📝 `docs/FEDERATION_PROTOCOL.md` (protocol specification)

---

## Implementation Guidelines

All Phase 5 implementations MUST follow project standards:

### Test-First Development
- Write failing tests before implementation
- Minimum 100% coverage for critical paths
- Include edge cases and error conditions
- Security-focused test scenarios

### Zero Dependencies
- Only C++17 standard library
- Platform APIs via syscalls only
- Custom implementations for all algorithms

### Flat Architecture
- No deep inheritance hierarchies
- Explicit error handling (no exceptions)
- Clear ownership and state management

### Security Considerations
- Constant-time operations where applicable
- No metadata leakage to intermediate nodes
- Proper key isolation between circuit layers
- Resistance to traffic analysis (where feasible)

---

## Success Criteria

### Phase 5 Completion Metrics

1. **Functionality**
   - ✅ Onion router creates 3-hop circuits
   - ✅ Bridge mode successfully relays traffic
   - ✅ Federation discovers and registers peers
   - ✅ Directory service operational (if implemented)
   - ✅ All end-to-end scenarios pass

2. **Quality**
   - ✅ Zero security vulnerabilities (CodeQL clean)
   - ✅ All tests passing (unit + scenario + E2E)
   - ✅ 100% test coverage maintained
   - ✅ Build time < 20 seconds
   - ✅ Zero compiler warnings

3. **Documentation**
   - ✅ README.md updated with Phase 5 status
   - ✅ docs/STATUS.md synchronized
   - ✅ Code examples for onion routing and federation
   - ✅ Technical documentation complete

4. **Performance**
   - ✅ Circuit creation: < 1 second (3-hop)
   - ✅ Onion encryption overhead: < 10% vs direct
   - ✅ Peer discovery: < 5 seconds
   - ✅ Directory query: < 500ms

---

## Timeline Estimate

**Conservative Estimate:** 20-25 days (4-5 weeks)

| Task | Effort | Dependencies |
|------|--------|--------------|
| Onion Router Core | 5-7 days | None |
| Bridge/Relay Mode | 3-4 days | Onion Router Core |
| Federation Protocol | 4-5 days | None |
| Directory Service | 3-4 days | Federation Protocol |
| Integration Scenarios | 4-5 days | All above |
| Documentation | 2-3 days | All implementations |
| Testing & Security Review | 2-3 days | All implementations |

**Aggressive Estimate:** 14-18 days (3 weeks) if focused

---

## Security Considerations

### Onion Routing Security

1. **Traffic Analysis**
   - Timing attacks possible (not fully mitigated in Phase 5)
   - Packet size correlation can leak information
   - Mitigation: Consider constant-rate traffic (Phase 6)

2. **Circuit Compromise**
   - If all nodes in circuit are compromised, anonymity lost
   - Mitigation: Use diverse, trusted nodes
   - Future: Implement guard nodes and node reputation

3. **Metadata Leakage**
   - Circuit construction reveals network topology to directory
   - Mitigation: Multiple directories, encrypted queries (future)

### Federation Security

1. **Sybil Attacks**
   - Attacker creates many fake nodes to control routing
   - Mitigation: Node reputation, proof-of-work (future)

2. **Eclipse Attacks**
   - Attacker isolates victim from honest nodes
   - Mitigation: Multiple bootstrap nodes, diverse discovery

3. **Directory Attacks**
   - Malicious directory can provide fake nodes
   - Mitigation: Multiple directories, cross-validation

---

## Notes and Considerations

### Onion Routing

- **Simplified Implementation**: Phase 5 uses symmetric encryption (ChaCha20) for all layers, not full Diffie-Hellman key exchange
- **Future Enhancement**: Phase 6+ will add proper public key crypto for circuit setup
- **Circuit Lifetime**: Circuits expire after inactivity or on error
- **No Traffic Padding**: Phase 5 does not implement cover traffic or timing obfuscation

### Federation

- **Bootstrap Dependency**: Initial network requires bootstrap nodes or directory
- **Centralization Trade-off**: Directory service is optional but adds centralization
- **Discovery Latency**: Initial peer discovery may take several seconds
- **Network Topology**: Assumes IP-based networking (can use any transport)

### Performance

- **Circuit Overhead**: Each hop adds latency and encryption overhead
- **Optimal Hop Count**: 3 hops is standard (balance between anonymity and performance)
- **Throughput**: Limited by slowest hop in circuit
- **CPU Usage**: Multiple encryption layers are CPU-intensive

---

## References

### Academic Papers
- Goldschlag, Reed, Syverson - "Onion Routing for Anonymous and Private Internet Connections" (1999)
- Dingledine, Mathewson, Syverson - "Tor: The Second-Generation Onion Router" (2004)
- Maymounkov & Mazières - "Kademlia: A Peer-to-peer Information System Based on the XOR Metric" (2002)

### Specifications
- Tor Project - Tor Protocol Specification: https://spec.torproject.org/
- BitTorrent - DHT Protocol (BEP 5): http://www.bittorrent.org/beps/bep_0005.html

### Project Documentation
- `README.md` - Project overview
- `docs/STATUS.md` - Implementation status
- `docs/PHASE4_ROADMAP.md` - Previous phase roadmap
- `docs/architecture.md` - System architecture
- `.github/agents/copilot-instructions.md` - Development guidelines

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-27 | 1.0 | Initial Phase 5 roadmap created |

---

**Next Steps:**
1. Review and approve this roadmap
2. Begin with Onion Router core implementation
3. Create federation protocol in parallel
4. Integrate and test end-to-end scenarios
5. Update documentation and metrics

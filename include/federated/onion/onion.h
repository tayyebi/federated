#ifndef FEDERATED_ONION_ONION_H
#define FEDERATED_ONION_ONION_H

/**
 * Onion Routing Layer
 * 
 * Provides anonymous multi-hop routing through layered encryption.
 * Each hop in a circuit adds or removes one encryption layer.
 * 
 * Design inspired by Tor but simplified for zero-dependency implementation:
 * - No directory authorities (uses federation layer)
 * - Symmetric encryption only (ChaCha20 per layer)
 * - Simplified circuit construction
 * 
 * References:
 * - Tor Design: https://spec.torproject.org/tor-spec
 * - Goldschlag, Reed, Syverson - "Onion Routing" (1999)
 * - Dingledine et al - "Tor: The Second-Generation Onion Router" (2004)
 * 
 * Architecture:
 * 
 *   Client                                    Destination
 *     |                                            |
 *     |---> Node A (peel layer 1) --->            |
 *              |                                   |
 *              |---> Node B (peel layer 2) --->   |
 *                       |                          |
 *                       |---> Node C (peel layer 3) --->
 * 
 * Encryption layers (for 3-hop circuit A->B->C):
 *   Plaintext -> E_C(E_B(E_A(plaintext)))
 *   At A: E_C(E_B(E_A(msg))) -> E_C(E_B(msg))
 *   At B: E_C(E_B(msg)) -> E_C(msg)
 *   At C: E_C(msg) -> msg (plaintext)
 */

#include "federated/core/buffer.h"
#include "federated/core/error.h"
#include "federated/crypto/crypto.h"
#include "federated/transport/transport.h"

namespace federated {
namespace onion {

// Forward declarations
struct Circuit;
struct Node;
struct OnionPacket;

/**
 * Node in the onion routing network
 */
struct Node {
    uint32_t node_id;           // Unique identifier
    char address[256];          // Transport-specific address (IP, hostname, etc.)
    uint16_t port;              // Port number
    uint8_t session_key[32];    // Symmetric key for this hop (pre-shared for now)
    uint8_t capabilities;       // Bitmask: relay, bridge, exit, etc.
    
    Node() : node_id(0), port(0), capabilities(0) {
        address[0] = '\0';
        for (size_t i = 0; i < 32; i++) session_key[i] = 0;
    }
};

// Node capability flags
constexpr uint8_t NODE_CAPABILITY_RELAY = 0x01;  // Can relay traffic
constexpr uint8_t NODE_CAPABILITY_BRIDGE = 0x02; // Can act as bridge (first hop)
constexpr uint8_t NODE_CAPABILITY_EXIT = 0x04;   // Can be exit node (last hop)
constexpr uint8_t NODE_CAPABILITY_DIRECTORY = 0x08; // Provides directory service

/**
 * Circuit through the onion network
 */
struct Circuit {
    uint32_t circuit_id;              // Unique identifier
    Node* hops;                       // Array of nodes in the circuit
    size_t hop_count;                 // Number of hops
    crypto::Crypto** crypto_layers;   // Encryption for each hop
    transport::Transport* transport;  // Underlying transport
    bool is_active;                   // Circuit state
    uint64_t created_at;              // Timestamp (for expiration)
    
    Circuit() : circuit_id(0), hops(nullptr), hop_count(0), 
                crypto_layers(nullptr), transport(nullptr), 
                is_active(false), created_at(0) {}
};

/**
 * Onion packet structure
 * 
 * Format:
 * [ Next Hop Info (encrypted) | Payload (encrypted) ]
 * 
 * Each relay peels one layer to reveal next hop and inner payload.
 */
struct OnionPacket {
    uint8_t* data;           // Encrypted packet data
    size_t size;             // Total packet size
    uint32_t circuit_id;     // Circuit this packet belongs to
    
    OnionPacket() : data(nullptr), size(0), circuit_id(0) {}
};

/**
 * Onion Router Interface
 * 
 * Provides anonymous multi-hop routing with layered encryption.
 */
class OnionRouter {
public:
    virtual ~OnionRouter() = default;
    
    /**
     * Create a new circuit through specified hops
     * 
     * @param hops Array of nodes to use in the circuit
     * @param hop_count Number of nodes in the circuit (typically 3)
     * @param out_circuit Pointer to receive created circuit
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode create_circuit(
        const Node* hops,
        size_t hop_count,
        Circuit** out_circuit
    ) = 0;
    
    /**
     * Send data through a circuit with layered encryption
     * 
     * Applies encryption layers from inside out:
     * - Last hop layer first (innermost)
     * - First hop layer last (outermost)
     * 
     * @param circuit Circuit to use
     * @param plaintext Data to send
     * @param onion_packet Output encrypted packet
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode send_onion(
        Circuit* circuit,
        const core::Buffer& plaintext,
        OnionPacket& onion_packet
    ) = 0;
    
    /**
     * Peel one encryption layer (for relay nodes)
     * 
     * Decrypts the outermost layer and extracts:
     * - Next hop information
     * - Inner encrypted packet
     * 
     * @param onion_packet Incoming encrypted packet
     * @param inner_packet Output packet with one layer removed
     * @param next_hop Output next hop in the circuit
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode peel_layer(
        const OnionPacket& onion_packet,
        OnionPacket& inner_packet,
        Node* next_hop
    ) = 0;
    
    /**
     * Destroy a circuit and free resources
     * 
     * @param circuit Circuit to destroy
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode destroy_circuit(Circuit* circuit) = 0;
    
    /**
     * Enable bridge/relay mode
     * 
     * When enabled, this node will accept and forward onion packets.
     */
    virtual core::ErrorCode enable_bridge_mode() = 0;
    
    /**
     * Disable bridge/relay mode
     */
    virtual core::ErrorCode disable_bridge_mode() = 0;
    
    /**
     * Check if bridge mode is enabled
     */
    virtual bool is_bridge_mode() const = 0;
};

/**
 * Simple onion router implementation
 */
class SimpleOnionRouter : public OnionRouter {
private:
    bool bridge_mode_enabled;
    Circuit** active_circuits;
    size_t circuit_capacity;
    size_t circuit_count;
    uint32_t next_circuit_id;
    
public:
    SimpleOnionRouter();
    virtual ~SimpleOnionRouter();
    
    core::ErrorCode create_circuit(
        const Node* hops,
        size_t hop_count,
        Circuit** out_circuit
    ) override;
    
    core::ErrorCode send_onion(
        Circuit* circuit,
        const core::Buffer& plaintext,
        OnionPacket& onion_packet
    ) override;
    
    core::ErrorCode peel_layer(
        const OnionPacket& onion_packet,
        OnionPacket& inner_packet,
        Node* next_hop
    ) override;
    
    core::ErrorCode destroy_circuit(Circuit* circuit) override;
    
    core::ErrorCode enable_bridge_mode() override;
    core::ErrorCode disable_bridge_mode() override;
    bool is_bridge_mode() const override;
    
    /**
     * Get singleton instance
     */
    static OnionRouter* get_instance();
};

} // namespace onion
} // namespace federated

#endif // FEDERATED_ONION_ONION_H

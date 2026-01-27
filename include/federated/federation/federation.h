#ifndef FEDERATED_FEDERATION_FEDERATION_H
#define FEDERATED_FEDERATION_FEDERATION_H

/**
 * Federation Protocol
 * 
 * Manages peer discovery, registry, and coordination in a decentralized network.
 * Supports multiple discovery mechanisms for resilience.
 * 
 * Discovery Methods:
 * - Bootstrap nodes (hardcoded initial peers)
 * - Local network broadcast (UDP)
 * - Directory service (centralized/federated)
 * - Peer exchange (PEX) - ask peers for their peers
 * 
 * References:
 * - BitTorrent DHT: BEP 5
 * - Kademlia: Maymounkov & Mazières (2002)
 * - Gnutella Protocol Specification
 * 
 * Design Principles:
 * - Decentralized where possible
 * - Multiple discovery methods for resilience
 * - Graceful degradation
 * - Zero external dependencies
 */

#include "federated/core/buffer.h"
#include "federated/core/error.h"

namespace federated {
namespace federation {

// Forward declarations
struct Peer;
struct NodeInfo;

/**
 * Peer in the federated network
 */
struct Peer {
    uint32_t peer_id;           // Unique identifier
    char address[256];          // Network address (IP, hostname, etc.)
    uint16_t port;              // Port number
    uint64_t last_seen;         // Timestamp of last contact
    uint32_t latency_ms;        // Network latency in milliseconds
    uint8_t capabilities;       // Capability bitmask
    bool is_trusted;            // Trust flag
    
    Peer() : peer_id(0), port(0), last_seen(0), latency_ms(0), 
             capabilities(0), is_trusted(false) {
        address[0] = '\0';
    }
};

// Peer capability flags
constexpr uint8_t PEER_CAPABILITY_RELAY = 0x01;     // Can relay onion traffic
constexpr uint8_t PEER_CAPABILITY_BRIDGE = 0x02;    // Can act as bridge
constexpr uint8_t PEER_CAPABILITY_EXIT = 0x04;      // Can be exit node
constexpr uint8_t PEER_CAPABILITY_DIRECTORY = 0x08; // Provides directory service
constexpr uint8_t PEER_CAPABILITY_STORAGE = 0x10;   // Provides storage service

/**
 * Node information (for registration)
 */
struct NodeInfo {
    char address[256];          // This node's address
    uint16_t port;              // This node's port
    uint8_t capabilities;       // This node's capabilities
    char version[32];           // Protocol version
    
    NodeInfo() : port(0), capabilities(0) {
        address[0] = '\0';
        version[0] = '\0';
    }
};

/**
 * Federation Interface
 * 
 * Manages peer discovery and coordination in the network.
 */
class Federation {
public:
    virtual ~Federation() = default;
    
    /**
     * Initialize federation with bootstrap nodes
     * 
     * @param bootstrap_nodes Array of bootstrap node addresses
     * @param node_count Number of bootstrap nodes
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode initialize(
        const char** bootstrap_nodes,
        size_t node_count
    ) = 0;
    
    /**
     * Discover peers in the network
     * 
     * Uses all available discovery methods:
     * - Query bootstrap nodes
     * - Local network broadcast
     * - Directory service (if available)
     * - Peer exchange
     * 
     * @param peers Output array of discovered peers
     * @param peer_count Output number of peers discovered
     * @param max_peers Maximum peers to return
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode discover_peers(
        Peer* peers,
        size_t* peer_count,
        size_t max_peers
    ) = 0;
    
    /**
     * Register this node as available in the network
     * 
     * @param info Information about this node
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode register_node(
        const NodeInfo& info
    ) = 0;
    
    /**
     * Send message to a specific peer
     * 
     * @param peer Target peer
     * @param message Message to send
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode send_to_peer(
        const Peer& peer,
        const core::Buffer& message
    ) = 0;
    
    /**
     * Broadcast message to all known peers
     * 
     * @param message Message to broadcast
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode broadcast(
        const core::Buffer& message
    ) = 0;
    
    /**
     * Get list of known peers
     * 
     * @param peers Output array to fill with peers
     * @param peer_count Output number of peers
     * @param max_peers Maximum peers to return
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode get_peers(
        Peer* peers,
        size_t* peer_count,
        size_t max_peers
    ) = 0;
    
    /**
     * Add a peer manually
     * 
     * @param peer Peer to add
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode add_peer(
        const Peer& peer
    ) = 0;
    
    /**
     * Remove a peer
     * 
     * @param peer_id ID of peer to remove
     * @return OK on success, error code on failure
     */
    virtual core::ErrorCode remove_peer(
        uint32_t peer_id
    ) = 0;
    
    /**
     * Clear all peers (for testing)
     * 
     * @return OK on success
     */
    virtual core::ErrorCode clear_peers() = 0;
};

/**
 * Simple federation implementation
 */
class SimpleFederation : public Federation {
private:
    Peer* peer_list;
    size_t peer_capacity;
    size_t peer_count;
    bool is_initialized;
    NodeInfo local_node_info;
    uint32_t next_peer_id;
    
public:
    SimpleFederation();
    virtual ~SimpleFederation();
    
    core::ErrorCode initialize(
        const char** bootstrap_nodes,
        size_t node_count
    ) override;
    
    core::ErrorCode discover_peers(
        Peer* peers,
        size_t* peer_count,
        size_t max_peers
    ) override;
    
    core::ErrorCode register_node(
        const NodeInfo& info
    ) override;
    
    core::ErrorCode send_to_peer(
        const Peer& peer,
        const core::Buffer& message
    ) override;
    
    core::ErrorCode broadcast(
        const core::Buffer& message
    ) override;
    
    core::ErrorCode get_peers(
        Peer* peers,
        size_t* peer_count,
        size_t max_peers
    ) override;
    
    core::ErrorCode add_peer(
        const Peer& peer
    ) override;
    
    core::ErrorCode remove_peer(
        uint32_t peer_id
    ) override;
    
    core::ErrorCode clear_peers() override;
    
    /**
     * Get singleton instance
     */
    static Federation* get_instance();
};

} // namespace federation
} // namespace federated

#endif // FEDERATED_FEDERATION_FEDERATION_H

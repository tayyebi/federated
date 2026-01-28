#include "federated/federation/federation.h"
#include "../test_runner.h"
#include <cstring>

using namespace federated;
using namespace federated::federation;

// Test: Get federation instance
TEST(federation_get_instance) {
    Federation* fed = SimpleFederation::get_instance();
    ASSERT_NOT_NULL(fed);
    
    // Singleton pattern
    Federation* fed2 = SimpleFederation::get_instance();
    ASSERT_EQUAL(fed, fed2);
}

// Test: Initialize with bootstrap nodes
TEST(federation_initialize) {
    Federation* fed = SimpleFederation::get_instance();
    
    const char* bootstrap[] = {
        "192.168.1.10:9050",
        "192.168.1.11:9051",
        "node.example.com:9052"
    };
    
    core::ErrorCode err = fed->initialize(bootstrap, 3);
    ASSERT_EQUAL(err, core::OK);
}

// Test: Initialize with no bootstrap nodes
TEST(federation_initialize_empty) {
    Federation* fed = SimpleFederation::get_instance();
    
    core::ErrorCode err = fed->initialize(nullptr, 0);
    ASSERT_EQUAL(err, core::OK);
}

// Test: Register local node
TEST(federation_register_node) {
    Federation* fed = SimpleFederation::get_instance();
    
    // Initialize first
    fed->initialize(nullptr, 0);
    
    NodeInfo info;
    strcpy(info.address, "192.168.1.100");
    info.port = 9050;
    info.capabilities = PEER_CAPABILITY_RELAY | PEER_CAPABILITY_BRIDGE;
    strcpy(info.version, "0.1.0");
    
    core::ErrorCode err = fed->register_node(info);
    ASSERT_EQUAL(err, core::OK);
}

// Test: Add peers manually
TEST(federation_add_peer) {
    Federation* fed = SimpleFederation::get_instance();
    
    fed->initialize(nullptr, 0);
    
    // Add a peer
    Peer peer;
    peer.peer_id = 100;
    strcpy(peer.address, "192.168.1.50");
    peer.port = 9050;
    peer.capabilities = PEER_CAPABILITY_RELAY;
    peer.is_trusted = true;
    
    core::ErrorCode err = fed->add_peer(peer);
    ASSERT_EQUAL(err, core::OK);
    
    // Verify peer was added
    Peer peers[10];
    size_t count = 0;
    err = fed->get_peers(peers, &count, 10);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_TRUE(count > 0);
    
    // Find our peer
    bool found = false;
    for (size_t i = 0; i < count; i++) {
        if (peers[i].peer_id == 100) {
            found = true;
            ASSERT_EQUAL(strcmp(peers[i].address, "192.168.1.50"), 0);
            ASSERT_EQUAL(peers[i].port, static_cast<uint16_t>(9050));
            break;
        }
    }
    ASSERT_TRUE(found);
}

// Test: Remove peer
TEST(federation_remove_peer) {
    Federation* fed = SimpleFederation::get_instance();
    
    // Clear any previous state
    fed->clear_peers();
    
    fed->initialize(nullptr, 0);
    
    // Add a peer
    Peer peer;
    peer.peer_id = 200;
    strcpy(peer.address, "192.168.1.60");
    peer.port = 9050;
    
    fed->add_peer(peer);
    
    // Verify it exists
    Peer peers[10];
    size_t count = 0;
    fed->get_peers(peers, &count, 10);
    size_t initial_count = count;
    ASSERT_TRUE(count > 0);
    
    // Remove the peer
    core::ErrorCode err = fed->remove_peer(200);
    ASSERT_EQUAL(err, core::OK);
    
    // Verify it's gone
    fed->get_peers(peers, &count, 10);
    ASSERT_EQUAL(count, initial_count - 1);
    
    // Try to remove non-existent peer
    err = fed->remove_peer(999);
    ASSERT_EQUAL(err, core::ERR_NOT_FOUND);
}

// Test: Get peers list
TEST(federation_get_peers) {
    Federation* fed = SimpleFederation::get_instance();
    
    // Clear any previous state
    fed->clear_peers();
    
    const char* bootstrap[] = {
        "192.168.1.10:9050",
        "192.168.1.11:9051"
    };
    
    fed->initialize(bootstrap, 2);
    
    // Get peers
    Peer peers[10];
    size_t count = 0;
    core::ErrorCode err = fed->get_peers(peers, &count, 10);
    
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(count, static_cast<size_t>(2));
    
    // Verify bootstrap nodes were added
    ASSERT_EQUAL(strcmp(peers[0].address, "192.168.1.10"), 0);
    ASSERT_EQUAL(peers[0].port, static_cast<uint16_t>(9050));
    ASSERT_EQUAL(strcmp(peers[1].address, "192.168.1.11"), 0);
    ASSERT_EQUAL(peers[1].port, static_cast<uint16_t>(9051));
}

// Test: Discover peers
TEST(federation_discover_peers) {
    Federation* fed = SimpleFederation::get_instance();
    
    const char* bootstrap[] = {
        "192.168.1.10:9050"
    };
    
    fed->initialize(bootstrap, 1);
    
    // Discover peers (returns existing for now)
    Peer peers[10];
    size_t count = 0;
    core::ErrorCode err = fed->discover_peers(peers, &count, 10);
    
    ASSERT_EQUAL(err, core::OK);
    ASSERT_TRUE(count > 0);
}

// Test: Broadcast message
TEST(federation_broadcast) {
    Federation* fed = SimpleFederation::get_instance();
    
    const char* bootstrap[] = {
        "192.168.1.10:9050",
        "192.168.1.11:9050"
    };
    
    fed->initialize(bootstrap, 2);
    
    // Broadcast a message
    const char* msg = "Hello, peers!";
    core::Buffer message(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(msg)),
        strlen(msg)
    );
    
    core::ErrorCode err = fed->broadcast(message);
    ASSERT_EQUAL(err, core::OK);
}

// Test: Send to specific peer
TEST(federation_send_to_peer) {
    Federation* fed = SimpleFederation::get_instance();
    
    fed->initialize(nullptr, 0);
    
    // Add a peer
    Peer peer;
    peer.peer_id = 300;
    strcpy(peer.address, "192.168.1.70");
    peer.port = 9050;
    fed->add_peer(peer);
    
    // Send message to peer
    const char* msg = "Direct message";
    core::Buffer message(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(msg)),
        strlen(msg)
    );
    
    core::ErrorCode err = fed->send_to_peer(peer, message);
    ASSERT_EQUAL(err, core::OK);
}

// Test: Peer structure initialization
TEST(federation_peer_structure) {
    Peer peer;
    
    // Default values
    ASSERT_EQUAL(peer.peer_id, static_cast<uint32_t>(0));
    ASSERT_EQUAL(peer.port, static_cast<uint16_t>(0));
    ASSERT_EQUAL(peer.last_seen, static_cast<uint64_t>(0));
    ASSERT_EQUAL(peer.latency_ms, static_cast<uint32_t>(0));
    ASSERT_EQUAL(peer.capabilities, static_cast<uint8_t>(0));
    ASSERT_FALSE(peer.is_trusted);
    
    // Set values
    peer.peer_id = 42;
    strcpy(peer.address, "example.com");
    peer.port = 8080;
    peer.capabilities = PEER_CAPABILITY_RELAY | PEER_CAPABILITY_EXIT;
    peer.is_trusted = true;
    
    ASSERT_EQUAL(peer.peer_id, static_cast<uint32_t>(42));
    ASSERT_EQUAL(strcmp(peer.address, "example.com"), 0);
    ASSERT_EQUAL(peer.port, static_cast<uint16_t>(8080));
    ASSERT_TRUE(peer.capabilities & PEER_CAPABILITY_RELAY);
    ASSERT_TRUE(peer.capabilities & PEER_CAPABILITY_EXIT);
    ASSERT_TRUE(peer.is_trusted);
}

// Test: NodeInfo structure
TEST(federation_nodeinfo_structure) {
    NodeInfo info;
    
    // Default values
    ASSERT_EQUAL(info.port, static_cast<uint16_t>(0));
    ASSERT_EQUAL(info.capabilities, static_cast<uint8_t>(0));
    
    // Set values
    strcpy(info.address, "localhost");
    info.port = 9050;
    info.capabilities = PEER_CAPABILITY_DIRECTORY;
    strcpy(info.version, "1.0.0");
    
    ASSERT_EQUAL(strcmp(info.address, "localhost"), 0);
    ASSERT_EQUAL(info.port, static_cast<uint16_t>(9050));
    ASSERT_TRUE(info.capabilities & PEER_CAPABILITY_DIRECTORY);
    ASSERT_EQUAL(strcmp(info.version, "1.0.0"), 0);
}

// Test: Multiple peers with different capabilities
TEST(federation_peer_capabilities) {
    Federation* fed = SimpleFederation::get_instance();
    
    // Clear any previous state
    fed->clear_peers();
    
    fed->initialize(nullptr, 0);
    
    // Add relay peer
    Peer relay_peer;
    relay_peer.peer_id = 1;
    relay_peer.capabilities = PEER_CAPABILITY_RELAY;
    fed->add_peer(relay_peer);
    
    // Add bridge peer
    Peer bridge_peer;
    bridge_peer.peer_id = 2;
    bridge_peer.capabilities = PEER_CAPABILITY_BRIDGE;
    fed->add_peer(bridge_peer);
    
    // Add exit peer
    Peer exit_peer;
    exit_peer.peer_id = 3;
    exit_peer.capabilities = PEER_CAPABILITY_EXIT;
    fed->add_peer(exit_peer);
    
    // Get all peers
    Peer peers[10];
    size_t count = 0;
    fed->get_peers(peers, &count, 10);
    
    ASSERT_TRUE(count >= 3);
    
    // Verify capabilities
    int relay_count = 0;
    int bridge_count = 0;
    int exit_count = 0;
    
    for (size_t i = 0; i < count; i++) {
        if (peers[i].capabilities & PEER_CAPABILITY_RELAY) relay_count++;
        if (peers[i].capabilities & PEER_CAPABILITY_BRIDGE) bridge_count++;
        if (peers[i].capabilities & PEER_CAPABILITY_EXIT) exit_count++;
    }
    
    ASSERT_TRUE(relay_count >= 1);
    ASSERT_TRUE(bridge_count >= 1);
    ASSERT_TRUE(exit_count >= 1);
}

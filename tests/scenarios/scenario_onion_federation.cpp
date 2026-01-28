#include "federated/onion/onion.h"
#include "federated/federation/federation.h"
#include "../test_runner.h"
#include <cstring>

using namespace federated;

/**
 * Scenario: Anonymous Communication Through Onion Network
 * 
 * This scenario demonstrates:
 * 1. Federation discovers peers
 * 2. Client selects nodes from peer list
 * 3. Client creates onion circuit through selected nodes
 * 4. Client sends encrypted message through circuit
 * 5. Message is protected from intermediate nodes
 */
SCENARIO(anonymous_onion_communication) {
    // Step 1: Initialize federation with bootstrap nodes
    federation::Federation* fed = federation::SimpleFederation::get_instance();
    
    const char* bootstrap[] = {
        "192.168.1.10:9050",
        "192.168.1.11:9050",
        "192.168.1.12:9050"
    };
    
    core::ErrorCode err = fed->initialize(bootstrap, 3);
    ASSERT_EQUAL(err, core::OK);
    
    // Step 2: Discover available peers
    federation::Peer peers[10];
    size_t peer_count = 0;
    err = fed->discover_peers(peers, &peer_count, 10);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_TRUE(peer_count >= 3);
    
    // Step 3: Select nodes for onion circuit (use first 3 peers)
    onion::Node hops[3];
    for (size_t i = 0; i < 3; i++) {
        hops[i].node_id = peers[i].peer_id;
        strcpy(hops[i].address, peers[i].address);
        hops[i].port = peers[i].port;
        hops[i].capabilities = onion::NODE_CAPABILITY_RELAY;
        
        // In real implementation, session keys would be negotiated
        // For testing, use predictable keys
        for (int j = 0; j < 32; j++) {
            hops[i].session_key[j] = static_cast<uint8_t>((i + 1) * 13 + j);
        }
    }
    
    // Step 4: Create onion circuit
    onion::OnionRouter* router = onion::SimpleOnionRouter::get_instance();
    onion::Circuit* circuit = nullptr;
    err = router->create_circuit(hops, 3, &circuit);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(circuit);
    ASSERT_TRUE(circuit->is_active);
    
    // Step 5: Send secret message through circuit
    const char* secret = "This is a confidential message";
    core::Buffer plaintext(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(secret)),
        strlen(secret)
    );
    
    onion::OnionPacket packet;
    err = router->send_onion(circuit, plaintext, packet);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(packet.data);
    ASSERT_TRUE(packet.size > 0);
    
    // Step 6: Verify encryption (packet should not contain plaintext)
    // Search for plaintext substring in encrypted packet
    bool plaintext_found = false;
    size_t secret_len = strlen(secret);
    
    // Only search if packet is large enough to contain the plaintext
    if (packet.size >= secret_len) {
        for (size_t i = 0; i <= packet.size - secret_len; i++) {
            if (memcmp(packet.data + i, secret, secret_len) == 0) {
                plaintext_found = true;
                break;
            }
        }
    }
    
    ASSERT_FALSE(plaintext_found); // Plaintext should be encrypted
    
    // Step 7: Clean up
    if (packet.data) {
        delete[] packet.data;
    }
    router->destroy_circuit(circuit);
}

/**
 * Scenario: Bridge Network Discovery
 * 
 * Demonstrates:
 * 1. Multiple nodes register with federation
 * 2. Some nodes enable bridge mode
 * 3. Client discovers bridge nodes
 * 4. Client creates circuits through bridges
 */
SCENARIO(bridge_network_discovery) {
    federation::Federation* fed = federation::SimpleFederation::get_instance();
    
    // Initialize with empty bootstrap
    fed->initialize(nullptr, 0);
    
    // Register local node
    federation::NodeInfo local;
    strcpy(local.address, "192.168.1.100");
    local.port = 9050;
    local.capabilities = federation::PEER_CAPABILITY_RELAY;
    strcpy(local.version, "0.1.0");
    fed->register_node(local);
    
    // Add several bridge nodes
    for (int i = 0; i < 5; i++) {
        federation::Peer peer;
        peer.peer_id = static_cast<uint32_t>(100 + i);
        snprintf(peer.address, sizeof(peer.address), "192.168.1.%d", 150 + i);
        peer.port = 9050;
        peer.capabilities = federation::PEER_CAPABILITY_BRIDGE | 
                           federation::PEER_CAPABILITY_RELAY;
        peer.is_trusted = true;
        
        fed->add_peer(peer);
    }
    
    // Discover peers
    federation::Peer peers[20];
    size_t count = 0;
    core::ErrorCode err = fed->get_peers(peers, &count, 20);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_TRUE(count >= 5);
    
    // Filter for bridge-capable nodes
    int bridge_count = 0;
    onion::Node bridge_hops[5];
    
    for (size_t i = 0; i < count && bridge_count < 5; i++) {
        if (peers[i].capabilities & federation::PEER_CAPABILITY_BRIDGE) {
            bridge_hops[bridge_count].node_id = peers[i].peer_id;
            strcpy(bridge_hops[bridge_count].address, peers[i].address);
            bridge_hops[bridge_count].port = peers[i].port;
            bridge_hops[bridge_count].capabilities = onion::NODE_CAPABILITY_BRIDGE;
            bridge_count++;
        }
    }
    
    ASSERT_TRUE(bridge_count >= 3); // Need at least 3 for circuit
    
    // Create circuit through bridges
    onion::OnionRouter* router = onion::SimpleOnionRouter::get_instance();
    
    // Set session keys
    for (int i = 0; i < bridge_count && i < 3; i++) {
        for (int j = 0; j < 32; j++) {
            bridge_hops[i].session_key[j] = static_cast<uint8_t>((i + 1) * 19 + j);
        }
    }
    
    onion::Circuit* circuit = nullptr;
    err = router->create_circuit(bridge_hops, 3, &circuit);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(circuit);
    
    // Verify all hops are bridge-capable
    for (size_t i = 0; i < circuit->hop_count; i++) {
        ASSERT_TRUE(circuit->hops[i].capabilities & onion::NODE_CAPABILITY_BRIDGE);
    }
    
    router->destroy_circuit(circuit);
}

/**
 * Scenario: Peer Exchange and Network Growth
 * 
 * Demonstrates:
 * 1. Start with small number of bootstrap nodes
 * 2. Query those nodes for their peers
 * 3. Add discovered peers to local registry
 * 4. Network knowledge grows organically
 */
SCENARIO(peer_exchange_network_growth) {
    federation::Federation* fed = federation::SimpleFederation::get_instance();
    
    // Clear any previous state
    fed->clear_peers();
    
    // Start with just 2 bootstrap nodes
    const char* bootstrap[] = {
        "192.168.1.10:9050",
        "192.168.1.11:9050"
    };
    
    fed->initialize(bootstrap, 2);
    
    // Initial peer count
    federation::Peer initial_peers[10];
    size_t initial_count = 0;
    fed->get_peers(initial_peers, &initial_count, 10);
    ASSERT_EQUAL(initial_count, static_cast<size_t>(2));
    
    // Simulate peer exchange: add peers learned from others
    federation::Peer new_peers[3];
    for (int i = 0; i < 3; i++) {
        new_peers[i].peer_id = static_cast<uint32_t>(200 + i);
        snprintf(new_peers[i].address, sizeof(new_peers[i].address), 
                 "192.168.1.%d", 20 + i);
        new_peers[i].port = 9050;
        new_peers[i].capabilities = federation::PEER_CAPABILITY_RELAY;
        
        fed->add_peer(new_peers[i]);
    }
    
    // Verify network has grown
    federation::Peer all_peers[20];
    size_t total_count = 0;
    fed->get_peers(all_peers, &total_count, 20);
    ASSERT_EQUAL(total_count, static_cast<size_t>(5)); // 2 bootstrap + 3 new
    
    // Can now create longer circuits with more node choices
    onion::Node hops[5];
    for (size_t i = 0; i < 5 && i < total_count; i++) {
        hops[i].node_id = all_peers[i].peer_id;
        strcpy(hops[i].address, all_peers[i].address);
        hops[i].port = all_peers[i].port;
        
        for (int j = 0; j < 32; j++) {
            hops[i].session_key[j] = static_cast<uint8_t>((i + 1) * 23 + j);
        }
    }
    
    onion::OnionRouter* router = onion::SimpleOnionRouter::get_instance();
    onion::Circuit* circuit = nullptr;
    core::ErrorCode err = router->create_circuit(hops, 5, &circuit);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(circuit->hop_count, static_cast<size_t>(5));
    
    router->destroy_circuit(circuit);
}

/**
 * Scenario: Multi-Circuit Communication
 * 
 * Demonstrates:
 * 1. Create multiple circuits for different purposes
 * 2. Each circuit provides independent anonymity
 * 3. Circuits can use different paths
 */
SCENARIO(multi_circuit_communication) {
    federation::Federation* fed = federation::SimpleFederation::get_instance();
    onion::OnionRouter* router = onion::SimpleOnionRouter::get_instance();
    
    // Set up peers
    const char* bootstrap[] = {
        "192.168.1.10:9050",
        "192.168.1.11:9050",
        "192.168.1.12:9050",
        "192.168.1.13:9050",
        "192.168.1.14:9050"
    };
    
    fed->initialize(bootstrap, 5);
    
    federation::Peer peers[10];
    size_t peer_count = 0;
    fed->discover_peers(peers, &peer_count, 10);
    ASSERT_TRUE(peer_count >= 5);
    
    // Create 3 different circuits with different paths
    onion::Circuit* circuits[3] = {nullptr, nullptr, nullptr};
    
    for (int c = 0; c < 3; c++) {
        onion::Node hops[3];
        
        // Use different nodes for each circuit
        for (int i = 0; i < 3; i++) {
            size_t peer_idx = (c + i) % peer_count;
            hops[i].node_id = peers[peer_idx].peer_id;
            strcpy(hops[i].address, peers[peer_idx].address);
            hops[i].port = peers[peer_idx].port;
            
            for (int j = 0; j < 32; j++) {
                hops[i].session_key[j] = static_cast<uint8_t>((c + 1) * (i + 1) * 29 + j);
            }
        }
        
        core::ErrorCode err = router->create_circuit(hops, 3, &circuits[c]);
        ASSERT_EQUAL(err, core::OK);
        ASSERT_NOT_NULL(circuits[c]);
    }
    
    // Verify all circuits are unique
    ASSERT_NOT_EQUAL(circuits[0]->circuit_id, circuits[1]->circuit_id);
    ASSERT_NOT_EQUAL(circuits[1]->circuit_id, circuits[2]->circuit_id);
    ASSERT_NOT_EQUAL(circuits[0]->circuit_id, circuits[2]->circuit_id);
    
    // Send different messages through each circuit
    const char* messages[] = {
        "Message through circuit 1",
        "Message through circuit 2",
        "Message through circuit 3"
    };
    
    for (int c = 0; c < 3; c++) {
        core::Buffer plaintext(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(messages[c])),
            strlen(messages[c])
        );
        
        onion::OnionPacket packet;
        core::ErrorCode err = router->send_onion(circuits[c], plaintext, packet);
        ASSERT_EQUAL(err, core::OK);
        ASSERT_NOT_NULL(packet.data);
        
        if (packet.data) {
            delete[] packet.data;
        }
    }
    
    // Clean up all circuits
    for (int c = 0; c < 3; c++) {
        router->destroy_circuit(circuits[c]);
    }
}

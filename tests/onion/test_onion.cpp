#include "federated/onion/onion.h"
#include "../test_runner.h"
#include <cstring>

using namespace federated;
using namespace federated::onion;

// Test: Get onion router instance
TEST(onion_get_instance) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    ASSERT_NOT_NULL(router);
    
    // Singleton pattern - should return same instance
    OnionRouter* router2 = SimpleOnionRouter::get_instance();
    ASSERT_EQUAL(router, router2);
}

// Test: Create a simple circuit
TEST(onion_create_circuit) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    // Create 3 nodes for the circuit
    Node hops[3];
    for (int i = 0; i < 3; i++) {
        hops[i].node_id = static_cast<uint32_t>(i + 1);
        snprintf(hops[i].address, sizeof(hops[i].address), "192.168.1.%d", i + 10);
        hops[i].port = static_cast<uint16_t>(9050 + i);
        hops[i].capabilities = NODE_CAPABILITY_RELAY;
        
        // Set session keys (in real implementation, these would be negotiated)
        for (int j = 0; j < 32; j++) {
            hops[i].session_key[j] = static_cast<uint8_t>((i + 1) * 10 + j);
        }
    }
    
    // Create circuit
    Circuit* circuit = nullptr;
    core::ErrorCode err = router->create_circuit(hops, 3, &circuit);
    
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(circuit);
    ASSERT_EQUAL(circuit->hop_count, static_cast<size_t>(3));
    ASSERT_TRUE(circuit->is_active);
    
    // Verify hops were copied correctly
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQUAL(circuit->hops[i].node_id, hops[i].node_id);
        ASSERT_EQUAL(strcmp(circuit->hops[i].address, hops[i].address), 0);
        ASSERT_EQUAL(circuit->hops[i].port, hops[i].port);
    }
    
    // Clean up
    router->destroy_circuit(circuit);
}

// Test: Create circuit with invalid arguments
TEST(onion_create_circuit_invalid_args) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    Node hops[3];
    Circuit* circuit = nullptr;
    
    // Null hops
    core::ErrorCode err = router->create_circuit(nullptr, 3, &circuit);
    ASSERT_EQUAL(err, core::ERR_INVALID_ARG);
    
    // Zero hop count
    err = router->create_circuit(hops, 0, &circuit);
    ASSERT_EQUAL(err, core::ERR_INVALID_ARG);
    
    // Null output
    err = router->create_circuit(hops, 3, nullptr);
    ASSERT_EQUAL(err, core::ERR_INVALID_ARG);
}

// Test: Send data through onion circuit (layered encryption)
TEST(onion_send_through_circuit) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    // Create 3-hop circuit
    Node hops[3];
    for (int i = 0; i < 3; i++) {
        hops[i].node_id = static_cast<uint32_t>(i + 1);
        hops[i].port = static_cast<uint16_t>(9050 + i);
        hops[i].capabilities = NODE_CAPABILITY_RELAY;
        
        // Set predictable session keys
        for (int j = 0; j < 32; j++) {
            hops[i].session_key[j] = static_cast<uint8_t>((i + 1) * 17 + j);
        }
    }
    
    Circuit* circuit = nullptr;
    core::ErrorCode err = router->create_circuit(hops, 3, &circuit);
    ASSERT_EQUAL(err, core::OK);
    
    // Send plaintext through circuit
    const char* message = "Secret message";
    core::Buffer plaintext(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(message)),
        strlen(message)
    );
    
    OnionPacket packet;
    err = router->send_onion(circuit, plaintext, packet);
    
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(packet.data);
    ASSERT_TRUE(packet.size > 0);
    ASSERT_EQUAL(packet.circuit_id, circuit->circuit_id);
    
    // Verify packet is encrypted (should not match plaintext)
    ASSERT_NOT_EQUAL(
        memcmp(packet.data, plaintext.data, plaintext.size), 
        0
    );
    
    // Clean up
    if (packet.data) {
        delete[] packet.data;
    }
    router->destroy_circuit(circuit);
}

// Test: Destroy circuit
TEST(onion_destroy_circuit) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    // Create circuit
    Node hops[2];
    for (int i = 0; i < 2; i++) {
        hops[i].node_id = static_cast<uint32_t>(i + 1);
        hops[i].port = 9050;
    }
    
    Circuit* circuit = nullptr;
    core::ErrorCode err = router->create_circuit(hops, 2, &circuit);
    ASSERT_EQUAL(err, core::OK);
    
    uint32_t circuit_id = circuit->circuit_id;
    
    // Destroy circuit
    err = router->destroy_circuit(circuit);
    ASSERT_EQUAL(err, core::OK);
    
    // Circuit should no longer be active
    // (we can't verify this directly without exposing internals,
    //  but at least verify destroy succeeded)
    (void)circuit_id; // Suppress unused warning
}

// Test: Bridge mode enable/disable
TEST(onion_bridge_mode) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    // Initially disabled
    ASSERT_FALSE(router->is_bridge_mode());
    
    // Enable bridge mode
    core::ErrorCode err = router->enable_bridge_mode();
    ASSERT_EQUAL(err, core::OK);
    ASSERT_TRUE(router->is_bridge_mode());
    
    // Disable bridge mode
    err = router->disable_bridge_mode();
    ASSERT_EQUAL(err, core::OK);
    ASSERT_FALSE(router->is_bridge_mode());
}

// Test: Node structure initialization
TEST(onion_node_structure) {
    Node node;
    
    // Default values
    ASSERT_EQUAL(node.node_id, static_cast<uint32_t>(0));
    ASSERT_EQUAL(node.port, static_cast<uint16_t>(0));
    ASSERT_EQUAL(node.capabilities, static_cast<uint8_t>(0));
    
    // Set values
    node.node_id = 42;
    strcpy(node.address, "192.168.1.100");
    node.port = 9050;
    node.capabilities = NODE_CAPABILITY_RELAY | NODE_CAPABILITY_BRIDGE;
    
    ASSERT_EQUAL(node.node_id, static_cast<uint32_t>(42));
    ASSERT_EQUAL(strcmp(node.address, "192.168.1.100"), 0);
    ASSERT_EQUAL(node.port, static_cast<uint16_t>(9050));
    ASSERT_TRUE(node.capabilities & NODE_CAPABILITY_RELAY);
    ASSERT_TRUE(node.capabilities & NODE_CAPABILITY_BRIDGE);
}

// Test: Multiple circuits
TEST(onion_multiple_circuits) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    Node hops[2];
    for (int i = 0; i < 2; i++) {
        hops[i].node_id = static_cast<uint32_t>(i + 1);
        hops[i].port = 9050;
    }
    
    // Create multiple circuits
    Circuit* circuit1 = nullptr;
    Circuit* circuit2 = nullptr;
    Circuit* circuit3 = nullptr;
    
    core::ErrorCode err1 = router->create_circuit(hops, 2, &circuit1);
    core::ErrorCode err2 = router->create_circuit(hops, 2, &circuit2);
    core::ErrorCode err3 = router->create_circuit(hops, 2, &circuit3);
    
    ASSERT_EQUAL(err1, core::OK);
    ASSERT_EQUAL(err2, core::OK);
    ASSERT_EQUAL(err3, core::OK);
    
    // Circuits should have unique IDs
    ASSERT_NOT_EQUAL(circuit1->circuit_id, circuit2->circuit_id);
    ASSERT_NOT_EQUAL(circuit2->circuit_id, circuit3->circuit_id);
    ASSERT_NOT_EQUAL(circuit1->circuit_id, circuit3->circuit_id);
    
    // Clean up
    router->destroy_circuit(circuit1);
    router->destroy_circuit(circuit2);
    router->destroy_circuit(circuit3);
}

// Test: Circuit with different hop counts
TEST(onion_variable_hop_count) {
    OnionRouter* router = SimpleOnionRouter::get_instance();
    
    // Test 1-hop circuit
    Node hops1[1];
    hops1[0].node_id = 1;
    hops1[0].port = 9050;
    
    Circuit* circuit1 = nullptr;
    core::ErrorCode err = router->create_circuit(hops1, 1, &circuit1);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(circuit1->hop_count, static_cast<size_t>(1));
    
    // Test 5-hop circuit
    Node hops5[5];
    for (int i = 0; i < 5; i++) {
        hops5[i].node_id = static_cast<uint32_t>(i + 1);
        hops5[i].port = 9050;
    }
    
    Circuit* circuit5 = nullptr;
    err = router->create_circuit(hops5, 5, &circuit5);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(circuit5->hop_count, static_cast<size_t>(5));
    
    // Clean up
    router->destroy_circuit(circuit1);
    router->destroy_circuit(circuit5);
}

#include "federated/onion/onion.h"
#include "federated/crypto/chacha20.h"
#include <cstring>
#include <ctime>

namespace federated {
namespace onion {

// Singleton instance
static SimpleOnionRouter* g_onion_router_instance = nullptr;

SimpleOnionRouter::SimpleOnionRouter() 
    : bridge_mode_enabled(false),
      active_circuits(nullptr),
      circuit_capacity(16),
      circuit_count(0),
      next_circuit_id(1) {
    // Allocate circuit array
    active_circuits = new Circuit*[circuit_capacity];
    for (size_t i = 0; i < circuit_capacity; i++) {
        active_circuits[i] = nullptr;
    }
}

SimpleOnionRouter::~SimpleOnionRouter() {
    // Clean up all circuits
    if (active_circuits) {
        for (size_t i = 0; i < circuit_capacity; i++) {
            if (active_circuits[i]) {
                destroy_circuit(active_circuits[i]);
            }
        }
        delete[] active_circuits;
    }
}

OnionRouter* SimpleOnionRouter::get_instance() {
    if (!g_onion_router_instance) {
        g_onion_router_instance = new SimpleOnionRouter();
    }
    return g_onion_router_instance;
}

core::ErrorCode SimpleOnionRouter::create_circuit(
    const Node* hops,
    size_t hop_count,
    Circuit** out_circuit
) {
    if (!hops || hop_count == 0 || !out_circuit) {
        return core::ERR_INVALID_ARG;
    }
    
    if (hop_count > 10) {
        // Sanity limit: max 10 hops
        return core::ERR_INVALID_ARG;
    }
    
    // Check if we have capacity
    if (circuit_count >= circuit_capacity) {
        return core::ERR_OUT_OF_MEMORY;
    }
    
    // Allocate new circuit
    Circuit* circuit = new Circuit();
    circuit->circuit_id = next_circuit_id++;
    circuit->hop_count = hop_count;
    circuit->is_active = true;
    circuit->created_at = static_cast<uint64_t>(time(nullptr));
    
    // Copy hops
    circuit->hops = new Node[hop_count];
    for (size_t i = 0; i < hop_count; i++) {
        circuit->hops[i] = hops[i];
    }
    
    // Create crypto layer for each hop
    // Note: Currently uses singleton ChaCha20 instance for all hops.
    // This is safe because each encryption uses a different session key.
    // Future enhancement: Could use separate crypto instances per hop
    // for additional isolation if needed.
    circuit->crypto_layers = new crypto::Crypto*[hop_count];
    for (size_t i = 0; i < hop_count; i++) {
        // Use ChaCha20 for each layer
        circuit->crypto_layers[i] = crypto::ChaCha20Crypto::get_instance();
    }
    
    // Add to active circuits
    bool added = false;
    for (size_t i = 0; i < circuit_capacity; i++) {
        if (!active_circuits[i]) {
            active_circuits[i] = circuit;
            circuit_count++;
            added = true;
            break;
        }
    }
    
    // If we couldn't add to active circuits (shouldn't happen due to check above),
    // clean up and return error
    if (!added) {
        delete[] circuit->hops;
        delete[] circuit->crypto_layers;
        delete circuit;
        return core::ERR_OUT_OF_MEMORY;
    }
    
    *out_circuit = circuit;
    return core::OK;
}

core::ErrorCode SimpleOnionRouter::send_onion(
    Circuit* circuit,
    const core::Buffer& plaintext,
    OnionPacket& onion_packet
) {
    if (!circuit || !circuit->is_active) {
        return core::ERR_INVALID_ARG;
    }
    
    if (plaintext.size == 0) {
        return core::ERR_INVALID_ARG;
    }
    
    // Allocate buffer for layered encryption
    // Each layer adds overhead (nonce + tag for AEAD, or just nonce for stream cipher)
    // For simplicity, allocate generous buffer
    size_t max_size = plaintext.size + (circuit->hop_count * 64);
    uint8_t* working_buffer = new uint8_t[max_size];
    uint8_t* output_buffer = new uint8_t[max_size];
    
    // Copy plaintext to working buffer
    memcpy(working_buffer, plaintext.data, plaintext.size);
    size_t current_size = plaintext.size;
    
    // Apply encryption layers from innermost (last hop) to outermost (first hop)
    for (int i = static_cast<int>(circuit->hop_count) - 1; i >= 0; i--) {
        Node& hop = circuit->hops[i];
        crypto::Crypto* crypto = circuit->crypto_layers[i];
        
        // Prepare key buffer: session_key (32 bytes) + nonce (12 bytes)
        uint8_t key_buffer[44];
        memcpy(key_buffer, hop.session_key, 32);
        
        // Generate simple nonce from circuit_id and hop index
        uint32_t nonce_seed = circuit->circuit_id ^ (static_cast<uint32_t>(i) << 16);
        memset(key_buffer + 32, 0, 12);
        memcpy(key_buffer + 32, &nonce_seed, 4);
        
        core::Buffer key(key_buffer, 44);
        core::Buffer input(working_buffer, current_size);
        core::Buffer output(output_buffer, max_size);
        
        // Encrypt this layer
        core::ErrorCode err = crypto->encrypt(input, key, output);
        if (err != core::OK) {
            delete[] working_buffer;
            delete[] output_buffer;
            return err;
        }
        
        // Swap buffers
        memcpy(working_buffer, output.data, output.size);
        current_size = output.size;
    }
    
    // Create onion packet
    onion_packet.data = new uint8_t[current_size];
    memcpy(onion_packet.data, working_buffer, current_size);
    onion_packet.size = current_size;
    onion_packet.circuit_id = circuit->circuit_id;
    
    delete[] working_buffer;
    delete[] output_buffer;
    
    return core::OK;
}

core::ErrorCode SimpleOnionRouter::peel_layer(
    const OnionPacket& onion_packet,
    OnionPacket& inner_packet,
    Node* next_hop
) {
    if (!onion_packet.data || onion_packet.size == 0) {
        return core::ERR_INVALID_ARG;
    }
    
    // In a real implementation, we would:
    // 1. Look up the circuit by circuit_id
    // 2. Find which hop we are in the circuit
    // 3. Decrypt using our layer's key
    // 4. Extract next hop info from decrypted header
    // 5. Return inner packet
    
    // For now, this is a simplified stub that will be enhanced
    // when we integrate with the transport layer
    
    // Suppress unused parameter warnings
    (void)inner_packet;
    (void)next_hop;
    
    return core::ERR_UNSUPPORTED;
}

core::ErrorCode SimpleOnionRouter::destroy_circuit(Circuit* circuit) {
    if (!circuit) {
        return core::ERR_INVALID_ARG;
    }
    
    // Remove from active circuits
    for (size_t i = 0; i < circuit_capacity; i++) {
        if (active_circuits[i] == circuit) {
            active_circuits[i] = nullptr;
            circuit_count--;
            break;
        }
    }
    
    // Free resources
    if (circuit->hops) {
        delete[] circuit->hops;
    }
    
    if (circuit->crypto_layers) {
        delete[] circuit->crypto_layers;
    }
    
    circuit->is_active = false;
    delete circuit;
    
    return core::OK;
}

core::ErrorCode SimpleOnionRouter::enable_bridge_mode() {
    bridge_mode_enabled = true;
    return core::OK;
}

core::ErrorCode SimpleOnionRouter::disable_bridge_mode() {
    bridge_mode_enabled = false;
    return core::OK;
}

bool SimpleOnionRouter::is_bridge_mode() const {
    return bridge_mode_enabled;
}

} // namespace onion
} // namespace federated

#include "../include/federated/core/registry.h"
#include "../include/federated/transport/loopback.h"
#include "../include/federated/framer/length_prefix.h"
#include "../include/federated/crypto/none.h"
#include <cstdio>
#include <cstring>

using namespace federated;

/**
 * Simple example demonstrating the Federated communication stack
 * 
 * Shows:
 * - Component registration in the global registry
 * - Message encoding with framer
 * - Message transport
 * - Message decoding
 */

int main() {
    printf("=== Federated Communication Example ===\n\n");
    
    // Get component instances
    transport::Transport* loopback = transport::LoopbackTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    crypto::Crypto* crypto = crypto::NoneCrypto::get_instance();
    
    // Register components in global registry
    printf("Registering components...\n");
    core::global_registry.register_transport(loopback);
    core::global_registry.register_framer(framer);
    core::global_registry.register_crypto(crypto);
    
    printf("  Transport: %s\n", loopback->name);
    printf("  Framer: %s\n", framer->name);
    printf("  Crypto: %s (key size: %zu bytes)\n\n", crypto->name, crypto->key_size);
    
    // Open transport
    printf("Opening transport...\n");
    if (loopback->open() != core::OK) {
        fprintf(stderr, "Failed to open transport\n");
        return 1;
    }
    
    // Prepare message
    const char* message = "Hello from Federated!";
    printf("Original message: \"%s\"\n", message);
    
    uint8_t msg_buf[256];
    memcpy(msg_buf, message, strlen(message));
    core::Buffer msg(msg_buf, strlen(message));
    
    // Encrypt (no-op for "none" crypto)
    uint8_t encrypted_buf[256];
    core::Buffer encrypted(encrypted_buf, sizeof(encrypted_buf));
    uint8_t key[1] = {0};
    core::Buffer key_buf(key, 0);
    
    if (crypto->encrypt(msg, key_buf, encrypted) != core::OK) {
        fprintf(stderr, "Encryption failed\n");
        return 1;
    }
    printf("After encryption: %zu bytes\n", encrypted.size);
    
    // Frame the message
    uint8_t framed_buf[256];
    core::Buffer framed(framed_buf, sizeof(framed_buf));
    
    if (framer->encode(encrypted, framed) != core::OK) {
        fprintf(stderr, "Framing failed\n");
        return 1;
    }
    printf("After framing: %zu bytes (includes 4-byte header)\n", framed.size);
    
    // Send over transport
    if (loopback->send(framed) != core::OK) {
        fprintf(stderr, "Send failed\n");
        return 1;
    }
    printf("Sent via %s transport\n\n", loopback->name);
    
    // ===== Receiving side =====
    
    printf("Receiving...\n");
    
    // Receive from transport
    uint8_t recv_buf[256];
    core::Buffer received(recv_buf, sizeof(recv_buf));
    
    if (loopback->recv(received) != core::OK) {
        fprintf(stderr, "Receive failed\n");
        return 1;
    }
    printf("Received %zu bytes\n", received.size);
    
    // Decode frame
    uint8_t decoded_buf[256];
    core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
    
    if (framer->decode(received, decoded) != core::OK) {
        fprintf(stderr, "Decoding failed\n");
        return 1;
    }
    printf("After decoding: %zu bytes\n", decoded.size);
    
    // Decrypt (no-op for "none" crypto)
    uint8_t decrypted_buf[256];
    core::Buffer decrypted(decrypted_buf, sizeof(decrypted_buf));
    
    if (crypto->decrypt(decoded, key_buf, decrypted) != core::OK) {
        fprintf(stderr, "Decryption failed\n");
        return 1;
    }
    
    // Verify message
    decrypted_buf[decrypted.size] = '\0';  // Null-terminate for printing
    printf("Decrypted message: \"%s\"\n\n", (char*)decrypted_buf);
    
    // Verify integrity
    if (memcmp(decrypted_buf, message, strlen(message)) == 0) {
        printf("✓ Message integrity verified!\n");
    } else {
        printf("✗ Message corrupted!\n");
    }
    
    // Close transport
    loopback->close();
    
    printf("\n=== Example Complete ===\n");
    
    return 0;
}

/**
 * @file scenario_end_to_end_crypto.cpp
 * @brief User Scenario Test: End-to-End Encrypted Communication
 * 
 * Tests a complete communication scenario with encryption:
 * 1. Encrypt message with crypto layer
 * 2. Frame encrypted message
 * 3. Send via transport
 * 4. Receive and decode frame
 * 5. Decrypt message
 * 6. Verify original message integrity
 * 
 * This tests the integration of Crypto + Framer + Transport layers.
 */

#include "../test_runner.h"
#include "../../include/federated/transport/loopback.h"
#include "../../include/federated/framer/length_prefix.h"
#include "../../include/federated/crypto/none.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>

using namespace federated;

TEST(scenario_end_to_end_with_crypto) {
    printf("\n  === User Scenario: End-to-End Encrypted Communication ===\n");
    
    const char* original_message = "This is a secret message that should be encrypted!";
    
    // Get components
    transport::Transport* transport = transport::LoopbackTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    crypto::Crypto* crypto_module = crypto::NoneCrypto::get_instance();
    
    // Setup transport
    TEST_ASSERT_EQ(transport->open(), core::OK);
    printf("  1. Communication channel opened\n");
    
    // Sender side: Encrypt -> Frame -> Send
    printf("  === Sender Side ===\n");
    
    // Create message buffer
    uint8_t msg_buf[256];
    memcpy(msg_buf, original_message, strlen(original_message));
    core::Buffer msg(msg_buf, strlen(original_message));
    printf("  2. Original message: \"%s\" (%zu bytes)\n", original_message, msg.size);
    
    // Encrypt message
    uint8_t encrypted_buf[256];
    core::Buffer encrypted(encrypted_buf, sizeof(encrypted_buf));
    uint8_t key[1] = {0};  // Dummy key for "none" crypto
    core::Buffer key_buf(key, 0);
    
    TEST_ASSERT_EQ(crypto_module->encrypt(msg, key_buf, encrypted), core::OK);
    printf("  3. Message encrypted (%zu bytes)\n", encrypted.size);
    
    // Frame encrypted message
    uint8_t framed_buf[512];
    core::Buffer framed(framed_buf, sizeof(framed_buf));
    
    TEST_ASSERT_EQ(framer->encode(encrypted, framed), core::OK);
    printf("  4. Encrypted message framed (%zu bytes)\n", framed.size);
    
    // Send framed, encrypted message
    TEST_ASSERT_EQ(transport->send(framed), core::OK);
    printf("  5. Sent over transport ✓\n");
    
    // Receiver side: Receive -> Decode -> Decrypt
    printf("  === Receiver Side ===\n");
    
    // Receive framed message
    uint8_t received_buf[512];
    core::Buffer received(received_buf, sizeof(received_buf));
    
    TEST_ASSERT_EQ(transport->recv(received), core::OK);
    printf("  6. Received from transport (%zu bytes)\n", received.size);
    
    // Decode frame
    uint8_t decoded_buf[256];
    core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
    
    TEST_ASSERT_EQ(framer->decode(received, decoded), core::OK);
    printf("  7. Frame decoded (%zu bytes)\n", decoded.size);
    
    // Decrypt message
    uint8_t decrypted_buf[256];
    core::Buffer decrypted(decrypted_buf, sizeof(decrypted_buf));
    
    TEST_ASSERT_EQ(crypto_module->decrypt(decoded, key_buf, decrypted), core::OK);
    printf("  8. Message decrypted (%zu bytes)\n", decrypted.size);
    
    // Verify original message
    decrypted_buf[decrypted.size] = '\0';  // Null-terminate for printing
    printf("  9. Decrypted message: \"%s\"\n", (char*)decrypted_buf);
    
    TEST_ASSERT_EQ(decrypted.size, strlen(original_message));
    TEST_ASSERT_EQ(memcmp(decrypted.data, original_message, decrypted.size), 0);
    printf("  10. Message integrity verified ✓\n");
    
    // Cleanup
    transport->close();
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_full_stack_communication) {
    printf("\n  === User Scenario: Full Stack Communication ===\n");
    
    const char* messages[] = {
        "First secure message",
        "Second secure message",
        "Third secure message"
    };
    int message_count = 3;
    
    // Get full stack components
    transport::Transport* transport = transport::LoopbackTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    crypto::Crypto* crypto_module = crypto::NoneCrypto::get_instance();
    
    TEST_ASSERT_EQ(transport->open(), core::OK);
    
    uint8_t key[1] = {0};
    core::Buffer key_buf(key, 0);
    
    // Send all messages through full stack
    for (int i = 0; i < message_count; i++) {
        // Prepare message
        uint8_t msg_buf[128];
        memcpy(msg_buf, messages[i], strlen(messages[i]));
        core::Buffer msg(msg_buf, strlen(messages[i]));
        
        // Encrypt
        uint8_t encrypted_buf[128];
        core::Buffer encrypted(encrypted_buf, sizeof(encrypted_buf));
        TEST_ASSERT_EQ(crypto_module->encrypt(msg, key_buf, encrypted), core::OK);
        
        // Frame
        uint8_t framed_buf[256];
        core::Buffer framed(framed_buf, sizeof(framed_buf));
        TEST_ASSERT_EQ(framer->encode(encrypted, framed), core::OK);
        
        // Send
        TEST_ASSERT_EQ(transport->send(framed), core::OK);
        printf("  Sent message %d (%zu bytes)\n", i + 1, framed.size);
    }
    
    // Receive all messages through full stack
    for (int i = 0; i < message_count; i++) {
        // Receive
        uint8_t received_buf[256];
        core::Buffer received(received_buf, sizeof(received_buf));
        TEST_ASSERT_EQ(transport->recv(received), core::OK);
        
        // Decode
        uint8_t decoded_buf[128];
        core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
        TEST_ASSERT_EQ(framer->decode(received, decoded), core::OK);
        
        // Decrypt
        uint8_t decrypted_buf[128];
        core::Buffer decrypted(decrypted_buf, sizeof(decrypted_buf));
        TEST_ASSERT_EQ(crypto_module->decrypt(decoded, key_buf, decrypted), core::OK);
        
        // Verify
        TEST_ASSERT_EQ(decrypted.size, strlen(messages[i]));
        TEST_ASSERT_EQ(memcmp(decrypted.data, messages[i], decrypted.size), 0);
        printf("  Received and verified message %d ✓\n", i + 1);
    }
    
    transport->close();
    printf("  === Scenario Complete ===\n\n");
}

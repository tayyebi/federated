#include "../test_runner.h"
#include "../../include/federated/crypto/chacha20.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>

using namespace federated;
using namespace federated::crypto;
using namespace federated::core;

// Helper to create key buffer
static Buffer create_chacha20_key(const uint8_t key[32], const uint8_t nonce[12], const uint8_t* counter = nullptr) {
    static uint8_t key_buffer[48];
    std::memcpy(key_buffer, key, 32);
    std::memcpy(key_buffer + 32, nonce, 12);
    if (counter) {
        std::memcpy(key_buffer + 44, counter, 4);
    }
    return Buffer(key_buffer, counter ? 48 : 44);
}

// RFC 8439 Test Vector - Section 2.4.2
TEST(chacha20_rfc_test_vector) {
    Crypto* c = ChaCha20Crypto::get_instance();
    TEST_ASSERT(c != nullptr);
    
    // Key: 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
    uint8_t key[32];
    for (int i = 0; i < 32; i++) key[i] = static_cast<uint8_t>(i);
    
    // Nonce: 000000000000004a00000000
    uint8_t nonce[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00};
    
    // Counter: 1
    uint8_t counter[4] = {0x01, 0x00, 0x00, 0x00};
    
    Buffer key_buf = create_chacha20_key(key, nonce, counter);
    
    // Plaintext
    const char* plaintext_str = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    size_t plaintext_len = std::strlen(plaintext_str);
    Buffer plaintext(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(plaintext_str)), plaintext_len);
    
    // Allocate output
    uint8_t ciphertext_data[256];
    Buffer ciphertext(ciphertext_data, sizeof(ciphertext_data));
    
    ErrorCode err = c->encrypt(plaintext, key_buf, ciphertext);
    TEST_ASSERT_EQ(err, OK);
    
    // Expected ciphertext from RFC 8439
    const uint8_t expected[] = {
        0x6e, 0x2e, 0x35, 0x9a, 0x25, 0x68, 0xf9, 0x80,
        0x41, 0xba, 0x07, 0x28, 0xdd, 0x0d, 0x69, 0x81,
        0xe9, 0x7e, 0x7a, 0xec, 0x1d, 0x43, 0x60, 0xc2,
        0x0a, 0x27, 0xaf, 0xcc, 0xfd, 0x9f, 0xae, 0x0b,
        0xf9, 0x1b, 0x65, 0xc5, 0x52, 0x47, 0x33, 0xab,
        0x8f, 0x59, 0x3d, 0xab, 0xcd, 0x62, 0xb3, 0x57,
        0x16, 0x39, 0xd6, 0x24, 0xe6, 0x51, 0x52, 0xab,
        0x8f, 0x53, 0x0c, 0x35, 0x9f, 0x08, 0x61, 0xd8,
        0x07, 0xca, 0x0d, 0xbf, 0x50, 0x0d, 0x6a, 0x61,
        0x56, 0xa3, 0x8e, 0x08, 0x8a, 0x22, 0xb6, 0x5e,
        0x52, 0xbc, 0x51, 0x4d, 0x16, 0xcc, 0xf8, 0x06,
        0x81, 0x8c, 0xe9, 0x1a, 0xb7, 0x79, 0x37, 0x36,
        0x5a, 0xf9, 0x0b, 0xbf, 0x74, 0xa3, 0x5b, 0xe6,
        0xb4, 0x0b, 0x8e, 0xed, 0xf2, 0x78, 0x5e, 0x42,
        0x87, 0x4d
    };
    
    TEST_ASSERT_EQ(ciphertext.size, sizeof(expected));
    TEST_ASSERT_EQ(std::memcmp(ciphertext.data, expected, sizeof(expected)), 0);
}

TEST(chacha20_get_instance) {
    Crypto* c = ChaCha20Crypto::get_instance();
    TEST_ASSERT(c != nullptr);
    TEST_ASSERT(c->name != nullptr);
    TEST_ASSERT_STR_EQ(c->name, "chacha20");
    TEST_ASSERT_EQ(c->key_size, 44);  // 32 bytes key + 12 bytes nonce
}

TEST(chacha20_roundtrip) {
    Crypto* c = ChaCha20Crypto::get_instance();
    
    // Create key
    uint8_t key[32];
    for (int i = 0; i < 32; i++) key[i] = static_cast<uint8_t>(i * 3);
    
    uint8_t nonce[12] = {0};
    Buffer key_buf = create_chacha20_key(key, nonce);
    
    // Test data
    const char* original = "Secret message for encryption";
    size_t original_len = std::strlen(original);
    Buffer plaintext(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(original)), original_len);
    
    // Encrypt
    uint8_t ciphertext_data[256];
    Buffer ciphertext(ciphertext_data, sizeof(ciphertext_data));
    
    ErrorCode err = c->encrypt(plaintext, key_buf, ciphertext);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(ciphertext.size, original_len);
    
    // Decrypt
    uint8_t decrypted_data[256];
    Buffer decrypted(decrypted_data, sizeof(decrypted_data));
    
    err = c->decrypt(ciphertext, key_buf, decrypted);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(decrypted.size, original_len);
    
    // Verify
    TEST_ASSERT_EQ(std::memcmp(decrypted.data, original, original_len), 0);
}

TEST(chacha20_empty_message) {
    Crypto* c = ChaCha20Crypto::get_instance();
    
    uint8_t key[32] = {0};
    uint8_t nonce[12] = {0};
    Buffer key_buf = create_chacha20_key(key, nonce);
    
    Buffer empty(nullptr, 0);
    uint8_t output_data[64];
    Buffer output(output_data, sizeof(output_data));
    
    ErrorCode err = c->encrypt(empty, key_buf, output);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(output.size, 0);
}

TEST(chacha20_large_message) {
    Crypto* c = ChaCha20Crypto::get_instance();
    
    uint8_t key[32];
    for (int i = 0; i < 32; i++) key[i] = static_cast<uint8_t>(i);
    
    uint8_t nonce[12] = {0};
    Buffer key_buf = create_chacha20_key(key, nonce);
    
    // Create a large message (200 bytes - more than 3 ChaCha20 blocks)
    uint8_t large_data[200];
    for (int i = 0; i < 200; i++) {
        large_data[i] = static_cast<uint8_t>('A' + (i % 26));
    }
    
    Buffer plaintext(large_data, 200);
    
    // Encrypt
    uint8_t ciphertext_data[256];
    Buffer ciphertext(ciphertext_data, sizeof(ciphertext_data));
    
    ErrorCode err = c->encrypt(plaintext, key_buf, ciphertext);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(ciphertext.size, 200);
    
    // Decrypt
    uint8_t decrypted_data[256];
    Buffer decrypted(decrypted_data, sizeof(decrypted_data));
    
    err = c->decrypt(ciphertext, key_buf, decrypted);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify
    TEST_ASSERT_EQ(std::memcmp(decrypted.data, large_data, 200), 0);
}

TEST(chacha20_invalid_key_size) {
    Crypto* c = ChaCha20Crypto::get_instance();
    
    // Invalid key (too short)
    uint8_t short_key[10] = {0};
    Buffer key_buf(short_key, sizeof(short_key));
    
    const char* msg = "Test";
    Buffer plaintext(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(msg)), 4);
    
    uint8_t output_data[64];
    Buffer output(output_data, sizeof(output_data));
    
    ErrorCode err = c->encrypt(plaintext, key_buf, output);
    TEST_ASSERT_EQ(err, ERR_CRYPTO);
}

TEST(chacha20_different_keys) {
    Crypto* c = ChaCha20Crypto::get_instance();
    
    // First key
    uint8_t key1[32] = {0};
    uint8_t nonce1[12] = {0};
    Buffer key_buf1 = create_chacha20_key(key1, nonce1);
    
    const char* msg = "Test message";
    size_t msg_len = std::strlen(msg);
    Buffer plaintext(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(msg)), msg_len);
    
    uint8_t ciphertext1_data[64];
    Buffer ciphertext1(ciphertext1_data, sizeof(ciphertext1_data));
    c->encrypt(plaintext, key_buf1, ciphertext1);
    
    // Second key (all 0xFF)
    uint8_t key2[32];
    for (int i = 0; i < 32; i++) key2[i] = 0xFF;
    uint8_t nonce2[12] = {0};
    Buffer key_buf2 = create_chacha20_key(key2, nonce2);
    
    uint8_t ciphertext2_data[64];
    Buffer ciphertext2(ciphertext2_data, sizeof(ciphertext2_data));
    c->encrypt(plaintext, key_buf2, ciphertext2);
    
    // Ciphertexts should be different
    TEST_ASSERT_NE(std::memcmp(ciphertext1.data, ciphertext2.data, msg_len), 0);
}

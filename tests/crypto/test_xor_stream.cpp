#include "../test_runner.h"
#include "../../include/federated/crypto/xor_stream.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::crypto;
using namespace federated::core;

TEST(xor_stream_get_instance) {
    Crypto* c = XORStreamCrypto::get_instance();
    TEST_ASSERT(c != nullptr);
    TEST_ASSERT(c->name != nullptr);
    TEST_ASSERT_STR_EQ(c->name, "xor_stream");
    TEST_ASSERT_EQ(c->key_size, 0);  // Variable key size
}

TEST(xor_stream_roundtrip) {
    Crypto* c = XORStreamCrypto::get_instance();
    
    const char* original = "This is a test message";
    uint8_t orig_buf[64];
    memcpy(orig_buf, original, strlen(original));
    Buffer orig(orig_buf, strlen(original));
    
    uint8_t key_data[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    Buffer key(key_data, sizeof(key_data));
    
    // Encrypt
    uint8_t encrypted[128];
    Buffer enc_buf(encrypted, sizeof(encrypted));
    TEST_ASSERT_EQ(c->encrypt(orig, key, enc_buf), OK);
    TEST_ASSERT_EQ(enc_buf.size, strlen(original));
    
    // Decrypt
    uint8_t decrypted[128];
    Buffer dec_buf(decrypted, sizeof(decrypted));
    TEST_ASSERT_EQ(c->decrypt(enc_buf, key, dec_buf), OK);
    
    // Verify roundtrip
    TEST_ASSERT_EQ(dec_buf.size, strlen(original));
    TEST_ASSERT_EQ(memcmp(decrypted, original, strlen(original)), 0);
}

TEST(xor_stream_key_sizes) {
    Crypto* c = XORStreamCrypto::get_instance();
    
    const char* msg = "Test data";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    // Test 1-byte key
    uint8_t key1[1] = {0xAA};
    Buffer key1_buf(key1, sizeof(key1));
    TEST_ASSERT_EQ(c->encrypt(in_buf, key1_buf, out_buf), OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    
    // Test 8-byte key
    uint8_t key8[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Buffer key8_buf(key8, sizeof(key8));
    TEST_ASSERT_EQ(c->encrypt(in_buf, key8_buf, out_buf), OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    
    // Test 256-byte key
    uint8_t key256[256];
    for (int i = 0; i < 256; i++) {
        key256[i] = (uint8_t)i;
    }
    Buffer key256_buf(key256, sizeof(key256));
    TEST_ASSERT_EQ(c->encrypt(in_buf, key256_buf, out_buf), OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    
    // Test invalid key size (0 bytes)
    Buffer key0_buf(key1, 0);
    TEST_ASSERT_EQ(c->encrypt(in_buf, key0_buf, out_buf), ERR_CRYPTO);
    
    // Test invalid key size (>256 bytes)
    uint8_t key257[257];
    Buffer key257_buf(key257, sizeof(key257));
    TEST_ASSERT_EQ(c->encrypt(in_buf, key257_buf, out_buf), ERR_CRYPTO);
}

TEST(xor_stream_double_encrypt) {
    Crypto* c = XORStreamCrypto::get_instance();
    
    const char* original = "XOR property test";
    uint8_t orig_buf[64];
    memcpy(orig_buf, original, strlen(original));
    Buffer orig(orig_buf, strlen(original));
    
    uint8_t key_data[4] = {0xFF, 0x00, 0xAA, 0x55};
    Buffer key(key_data, sizeof(key_data));
    
    // First encryption
    uint8_t encrypted1[128];
    Buffer enc1_buf(encrypted1, sizeof(encrypted1));
    TEST_ASSERT_EQ(c->encrypt(orig, key, enc1_buf), OK);
    
    // Second encryption (should restore original)
    uint8_t encrypted2[128];
    Buffer enc2_buf(encrypted2, sizeof(encrypted2));
    TEST_ASSERT_EQ(c->encrypt(enc1_buf, key, enc2_buf), OK);
    
    // Verify: XOR twice returns original
    TEST_ASSERT_EQ(enc2_buf.size, strlen(original));
    TEST_ASSERT_EQ(memcmp(encrypted2, original, strlen(original)), 0);
}

TEST(xor_stream_changes_data) {
    Crypto* c = XORStreamCrypto::get_instance();
    
    const char* msg = "Data should be modified";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t key_data[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    Buffer key(key_data, sizeof(key_data));
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    TEST_ASSERT_EQ(c->encrypt(in_buf, key, out_buf), OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    
    // Verify data is actually changed (not pass-through)
    TEST_ASSERT(memcmp(output, msg, strlen(msg)) != 0);
}

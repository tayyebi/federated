#include "../test_runner.h"
#include "../../include/federated/crypto/none.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::crypto;
using namespace federated::core;

TEST(none_get_instance) {
    Crypto* c = NoneCrypto::get_instance();
    TEST_ASSERT(c != nullptr);
    TEST_ASSERT(c->name != nullptr);
    TEST_ASSERT_STR_EQ(c->name, "none");
    TEST_ASSERT_EQ(c->key_size, 0);
}

TEST(none_encrypt_passthrough) {
    Crypto* c = NoneCrypto::get_instance();
    
    const char* msg = "Plaintext data";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t key[1] = {0};  // Dummy key, not used
    Buffer key_buf(key, 0);
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = c->encrypt(in_buf, key_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(output, msg, strlen(msg)), 0);
}

TEST(none_decrypt_passthrough) {
    Crypto* c = NoneCrypto::get_instance();
    
    const char* msg = "Ciphertext data";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t key[1] = {0};  // Dummy key, not used
    Buffer key_buf(key, 0);
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = c->decrypt(in_buf, key_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(output, msg, strlen(msg)), 0);
}

TEST(none_roundtrip) {
    Crypto* c = NoneCrypto::get_instance();
    
    const char* original = "Roundtrip crypto test";
    uint8_t orig_buf[64];
    memcpy(orig_buf, original, strlen(original));
    Buffer orig(orig_buf, strlen(original));
    
    uint8_t key[1] = {0};
    Buffer key_buf(key, 0);
    
    // Encrypt
    uint8_t encrypted[128];
    Buffer enc_buf(encrypted, sizeof(encrypted));
    TEST_ASSERT_EQ(c->encrypt(orig, key_buf, enc_buf), OK);
    
    // Decrypt
    uint8_t decrypted[128];
    Buffer dec_buf(decrypted, sizeof(decrypted));
    TEST_ASSERT_EQ(c->decrypt(enc_buf, key_buf, dec_buf), OK);
    
    // Verify
    TEST_ASSERT_EQ(dec_buf.size, strlen(original));
    TEST_ASSERT_EQ(memcmp(decrypted, original, strlen(original)), 0);
}

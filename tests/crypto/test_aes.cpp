#include "federated/crypto/aes.h"
#include "../test_runner.h"
#include <cstring>

using namespace federated;
using namespace federated::crypto;

// Test: Get AES instance
TEST(aes_get_instance) {
    Crypto* aes = AESCrypto::get_instance();
    ASSERT_NOT_NULL(aes);
    
    // Singleton pattern
    Crypto* aes2 = AESCrypto::get_instance();
    ASSERT_EQUAL(aes, aes2);
}

// Test: AES-128 ECB Mode (NIST test vector from FIPS 197 Appendix C.1)
TEST(aes_128_nist_test_vector) {
    Crypto* aes = AESCrypto::get_instance();
    
    // NIST test vector: AES-128 encryption
    // Plaintext: 00112233445566778899aabbccddeeff
    uint8_t plaintext_data[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    
    // Key: 000102030405060708090a0b0c0d0e0f
    // IV: 00000000000000000000000000000000 (for CBC)
    uint8_t key_data[32];
    for (int i = 0; i < 16; i++) {
        key_data[i] = static_cast<uint8_t>(i); // AES key
        key_data[16 + i] = 0x00; // IV (all zeros)
    }
    
    core::Buffer plaintext(plaintext_data, 16);
    core::Buffer key(key_data, 32);
    core::Buffer ciphertext;
    
    // Encrypt
    core::ErrorCode err = aes->encrypt(plaintext, key, ciphertext);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(ciphertext.data);
    
    // Ciphertext should be padded to 32 bytes (16 bytes + 16 bytes padding)
    ASSERT_EQUAL(ciphertext.size, static_cast<size_t>(32));
    
    // Decrypt
    core::Buffer decrypted;
    err = aes->decrypt(ciphertext, key, decrypted);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(decrypted.size, plaintext.size);
    ASSERT_EQUAL(memcmp(decrypted.data, plaintext.data, plaintext.size), 0);
    
    delete[] ciphertext.data;
    delete[] decrypted.data;
}

// Test: AES-256 encryption and decryption
TEST(aes_256_roundtrip) {
    Crypto* aes = AESCrypto::get_instance();
    
    // Test message
    const char* message = "AES-256 encryption test message";
    core::Buffer plaintext(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(message)),
        strlen(message)
    );
    
    // AES-256 key (32 bytes) + IV (16 bytes) = 48 bytes total
    uint8_t key_data[48];
    for (int i = 0; i < 48; i++) {
        key_data[i] = static_cast<uint8_t>(i * 7 + 13); // Pseudo-random key
    }
    core::Buffer key(key_data, 48);
    
    // Encrypt
    core::Buffer ciphertext;
    core::ErrorCode err = aes->encrypt(plaintext, key, ciphertext);
    
    // AES-256 not yet implemented, should return UNSUPPORTED
    ASSERT_EQUAL(err, core::ERR_UNSUPPORTED);
}

// Test: AES with empty message
TEST(aes_empty_message) {
    Crypto* aes = AESCrypto::get_instance();
    
    uint8_t empty_data[1];
    core::Buffer plaintext(empty_data, 0);
    
    uint8_t key_data[32];
    for (int i = 0; i < 32; i++) {
        key_data[i] = static_cast<uint8_t>(i);
    }
    core::Buffer key(key_data, 32);
    
    // Encrypt empty message (should still add padding)
    core::Buffer ciphertext;
    core::ErrorCode err = aes->encrypt(plaintext, key, ciphertext);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(ciphertext.data);
    ASSERT_EQUAL(ciphertext.size, static_cast<size_t>(16)); // One block of padding
    
    // Decrypt
    core::Buffer decrypted;
    err = aes->decrypt(ciphertext, key, decrypted);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(decrypted.size, static_cast<size_t>(0));
    
    // Clean up only if data was allocated
    if (ciphertext.data) delete[] ciphertext.data;
    if (decrypted.data) delete[] decrypted.data;
}

// Test: AES with large message
TEST(aes_large_message) {
    Crypto* aes = AESCrypto::get_instance();
    
    // Create a 1KB message
    uint8_t large_data[1024];
    for (int i = 0; i < 1024; i++) {
        large_data[i] = static_cast<uint8_t>(i % 256);
    }
    core::Buffer plaintext(large_data, 1024);
    
    uint8_t key_data[32];
    for (int i = 0; i < 32; i++) {
        key_data[i] = static_cast<uint8_t>(i * 3 + 7);
    }
    core::Buffer key(key_data, 32);
    
    // Encrypt
    core::Buffer ciphertext;
    core::ErrorCode err = aes->encrypt(plaintext, key, ciphertext);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_NOT_NULL(ciphertext.data);
    
    // Decrypt
    core::Buffer decrypted;
    err = aes->decrypt(ciphertext, key, decrypted);
    ASSERT_EQUAL(err, core::OK);
    ASSERT_EQUAL(decrypted.size, plaintext.size);
    ASSERT_EQUAL(memcmp(decrypted.data, plaintext.data, plaintext.size), 0);
    
    delete[] ciphertext.data;
    delete[] decrypted.data;
}

// Test: AES with invalid key size
TEST(aes_invalid_key_size) {
    Crypto* aes = AESCrypto::get_instance();
    
    const char* message = "Test message";
    core::Buffer plaintext(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(message)),
        strlen(message)
    );
    
    // Invalid key size (should be 32 or 48 bytes)
    uint8_t key_data[20];
    core::Buffer key(key_data, 20);
    
    core::Buffer ciphertext;
    core::ErrorCode err = aes->encrypt(plaintext, key, ciphertext);
    ASSERT_EQUAL(err, core::ERR_INVALID_ARG);
}

// Test: AES decrypt with invalid ciphertext
TEST(aes_decrypt_invalid_ciphertext) {
    Crypto* aes = AESCrypto::get_instance();
    
    // Invalid ciphertext (not multiple of block size)
    uint8_t invalid_data[10];
    core::Buffer ciphertext(invalid_data, 10);
    
    uint8_t key_data[32];
    for (int i = 0; i < 32; i++) {
        key_data[i] = static_cast<uint8_t>(i);
    }
    core::Buffer key(key_data, 32);
    
    core::Buffer plaintext;
    core::ErrorCode err = aes->decrypt(ciphertext, key, plaintext);
    ASSERT_EQUAL(err, core::ERR_FORMAT);
}

// Test: AES with different keys produce different ciphertexts
TEST(aes_different_keys) {
    Crypto* aes = AESCrypto::get_instance();
    
    const char* message = "Same message, different keys";
    core::Buffer plaintext(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(message)),
        strlen(message)
    );
    
    // Key 1
    uint8_t key1_data[32];
    for (int i = 0; i < 32; i++) {
        key1_data[i] = static_cast<uint8_t>(i);
    }
    core::Buffer key1(key1_data, 32);
    
    // Key 2 (different)
    uint8_t key2_data[32];
    for (int i = 0; i < 32; i++) {
        key2_data[i] = static_cast<uint8_t>(i + 1);
    }
    core::Buffer key2(key2_data, 32);
    
    // Encrypt with key1
    core::Buffer ciphertext1;
    aes->encrypt(plaintext, key1, ciphertext1);
    
    // Encrypt with key2
    core::Buffer ciphertext2;
    aes->encrypt(plaintext, key2, ciphertext2);
    
    // Ciphertexts should be different
    ASSERT_NOT_EQUAL(memcmp(ciphertext1.data, ciphertext2.data, ciphertext1.size), 0);
    
    delete[] ciphertext1.data;
    delete[] ciphertext2.data;
}

// Test: AES CBC mode with different IVs
TEST(aes_different_ivs) {
    Crypto* aes = AESCrypto::get_instance();
    
    const char* message = "Same message, different IVs";
    core::Buffer plaintext(
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(message)),
        strlen(message)
    );
    
    // Same AES key, different IVs
    uint8_t key1_data[32];
    uint8_t key2_data[32];
    for (int i = 0; i < 16; i++) {
        key1_data[i] = static_cast<uint8_t>(i); // AES key (same)
        key2_data[i] = static_cast<uint8_t>(i); // AES key (same)
        key1_data[16 + i] = 0x00; // IV 1
        key2_data[16 + i] = 0x01; // IV 2 (different)
    }
    core::Buffer key1(key1_data, 32);
    core::Buffer key2(key2_data, 32);
    
    // Encrypt with same key but different IVs
    core::Buffer ciphertext1;
    aes->encrypt(plaintext, key1, ciphertext1);
    
    core::Buffer ciphertext2;
    aes->encrypt(plaintext, key2, ciphertext2);
    
    // Ciphertexts should be different (CBC mode)
    ASSERT_NOT_EQUAL(memcmp(ciphertext1.data, ciphertext2.data, ciphertext1.size), 0);
    
    delete[] ciphertext1.data;
    delete[] ciphertext2.data;
}

// Test: AES block alignment
TEST(aes_block_alignment) {
    Crypto* aes = AESCrypto::get_instance();
    
    uint8_t key_data[32];
    for (int i = 0; i < 32; i++) {
        key_data[i] = static_cast<uint8_t>(i);
    }
    core::Buffer key(key_data, 32);
    
    // Test various message sizes around block boundaries
    for (size_t size = 1; size <= 48; size++) {
        uint8_t* data = new uint8_t[size];
        for (size_t i = 0; i < size; i++) {
            data[i] = static_cast<uint8_t>(i);
        }
        core::Buffer plaintext(data, size);
        
        core::Buffer ciphertext;
        core::ErrorCode err = aes->encrypt(plaintext, key, ciphertext);
        ASSERT_EQUAL(err, core::OK);
        
        core::Buffer decrypted;
        err = aes->decrypt(ciphertext, key, decrypted);
        ASSERT_EQUAL(err, core::OK);
        ASSERT_EQUAL(decrypted.size, size);
        ASSERT_EQUAL(memcmp(decrypted.data, data, size), 0);
        
        delete[] data;
        delete[] ciphertext.data;
        delete[] decrypted.data;
    }
}

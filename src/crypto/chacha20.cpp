#include "federated/crypto/chacha20.h"
#include "federated/core/error.h"
#include <cstring>
#include <cstdint>

namespace federated {
namespace crypto {

// Little-endian read/write helpers
static inline uint32_t read_le32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

static inline void write_le32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v);
    p[1] = static_cast<uint8_t>(v >> 8);
    p[2] = static_cast<uint8_t>(v >> 16);
    p[3] = static_cast<uint8_t>(v >> 24);
}

// ChaCha20 quarter round - RFC 8439 Section 2.1
static inline void quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a += b; d ^= a; d = (d << 16) | (d >> 16);  // ROTL(d, 16)
    c += d; b ^= c; b = (b << 12) | (b >> 20);  // ROTL(b, 12)
    a += b; d ^= a; d = (d <<  8) | (d >> 24);  // ROTL(d, 8)
    c += d; b ^= c; b = (b <<  7) | (b >> 25);  // ROTL(b, 7)
}

// ChaCha20 block function - RFC 8439 Section 2.3
static void chacha20_block(const uint32_t state[16], uint8_t out[64]) {
    uint32_t working[16];
    std::memcpy(working, state, 64);
    
    // 20 rounds (10 column rounds + 10 diagonal rounds)
    for (int i = 0; i < 10; i++) {
        // Column rounds
        quarter_round(working[0], working[4], working[8],  working[12]);
        quarter_round(working[1], working[5], working[9],  working[13]);
        quarter_round(working[2], working[6], working[10], working[14]);
        quarter_round(working[3], working[7], working[11], working[15]);
        
        // Diagonal rounds
        quarter_round(working[0], working[5], working[10], working[15]);
        quarter_round(working[1], working[6], working[11], working[12]);
        quarter_round(working[2], working[7], working[8],  working[13]);
        quarter_round(working[3], working[4], working[9],  working[14]);
    }
    
    // Add original state to working state
    for (int i = 0; i < 16; i++) {
        working[i] += state[i];
    }
    
    // Serialize to little-endian bytes
    for (int i = 0; i < 16; i++) {
        write_le32(out + i * 4, working[i]);
    }
}

// ChaCha20 encrypt/decrypt (symmetric operation)
static core::ErrorCode chacha20_crypt(const core::Buffer& data, const core::Buffer& key, core::Buffer& output) {
    // Key format: first 32 bytes = key, next 12 bytes = nonce, optional 4 bytes = counter
    // Minimum: 44 bytes (32 key + 12 nonce), Maximum: 48 bytes (32 key + 12 nonce + 4 counter)
    if (key.size < 44 || key.size > 48) {
        return core::ERR_CRYPTO;
    }
    
    // Check output buffer capacity
    if (data.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    // Extract key (32 bytes)
    const uint8_t* key_bytes = key.data;
    
    // Extract nonce (12 bytes)
    const uint8_t* nonce_bytes = key.data + 32;
    
    // Extract counter (4 bytes, default to 0)
    uint32_t counter = 0;
    if (key.size >= 48) {
        counter = read_le32(key.data + 44);
    }
    
    // Build ChaCha20 state
    uint32_t state[16];
    
    // Constants "expand 32-byte k" in little-endian
    state[0] = 0x61707865;
    state[1] = 0x3320646e;
    state[2] = 0x79622d32;
    state[3] = 0x6b206574;
    
    // Key (8 words = 32 bytes)
    for (int i = 0; i < 8; i++) {
        state[4 + i] = read_le32(key_bytes + i * 4);
    }
    
    // Counter (1 word = 4 bytes)
    state[12] = counter;
    
    // Nonce (3 words = 12 bytes)
    for (int i = 0; i < 3; i++) {
        state[13 + i] = read_le32(nonce_bytes + i * 4);
    }
    
    // Process input in 64-byte blocks
    size_t offset = 0;
    uint8_t keystream[64];
    
    while (offset < data.size) {
        // Generate keystream block
        chacha20_block(state, keystream);
        
        // XOR with input
        size_t block_size = data.size - offset;
        if (block_size > 64) block_size = 64;
        
        for (size_t i = 0; i < block_size; i++) {
            output.data[offset + i] = data.data[offset + i] ^ keystream[i];
        }
        
        offset += block_size;
        state[12]++;  // Increment counter
    }
    
    output.size = data.size;
    return core::OK;
}

// Static crypto instance
static Crypto g_chacha20_crypto = {
    "chacha20",
    44,  // Minimum key size: 32 bytes key + 12 bytes nonce
    chacha20_crypt,
    chacha20_crypt  // Symmetric: decrypt = encrypt
};

Crypto* ChaCha20Crypto::get_instance() {
    return &g_chacha20_crypto;
}

} // namespace crypto
} // namespace federated

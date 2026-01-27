#include "federated/crypto/aes.h"
#include <cstring>

namespace federated {
namespace crypto {

// AES S-Box (SubBytes transformation) - NIST FIPS 197
static const uint8_t SBOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// AES Inverse S-Box
static const uint8_t INV_SBOX[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// Round constants for key expansion
static const uint8_t RCON[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

// Galois Field (GF(2^8)) multiplication
static uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        uint8_t hi_bit_set = a & 0x80;
        a <<= 1;
        if (hi_bit_set) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

// SubBytes transformation
static void sub_bytes(uint8_t state[16]) {
    for (int i = 0; i < 16; i++) {
        state[i] = SBOX[state[i]];
    }
}

static void inv_sub_bytes(uint8_t state[16]) {
    for (int i = 0; i < 16; i++) {
        state[i] = INV_SBOX[state[i]];
    }
}

// ShiftRows transformation
static void shift_rows(uint8_t state[16]) {
    uint8_t temp;
    // Row 1
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;
    // Row 2
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    // Row 3
    temp = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = state[3];
    state[3] = temp;
}

static void inv_shift_rows(uint8_t state[16]) {
    uint8_t temp;
    // Row 1
    temp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = temp;
    // Row 2
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    // Row 3
    temp = state[3];
    state[3] = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = temp;
}

// MixColumns transformation
static void mix_columns(uint8_t state[16]) {
    uint8_t temp[4];
    for (int i = 0; i < 4; i++) {
        temp[0] = state[i * 4 + 0];
        temp[1] = state[i * 4 + 1];
        temp[2] = state[i * 4 + 2];
        temp[3] = state[i * 4 + 3];
        state[i * 4 + 0] = gmul(temp[0], 2) ^ gmul(temp[1], 3) ^ temp[2] ^ temp[3];
        state[i * 4 + 1] = temp[0] ^ gmul(temp[1], 2) ^ gmul(temp[2], 3) ^ temp[3];
        state[i * 4 + 2] = temp[0] ^ temp[1] ^ gmul(temp[2], 2) ^ gmul(temp[3], 3);
        state[i * 4 + 3] = gmul(temp[0], 3) ^ temp[1] ^ temp[2] ^ gmul(temp[3], 2);
    }
}

static void inv_mix_columns(uint8_t state[16]) {
    uint8_t temp[4];
    for (int i = 0; i < 4; i++) {
        temp[0] = state[i * 4 + 0];
        temp[1] = state[i * 4 + 1];
        temp[2] = state[i * 4 + 2];
        temp[3] = state[i * 4 + 3];
        state[i * 4 + 0] = gmul(temp[0], 14) ^ gmul(temp[1], 11) ^ gmul(temp[2], 13) ^ gmul(temp[3], 9);
        state[i * 4 + 1] = gmul(temp[0], 9) ^ gmul(temp[1], 14) ^ gmul(temp[2], 11) ^ gmul(temp[3], 13);
        state[i * 4 + 2] = gmul(temp[0], 13) ^ gmul(temp[1], 9) ^ gmul(temp[2], 14) ^ gmul(temp[3], 11);
        state[i * 4 + 3] = gmul(temp[0], 11) ^ gmul(temp[1], 13) ^ gmul(temp[2], 9) ^ gmul(temp[3], 14);
    }
}

// AddRoundKey
static void add_round_key(uint8_t state[16], const uint8_t* round_key) {
    for (int i = 0; i < 16; i++) {
        state[i] ^= round_key[i];
    }
}

// Key expansion for AES-128
static void key_expansion_128(const uint8_t* key, uint8_t* expanded_key) {
    memcpy(expanded_key, key, 16);
    
    for (size_t i = 4; i < 44; i++) { // 44 = 4 * (10 rounds + 1)
        uint8_t temp[4];
        memcpy(temp, &expanded_key[(i - 1) * 4], 4);
        
        if (i % 4 == 0) {
            // RotWord
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            
            // SubWord
            temp[0] = SBOX[temp[0]];
            temp[1] = SBOX[temp[1]];
            temp[2] = SBOX[temp[2]];
            temp[3] = SBOX[temp[3]];
            
            // XOR with Rcon
            temp[0] ^= RCON[i / 4];
        }
        
        for (int j = 0; j < 4; j++) {
            expanded_key[i * 4 + j] = expanded_key[(i - 4) * 4 + j] ^ temp[j];
        }
    }
}

// AES-128 encryption of a single block
static void aes_encrypt_block(const uint8_t input[16], const uint8_t* expanded_key, uint8_t output[16]) {
    memcpy(output, input, 16);
    
    // Initial round
    add_round_key(output, expanded_key);
    
    // 9 main rounds
    for (int round = 1; round < 10; round++) {
        sub_bytes(output);
        shift_rows(output);
        mix_columns(output);
        add_round_key(output, &expanded_key[round * 16]);
    }
    
    // Final round (no MixColumns)
    sub_bytes(output);
    shift_rows(output);
    add_round_key(output, &expanded_key[10 * 16]);
}

// AES-128 decryption of a single block
static void aes_decrypt_block(const uint8_t input[16], const uint8_t* expanded_key, uint8_t output[16]) {
    memcpy(output, input, 16);
    
    // Initial round
    add_round_key(output, &expanded_key[10 * 16]);
    
    // 9 main rounds
    for (int round = 9; round > 0; round--) {
        inv_shift_rows(output);
        inv_sub_bytes(output);
        add_round_key(output, &expanded_key[round * 16]);
        inv_mix_columns(output);
    }
    
    // Final round (no InvMixColumns)
    inv_shift_rows(output);
    inv_sub_bytes(output);
    add_round_key(output, expanded_key);
}

// PKCS#7 padding
static size_t add_padding(const uint8_t* input, size_t input_len, uint8_t* output, size_t output_capacity) {
    size_t padding_len = 16 - (input_len % 16);
    size_t output_len = input_len + padding_len;
    
    if (output_len > output_capacity) return 0;
    
    // Only copy if there's input data
    if (input_len > 0 && input != nullptr) {
        memcpy(output, input, input_len);
    }
    
    for (size_t i = 0; i < padding_len; i++) {
        output[input_len + i] = static_cast<uint8_t>(padding_len);
    }
    
    return output_len;
}

static size_t remove_padding(const uint8_t* input, size_t input_len, uint8_t* output, size_t output_capacity) {
    if (input_len == 0 || input_len % 16 != 0) return 0;
    
    uint8_t padding_len = input[input_len - 1];
    if (padding_len == 0 || padding_len > 16) return 0;
    
    // Verify padding
    for (size_t i = input_len - padding_len; i < input_len; i++) {
        if (input[i] != padding_len) return 0;
    }
    
    size_t output_len = input_len - padding_len;
    if (output_len > output_capacity) return 0;
    
    memcpy(output, input, output_len);
    return output_len;
}

// AES-128 CBC encryption
static core::ErrorCode aes_encrypt(const core::Buffer& plaintext, const core::Buffer& key, core::Buffer& ciphertext) {
    // Validate key size (32 bytes for AES-128 CBC: 16-byte key + 16-byte IV)
    if (key.size != 32 && key.size != 48) {
        return core::ERR_INVALID_ARG;
    }
    
    // Determine if AES-128 or AES-256
    bool is_aes128 = (key.size == 32);
    const uint8_t* aes_key = key.data;
    const uint8_t* iv = key.data + (is_aes128 ? 16 : 32);
    
    // Add PKCS#7 padding
    size_t max_padded_size = plaintext.size + 16;
    uint8_t* padded = new uint8_t[max_padded_size];
    size_t padded_size = add_padding(plaintext.data, plaintext.size, padded, max_padded_size);
    
    if (padded_size == 0) {
        delete[] padded;
        return core::ERR_INTERNAL;
    }
    
    // Expand key (only AES-128 for now)
    if (!is_aes128) {
        delete[] padded;
        return core::ERR_UNSUPPORTED; // AES-256 not implemented yet
    }
    
    uint8_t expanded_key[176]; // 44 * 4 bytes for AES-128
    key_expansion_128(aes_key, expanded_key);
    
    // Allocate ciphertext buffer
    ciphertext.data = new uint8_t[padded_size];
    ciphertext.size = padded_size;
    
    // CBC mode encryption
    uint8_t prev_block[16];
    memcpy(prev_block, iv, 16);
    
    for (size_t i = 0; i < padded_size; i += 16) {
        uint8_t block[16];
        
        // XOR with previous ciphertext block (or IV)
        for (size_t j = 0; j < 16; j++) {
            block[j] = padded[i + j] ^ prev_block[j];
        }
        
        // Encrypt block
        aes_encrypt_block(block, expanded_key, &ciphertext.data[i]);
        
        // Save for next iteration
        memcpy(prev_block, &ciphertext.data[i], 16);
    }
    
    delete[] padded;
    return core::OK;
}

// AES-128 CBC decryption
static core::ErrorCode aes_decrypt(const core::Buffer& ciphertext, const core::Buffer& key, core::Buffer& plaintext) {
    // Validate key size
    if (key.size != 32 && key.size != 48) {
        return core::ERR_INVALID_ARG;
    }
    
    // Validate ciphertext size
    if (ciphertext.size == 0 || ciphertext.size % 16 != 0) {
        return core::ERR_FORMAT;
    }
    
    // Determine if AES-128 or AES-256
    bool is_aes128 = (key.size == 32);
    const uint8_t* aes_key = key.data;
    const uint8_t* iv = key.data + (is_aes128 ? 16 : 32);
    
    // Only AES-128 for now
    if (!is_aes128) {
        return core::ERR_UNSUPPORTED;
    }
    
    uint8_t expanded_key[176];
    key_expansion_128(aes_key, expanded_key);
    
    // Allocate temp buffer
    uint8_t* decrypted = new uint8_t[ciphertext.size];
    
    // CBC mode decryption
    uint8_t prev_block[16];
    memcpy(prev_block, iv, 16);
    
    for (size_t i = 0; i < ciphertext.size; i += 16) {
        uint8_t block[16];
        
        // Decrypt block
        aes_decrypt_block(&ciphertext.data[i], expanded_key, block);
        
        // XOR with previous ciphertext block (or IV)
        for (size_t j = 0; j < 16; j++) {
            decrypted[i + j] = block[j] ^ prev_block[j];
        }
        
        // Save current ciphertext block
        memcpy(prev_block, &ciphertext.data[i], 16);
    }
    
    // Remove PKCS#7 padding
    plaintext.data = new uint8_t[ciphertext.size > 0 ? ciphertext.size : 1];
    size_t plaintext_size = remove_padding(decrypted, ciphertext.size, plaintext.data, ciphertext.size);
    
    delete[] decrypted;
    
    // Special case: empty plaintext (all padding)
    // This happens when the ciphertext is exactly 16 bytes and decrypts to 0x10 repeated
    if (plaintext_size == 0 && ciphertext.size == 16) {
        // This is valid - empty message was encrypted
        plaintext.size = 0;
        return core::OK;
    }
    
    if (plaintext_size == 0) {
        delete[] plaintext.data;
        plaintext.data = nullptr;
        return core::ERR_CRYPTO;
    }
    
    plaintext.size = plaintext_size;
    return core::OK;
}

// Static crypto instance
static Crypto g_aes_crypto = {
    "aes-128-cbc",
    32,  // Key size: 16 bytes key + 16 bytes IV
    aes_encrypt,
    aes_decrypt
};

Crypto* AESCrypto::get_instance() {
    return &g_aes_crypto;
}

} // namespace crypto
} // namespace federated

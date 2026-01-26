#include "../../include/federated/crypto/xor_stream.h"
#include <cstring>

namespace federated {
namespace crypto {

// XOR stream cipher implementation
static core::ErrorCode xor_stream_encrypt(const core::Buffer& data, const core::Buffer& key, core::Buffer& output) {
    // Validate key size (1-256 bytes)
    if (key.size == 0 || key.size > 256) {
        return core::ERR_CRYPTO;
    }
    
    // Check output buffer capacity
    if (data.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    // XOR each byte with repeating key stream
    for (size_t i = 0; i < data.size; i++) {
        output.data[i] = data.data[i] ^ key.data[i % key.size];
    }
    
    output.size = data.size;
    return core::OK;
}

static core::ErrorCode xor_stream_decrypt(const core::Buffer& data, const core::Buffer& key, core::Buffer& output) {
    // XOR is symmetric: decryption = encryption
    return xor_stream_encrypt(data, key, output);
}

// Static crypto instance
static Crypto g_xor_stream_crypto = {
    "xor_stream",
    0,  // Variable key size (1-256 bytes)
    xor_stream_encrypt,
    xor_stream_decrypt
};

Crypto* XORStreamCrypto::get_instance() {
    return &g_xor_stream_crypto;
}

} // namespace crypto
} // namespace federated

#include "../../include/federated/crypto/none.h"
#include <cstring>

namespace federated {
namespace crypto {

// No-crypto implementation - simple pass-through
static core::ErrorCode none_encrypt(const core::Buffer& data, const core::Buffer& key, core::Buffer& output) {
    (void)key;  // Unused
    
    if (data.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(output.data, data.data, data.size);
    output.size = data.size;
    
    return core::OK;
}

static core::ErrorCode none_decrypt(const core::Buffer& data, const core::Buffer& key, core::Buffer& output) {
    (void)key;  // Unused
    
    if (data.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(output.data, data.data, data.size);
    output.size = data.size;
    
    return core::OK;
}

// Static crypto instance
static Crypto g_none_crypto = {
    "none",
    0,  // No key needed
    none_encrypt,
    none_decrypt
};

Crypto* NoneCrypto::get_instance() {
    return &g_none_crypto;
}

} // namespace crypto
} // namespace federated

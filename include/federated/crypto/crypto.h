#ifndef FEDERATED_CRYPTO_CRYPTO_H
#define FEDERATED_CRYPTO_CRYPTO_H

#include "../core/buffer.h"
#include "../core/error.h"
#include <cstddef>

namespace federated {
namespace crypto {

/**
 * Crypto - Cryptography interface
 * 
 * Supports multiple tiers:
 * - Tier 0: None
 * - Tier 1: XOR/basic
 * - Tier 2: ChaCha20 / AES
 * - Tier 3: Public-key
 * - Tier 4: Advanced / future
 * 
 * UX must make crypto inspectable and understandable.
 */
struct Crypto {
    const char* name;
    size_t key_size;
    
    // Encrypt data in-place or to output buffer
    core::ErrorCode (*encrypt)(const core::Buffer& data, const core::Buffer& key, core::Buffer& output);
    
    // Decrypt data in-place or to output buffer
    core::ErrorCode (*decrypt)(const core::Buffer& data, const core::Buffer& key, core::Buffer& output);
};

} // namespace crypto
} // namespace federated

#endif // FEDERATED_CRYPTO_CRYPTO_H

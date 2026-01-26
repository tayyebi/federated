#ifndef FEDERATED_CRYPTO_XOR_STREAM_H
#define FEDERATED_CRYPTO_XOR_STREAM_H

#include "crypto.h"

namespace federated {
namespace crypto {

/**
 * XOR stream cipher - simple repeating key XOR
 * 
 * Tier 1: Obfuscation only, not secure encryption
 * 
 * This is a simple XOR cipher with a repeating key stream.
 * For each byte: output[i] = data[i] ^ key[i % key_size]
 * 
 * WARNING: This provides obfuscation only, NOT cryptographic security.
 * Do not use for protecting sensitive data.
 * 
 * Key size: variable (1-256 bytes)
 * Encryption and decryption are identical (XOR is symmetric)
 */
struct XORStreamCrypto {
    static Crypto* get_instance();
};

} // namespace crypto
} // namespace federated

#endif // FEDERATED_CRYPTO_XOR_STREAM_H

#ifndef FEDERATED_CRYPTO_AES_H
#define FEDERATED_CRYPTO_AES_H

#include "crypto.h"

/**
 * AES Encryption (Advanced Encryption Standard)
 * 
 * NIST FIPS 197 compliant implementation of AES-128 with CBC mode.
 * 
 * References:
 * - NIST FIPS 197 - Advanced Encryption Standard (AES)
 * - RFC 3602 - The AES-CBC Cipher Algorithm and Its Use with IPsec
 * 
 * Key Format (for CBC mode):
 * - Bytes 0-15: AES-128 key (16 bytes)
 * - Bytes 16-31: IV (16 bytes)
 * Total: 32 bytes
 * 
 * For AES-256:
 * - Bytes 0-31: AES-256 key (32 bytes)
 * - Bytes 32-47: IV (16 bytes)
 * Total: 48 bytes
 * 
 * Features:
 * - PKCS#7 padding for CBC mode
 * - S-Box and inverse S-Box lookup tables
 * - Constant-time operations for security
 * 
 * Block size: 128 bits (16 bytes)
 */

namespace federated {
namespace crypto {

struct AESCrypto {
    static Crypto* get_instance();
};

} // namespace crypto
} // namespace federated

#endif // FEDERATED_CRYPTO_AES_H

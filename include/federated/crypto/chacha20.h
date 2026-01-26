#ifndef FEDERATED_CRYPTO_CHACHA20_H
#define FEDERATED_CRYPTO_CHACHA20_H

#include "crypto.h"

namespace federated {
namespace crypto {

/**
 * ChaCha20 Stream Cipher (Tier 2 - Strong Encryption)
 * 
 * RFC 8439 - ChaCha20 and Poly1305 for IETF Protocols
 * https://datatracker.ietf.org/doc/html/rfc8439
 * 
 * Modern, fast stream cipher by Daniel J. Bernstein.
 * Secure alternative to AES with better performance on platforms without AES-NI.
 * 
 * Algorithm:
 * - 256-bit key (32 bytes)
 * - 96-bit nonce (12 bytes)
 * - 32-bit counter (4 bytes, optional)
 * - 20 rounds of mixing
 * - XOR keystream with plaintext
 * 
 * Security:
 * - Cryptographically secure when used correctly
 * - Never reuse nonce with same key
 * - Counter starts at 0 by default
 * 
 * Implementation:
 * - Zero external dependencies
 * - Pure C++ implementation from RFC specification
 * - Constant-time operations where feasible
 * 
 * Key buffer format:
 * - Bytes 0-31: 256-bit key (required)
 * - Bytes 32-43: 96-bit nonce (required)
 * - Bytes 44-47: 32-bit counter (optional, defaults to 0)
 * 
 * Total key buffer size: 44 bytes (minimum) or 48 bytes (with counter)
 */
struct ChaCha20Crypto {
    static Crypto* get_instance();
};

} // namespace crypto
} // namespace federated

#endif // FEDERATED_CRYPTO_CHACHA20_H

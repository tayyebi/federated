#ifndef FEDERATED_CRYPTO_NONE_H
#define FEDERATED_CRYPTO_NONE_H

#include "crypto.h"

namespace federated {
namespace crypto {

/**
 * No-crypto module - pass-through, no encryption
 * 
 * Tier 0: No encryption
 * Use when encryption is not needed or not possible.
 */
struct NoneCrypto {
    static Crypto* get_instance();
};

} // namespace crypto
} // namespace federated

#endif // FEDERATED_CRYPTO_NONE_H

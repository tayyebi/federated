#ifndef FEDERATED_FRAMER_LENGTH_PREFIX_H
#define FEDERATED_FRAMER_LENGTH_PREFIX_H

#include "framer.h"

namespace federated {
namespace framer {

/**
 * Length-prefix framer - prepends 4-byte length header
 * 
 * Format: [4-byte length in network byte order][payload]
 * Suitable for stream-based transports like TCP.
 */
struct LengthPrefixFramer {
    static Framer* get_instance();
};

} // namespace framer
} // namespace federated

#endif // FEDERATED_FRAMER_LENGTH_PREFIX_H

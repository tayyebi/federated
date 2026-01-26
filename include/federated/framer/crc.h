#ifndef FEDERATED_FRAMER_CRC_H
#define FEDERATED_FRAMER_CRC_H

#include "framer.h"

namespace federated {
namespace framer {

/**
 * CRC framer - length prefix with CRC32 checksum for integrity verification
 * 
 * Frame format: [4-byte length][payload][4-byte CRC32]
 * - Length: payload size in bytes (network byte order)
 * - Payload: the actual data
 * - CRC32: IEEE 802.3 checksum over payload (network byte order)
 * 
 * CRC32 uses the standard IEEE 802.3 polynomial: 0x04C11DB7
 * This provides strong error detection for corrupted frames.
 * 
 * On decode, the CRC is verified and ERR_FORMAT is returned if it doesn't match,
 * providing reliable detection of transmission errors.
 */
struct CRCFramer {
    static Framer* get_instance();
};

} // namespace framer
} // namespace federated

#endif // FEDERATED_FRAMER_CRC_H

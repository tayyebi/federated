#ifndef FEDERATED_CORE_ENDIAN_H
#define FEDERATED_CORE_ENDIAN_H

#include <cstdint>

#if !defined(_WIN32) && !defined(_WIN64)
    #include <arpa/inet.h>
#endif

namespace federated {
namespace core {

/**
 * Portable byte order conversion functions
 * Works on both POSIX and Windows systems
 */

// Host to network byte order (32-bit)
inline uint32_t hton32(uint32_t value) {
#if defined(_WIN32) || defined(_WIN64)
    // Windows byte order - manual conversion
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x000000FF) << 24);
#else
    // POSIX systems - use system functions
    return htonl(value);
#endif
}

// Network to host byte order (32-bit)
inline uint32_t ntoh32(uint32_t value) {
#if defined(_WIN32) || defined(_WIN64)
    // Windows byte order - manual conversion
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x000000FF) << 24);
#else
    // POSIX systems - use system functions
    return ntohl(value);
#endif
}

} // namespace core
} // namespace federated

#endif // FEDERATED_CORE_ENDIAN_H

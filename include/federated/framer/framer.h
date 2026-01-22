#ifndef FEDERATED_FRAMER_FRAMER_H
#define FEDERATED_FRAMER_FRAMER_H

#include "../core/buffer.h"
#include "../core/error.h"

namespace federated {
namespace framer {

/**
 * Framer - Frame encoding/decoding interface
 * 
 * Supports transport-specific adaptations.
 */
struct Framer {
    const char* name;
    
    // Encode payload into a frame
    // Returns encoded frame in output buffer
    core::ErrorCode (*encode)(const core::Buffer& payload, core::Buffer& output);
    
    // Decode frame into payload
    // Returns decoded payload in output buffer
    core::ErrorCode (*decode)(const core::Buffer& frame, core::Buffer& output);
};

} // namespace framer
} // namespace federated

#endif // FEDERATED_FRAMER_FRAMER_H

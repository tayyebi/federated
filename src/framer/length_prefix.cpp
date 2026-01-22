#include "../../include/federated/framer/length_prefix.h"
#include "../../include/federated/core/endian.h"
#include <cstring>
#include <cstdint>

namespace federated {
namespace framer {

// Length-prefix framer implementation
// Format: [4-byte length in network byte order][payload]

static core::ErrorCode length_prefix_encode(const core::Buffer& payload, core::Buffer& output) {
    // Check for integer overflow
    if (payload.size > UINT32_MAX - 4) {
        return core::ERR_FORMAT;
    }
    
    // Need space for 4-byte header + payload
    if (output.size < payload.size + 4) {
        return core::ERR_FORMAT;
    }
    
    // Write length header in network byte order
    uint32_t length = core::hton32(payload.size);
    memcpy(output.data, &length, 4);
    
    // Write payload
    memcpy(output.data + 4, payload.data, payload.size);
    
    output.size = payload.size + 4;
    
    return core::OK;
}

static core::ErrorCode length_prefix_decode(const core::Buffer& frame, core::Buffer& output) {
    // Need at least 4 bytes for header
    if (frame.size < 4) {
        return core::ERR_FORMAT;
    }
    
    // Read length header
    uint32_t length;
    memcpy(&length, frame.data, 4);
    length = core::ntoh32(length);
    
    // Check for integer overflow
    if (length > UINT32_MAX - 4) {
        return core::ERR_FORMAT;
    }
    
    // Verify frame has enough data
    if (frame.size < length + 4) {
        return core::ERR_FORMAT;
    }
    
    // Verify output buffer is large enough
    if (output.size < length) {
        return core::ERR_FORMAT;
    }
    
    // Extract payload
    memcpy(output.data, frame.data + 4, length);
    output.size = length;
    
    return core::OK;
}

// Static framer instance
static Framer g_length_prefix_framer = {
    "length_prefix",
    length_prefix_encode,
    length_prefix_decode
};

Framer* LengthPrefixFramer::get_instance() {
    return &g_length_prefix_framer;
}

} // namespace framer
} // namespace federated

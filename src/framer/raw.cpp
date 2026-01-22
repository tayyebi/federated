#include "../../include/federated/framer/raw.h"
#include <cstring>

namespace federated {
namespace framer {

// Raw framer implementation - simple pass-through
static core::ErrorCode raw_encode(const core::Buffer& payload, core::Buffer& output) {
    if (payload.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(output.data, payload.data, payload.size);
    output.size = payload.size;
    
    return core::OK;
}

static core::ErrorCode raw_decode(const core::Buffer& frame, core::Buffer& output) {
    if (frame.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(output.data, frame.data, frame.size);
    output.size = frame.size;
    
    return core::OK;
}

// Static framer instance
static Framer g_raw_framer = {
    "raw",
    raw_encode,
    raw_decode
};

Framer* RawFramer::get_instance() {
    return &g_raw_framer;
}

} // namespace framer
} // namespace federated

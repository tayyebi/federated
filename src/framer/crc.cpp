#include "../../include/federated/framer/crc.h"
#include "../../include/federated/core/endian.h"
#include <cstring>
#include <cstdint>

namespace federated {
namespace framer {

// CRC32 implementation using IEEE 802.3 polynomial (0x04C11DB7)
// This is the standard CRC32 used in Ethernet, ZIP, PNG, etc.

static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

static void crc32_init_table() {
    if (crc32_table_initialized) {
        return;
    }
    
    const uint32_t polynomial = 0xEDB88320; // Reversed polynomial for bit-reflected algorithm
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
    
    crc32_table_initialized = true;
}

static uint32_t crc32_compute(const uint8_t* data, uint32_t length) {
    crc32_init_table();
    
    uint32_t crc = 0xFFFFFFFF; // Initial value
    
    for (uint32_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }
    
    return crc ^ 0xFFFFFFFF; // Final XOR
}

// CRC framer implementation
// Format: [4-byte length][payload][4-byte CRC32]

static core::ErrorCode crc_encode(const core::Buffer& payload, core::Buffer& output) {
    // Check for integer overflow
    if (payload.size > UINT32_MAX - 8) {
        return core::ERR_FORMAT;
    }
    
    // Need space for 4-byte length header + payload + 4-byte CRC
    if (output.size < payload.size + 8) {
        return core::ERR_FORMAT;
    }
    
    // Write length header in network byte order
    uint32_t length = core::hton32(payload.size);
    memcpy(output.data, &length, 4);
    
    // Write payload
    memcpy(output.data + 4, payload.data, payload.size);
    
    // Compute and write CRC32 in network byte order
    uint32_t crc = crc32_compute(static_cast<const uint8_t*>(payload.data), payload.size);
    crc = core::hton32(crc);
    memcpy(output.data + 4 + payload.size, &crc, 4);
    
    output.size = payload.size + 8;
    
    return core::OK;
}

static core::ErrorCode crc_decode(const core::Buffer& frame, core::Buffer& output) {
    // Need at least 8 bytes for header + CRC
    if (frame.size < 8) {
        return core::ERR_FORMAT;
    }
    
    // Read length header
    uint32_t length;
    memcpy(&length, frame.data, 4);
    length = core::ntoh32(length);
    
    // Check for integer overflow
    if (length > UINT32_MAX - 8) {
        return core::ERR_FORMAT;
    }
    
    // Verify frame has enough data (length + payload + CRC)
    if (frame.size < length + 8) {
        return core::ERR_FORMAT;
    }
    
    // Verify output buffer is large enough
    if (output.size < length) {
        return core::ERR_FORMAT;
    }
    
    // Read CRC from frame
    uint32_t received_crc;
    memcpy(&received_crc, static_cast<const uint8_t*>(frame.data) + 4 + length, 4);
    received_crc = core::ntoh32(received_crc);
    
    // Compute CRC of payload
    uint32_t computed_crc = crc32_compute(static_cast<const uint8_t*>(frame.data) + 4, length);
    
    // Verify CRC matches
    if (received_crc != computed_crc) {
        return core::ERR_FORMAT;
    }
    
    // Extract payload
    memcpy(output.data, static_cast<const uint8_t*>(frame.data) + 4, length);
    output.size = length;
    
    return core::OK;
}

// Static framer instance
static Framer g_crc_framer = {
    "crc",
    crc_encode,
    crc_decode
};

Framer* CRCFramer::get_instance() {
    return &g_crc_framer;
}

} // namespace framer
} // namespace federated

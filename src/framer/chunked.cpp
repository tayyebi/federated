#include "../../include/federated/framer/chunked.h"
#include <cstring>
#include <cstdint>

namespace federated {
namespace framer {

// Maximum chunk size for encoding (1024 bytes)
static const uint32_t MAX_CHUNK_SIZE = 1024;

// Helper: Convert hex character to value (0-15), returns -1 on error
static int hex_to_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

// Helper: Convert value to lowercase hex character
static char value_to_hex(uint32_t val) {
    if (val < 10) {
        return '0' + val;
    }
    return 'a' + (val - 10);
}

// Helper: Write uint32 as hex string, returns number of characters written
static uint32_t write_hex(uint8_t* dest, uint32_t value) {
    if (value == 0) {
        dest[0] = '0';
        return 1;
    }
    
    // Find the highest nibble
    uint32_t nibbles = 0;
    uint32_t temp = value;
    while (temp > 0) {
        nibbles++;
        temp >>= 4;
    }
    
    // Write hex digits
    for (uint32_t i = 0; i < nibbles; i++) {
        uint32_t nibble = (value >> ((nibbles - 1 - i) * 4)) & 0xF;
        dest[i] = value_to_hex(nibble);
    }
    
    return nibbles;
}

// Chunked encoding implementation
// Format: [size_hex]\r\n[data]\r\n ... 0\r\n\r\n
static core::ErrorCode chunked_encode(const core::Buffer& payload, core::Buffer& output) {
    uint32_t payload_remaining = payload.size;
    uint32_t payload_offset = 0;
    uint32_t output_offset = 0;
    
    // Encode chunks
    while (payload_remaining > 0) {
        // Determine chunk size (up to MAX_CHUNK_SIZE)
        uint32_t chunk_size = (payload_remaining > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : payload_remaining;
        
        // Calculate space needed for this chunk: hex_size + "\r\n" + data + "\r\n"
        // Maximum hex size is 8 chars (for max uint32), but for 1024 it's 3 chars
        uint32_t hex_chars = 0;
        uint32_t temp = chunk_size;
        if (temp == 0) {
            hex_chars = 1;
        } else {
            while (temp > 0) {
                hex_chars++;
                temp >>= 4;
            }
        }
        
        uint32_t chunk_overhead = hex_chars + 2 + 2; // hex + \r\n + \r\n
        uint32_t space_needed = chunk_overhead + chunk_size;
        
        // Check if we have enough space
        if (output_offset + space_needed > output.size) {
            return core::ERR_FORMAT;
        }
        
        // Write chunk size in hex
        uint32_t written = write_hex(output.data + output_offset, chunk_size);
        output_offset += written;
        
        // Write \r\n
        output.data[output_offset++] = '\r';
        output.data[output_offset++] = '\n';
        
        // Write chunk data
        memcpy(output.data + output_offset, payload.data + payload_offset, chunk_size);
        output_offset += chunk_size;
        
        // Write \r\n
        output.data[output_offset++] = '\r';
        output.data[output_offset++] = '\n';
        
        payload_offset += chunk_size;
        payload_remaining -= chunk_size;
    }
    
    // Write final zero chunk: 0\r\n\r\n
    if (output_offset + 5 > output.size) {
        return core::ERR_FORMAT;
    }
    
    output.data[output_offset++] = '0';
    output.data[output_offset++] = '\r';
    output.data[output_offset++] = '\n';
    output.data[output_offset++] = '\r';
    output.data[output_offset++] = '\n';
    
    output.size = output_offset;
    return core::OK;
}

// Chunked decoding implementation
static core::ErrorCode chunked_decode(const core::Buffer& frame, core::Buffer& output) {
    uint32_t frame_offset = 0;
    uint32_t output_offset = 0;
    
    while (frame_offset < frame.size) {
        // Parse chunk size (hex)
        uint32_t chunk_size = 0;
        bool found_hex = false;
        
        while (frame_offset < frame.size) {
            char c = frame.data[frame_offset];
            
            // Check for \r (end of hex size)
            if (c == '\r') {
                break;
            }
            
            // Parse hex digit
            int hex_val = hex_to_value(c);
            if (hex_val < 0) {
                return core::ERR_FORMAT;
            }
            
            found_hex = true;
            
            // Check for overflow
            if (chunk_size > (UINT32_MAX >> 4)) {
                return core::ERR_FORMAT;
            }
            
            chunk_size = (chunk_size << 4) | hex_val;
            frame_offset++;
        }
        
        if (!found_hex) {
            return core::ERR_FORMAT;
        }
        
        // Expect \r\n after chunk size
        if (frame_offset + 2 > frame.size) {
            return core::ERR_FORMAT;
        }
        if (frame.data[frame_offset] != '\r' || frame.data[frame_offset + 1] != '\n') {
            return core::ERR_FORMAT;
        }
        frame_offset += 2;
        
        // If chunk size is 0, this is the final chunk
        if (chunk_size == 0) {
            // Expect final \r\n
            if (frame_offset + 2 > frame.size) {
                return core::ERR_FORMAT;
            }
            if (frame.data[frame_offset] != '\r' || frame.data[frame_offset + 1] != '\n') {
                return core::ERR_FORMAT;
            }
            frame_offset += 2;
            
            // Should be at end of frame
            if (frame_offset != frame.size) {
                return core::ERR_FORMAT;
            }
            
            break;
        }
        
        // Read chunk data
        if (frame_offset + chunk_size > frame.size) {
            return core::ERR_FORMAT;
        }
        
        // Check output buffer space
        if (output_offset + chunk_size > output.size) {
            return core::ERR_FORMAT;
        }
        
        memcpy(output.data + output_offset, frame.data + frame_offset, chunk_size);
        output_offset += chunk_size;
        frame_offset += chunk_size;
        
        // Expect \r\n after chunk data
        if (frame_offset + 2 > frame.size) {
            return core::ERR_FORMAT;
        }
        if (frame.data[frame_offset] != '\r' || frame.data[frame_offset + 1] != '\n') {
            return core::ERR_FORMAT;
        }
        frame_offset += 2;
    }
    
    output.size = output_offset;
    return core::OK;
}

// Static framer instance
static Framer g_chunked_framer = {
    "chunked",
    chunked_encode,
    chunked_decode
};

Framer* ChunkedFramer::get_instance() {
    return &g_chunked_framer;
}

} // namespace framer
} // namespace federated

#ifndef FEDERATED_FRAMER_CHUNKED_H
#define FEDERATED_FRAMER_CHUNKED_H

#include "framer.h"

namespace federated {
namespace framer {

/**
 * Chunked framer - HTTP-style chunked transfer encoding
 * 
 * Implements chunked transfer encoding as defined in RFC 9112 Section 7.1.
 * This encoding allows streaming of data without knowing the total length upfront.
 * 
 * Frame format:
 *   [chunk1_size_hex]\r\n[chunk1_data]\r\n
 *   [chunk2_size_hex]\r\n[chunk2_data]\r\n
 *   ...
 *   0\r\n\r\n
 * 
 * - Each chunk starts with its size in hexadecimal (ASCII, without "0x" prefix)
 * - Followed by \r\n (CRLF)
 * - Then the chunk data
 * - Followed by another \r\n (CRLF)
 * - The sequence ends with a zero-size chunk: "0\r\n\r\n"
 * 
 * For encoding:
 * - Large payloads are automatically split into multiple chunks
 * - Maximum chunk size: 1024 bytes (configurable)
 * - Each chunk is formatted with hex size + data
 * - Final zero chunk marks end of transmission
 * 
 * For decoding:
 * - Parses hex chunk sizes
 * - Validates CRLF separators
 * - Reassembles payload from chunks
 * - Returns ERR_FORMAT on malformed input
 * 
 * Example encoding of "Hello":
 *   5\r\nHello\r\n0\r\n\r\n
 * 
 * Example encoding of 2048 bytes splits into 2 chunks + terminator:
 *   400\r\n[1024 bytes]\r\n400\r\n[1024 bytes]\r\n0\r\n\r\n
 */
struct ChunkedFramer {
    static Framer* get_instance();
};

} // namespace framer
} // namespace federated

#endif // FEDERATED_FRAMER_CHUNKED_H

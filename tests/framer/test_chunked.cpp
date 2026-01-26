#include "../test_runner.h"
#include "../../include/federated/framer/chunked.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::framer;
using namespace federated::core;

TEST(chunked_get_instance) {
    Framer* f = ChunkedFramer::get_instance();
    TEST_ASSERT(f != nullptr);
    TEST_ASSERT(f->name != nullptr);
    TEST_ASSERT_STR_EQ(f->name, "chunked");
}

TEST(chunked_encode_small_payload) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Small payload that fits in one chunk
    const char* msg = "Hello";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t output[256];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    
    // Expected: "5\r\nHello\r\n0\r\n\r\n" = 13 bytes
    // Let's verify the format
    TEST_ASSERT(out_buf.size >= 13);
    
    // Check that it starts with "5\r\n"
    TEST_ASSERT_EQ(output[0], '5');
    TEST_ASSERT_EQ(output[1], '\r');
    TEST_ASSERT_EQ(output[2], '\n');
    
    // Check payload
    TEST_ASSERT_EQ(memcmp(output + 3, "Hello", 5), 0);
    
    // Check \r\n after payload
    TEST_ASSERT_EQ(output[8], '\r');
    TEST_ASSERT_EQ(output[9], '\n');
    
    // Check terminator "0\r\n\r\n"
    TEST_ASSERT_EQ(output[10], '0');
    TEST_ASSERT_EQ(output[11], '\r');
    TEST_ASSERT_EQ(output[12], '\n');
    TEST_ASSERT_EQ(output[13], '\r');
    TEST_ASSERT_EQ(output[14], '\n');
}

TEST(chunked_encode_large_payload) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Create a payload larger than MAX_CHUNK_SIZE (1024 bytes)
    // Let's use 2048 bytes to get 2 chunks
    uint8_t input[2048];
    for (uint32_t i = 0; i < 2048; i++) {
        input[i] = static_cast<uint8_t>(i & 0xFF);
    }
    Buffer in_buf(input, 2048);
    
    uint8_t output[4096];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    
    // Should have 2 chunks of 1024 bytes each + terminator
    // First chunk: "400\r\n" (5 bytes) + 1024 bytes + "\r\n" (2 bytes) = 1031 bytes
    // Second chunk: "400\r\n" (5 bytes) + 1024 bytes + "\r\n" (2 bytes) = 1031 bytes
    // Terminator: "0\r\n\r\n" (5 bytes)
    // Total: 1031 + 1031 + 5 = 2067 bytes
    TEST_ASSERT_EQ(out_buf.size, 2067);
    
    // Verify first chunk header is "400" (hex for 1024)
    TEST_ASSERT_EQ(output[0], '4');
    TEST_ASSERT_EQ(output[1], '0');
    TEST_ASSERT_EQ(output[2], '0');
    TEST_ASSERT_EQ(output[3], '\r');
    TEST_ASSERT_EQ(output[4], '\n');
}

TEST(chunked_encode_empty_payload) {
    Framer* f = ChunkedFramer::get_instance();
    
    uint8_t input[1] = {0};
    Buffer in_buf(input, 0);  // Empty payload
    
    uint8_t output[256];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    
    // Expected: "0\r\n\r\n" = 5 bytes
    TEST_ASSERT_EQ(out_buf.size, 5);
    TEST_ASSERT_EQ(output[0], '0');
    TEST_ASSERT_EQ(output[1], '\r');
    TEST_ASSERT_EQ(output[2], '\n');
    TEST_ASSERT_EQ(output[3], '\r');
    TEST_ASSERT_EQ(output[4], '\n');
}

TEST(chunked_roundtrip_small) {
    Framer* f = ChunkedFramer::get_instance();
    
    const char* original = "Chunked encoding test";
    uint8_t orig_buf[64];
    memcpy(orig_buf, original, strlen(original));
    Buffer orig(orig_buf, strlen(original));
    
    // Encode
    uint8_t encoded[256];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(orig, enc_buf), OK);
    
    // Decode
    uint8_t decoded[256];
    Buffer dec_buf(decoded, sizeof(decoded));
    TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
    
    // Verify
    TEST_ASSERT_EQ(dec_buf.size, strlen(original));
    TEST_ASSERT_EQ(memcmp(decoded, original, strlen(original)), 0);
}

TEST(chunked_roundtrip_large) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Create a large payload (3000 bytes) that will split into 3 chunks
    uint8_t input[3000];
    for (uint32_t i = 0; i < 3000; i++) {
        input[i] = static_cast<uint8_t>((i * 7 + 13) & 0xFF);
    }
    Buffer in_buf(input, 3000);
    
    // Encode
    uint8_t encoded[6000];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(in_buf, enc_buf), OK);
    
    // Decode
    uint8_t decoded[3000];
    Buffer dec_buf(decoded, sizeof(decoded));
    TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
    
    // Verify
    TEST_ASSERT_EQ(dec_buf.size, 3000);
    TEST_ASSERT_EQ(memcmp(decoded, input, 3000), 0);
}

TEST(chunked_roundtrip_empty) {
    Framer* f = ChunkedFramer::get_instance();
    
    uint8_t input[1] = {0};
    Buffer in_buf(input, 0);
    
    // Encode
    uint8_t encoded[256];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(in_buf, enc_buf), OK);
    
    // Decode
    uint8_t decoded[256];
    Buffer dec_buf(decoded, sizeof(decoded));
    TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
    
    // Verify empty payload
    TEST_ASSERT_EQ(dec_buf.size, 0);
}

TEST(chunked_decode_malformed_missing_crlf) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Malformed: "5\nHello\r\n0\r\n\r\n" (missing \r after size)
    const char* malformed = "5\nHello\r\n0\r\n\r\n";
    uint8_t input[64];
    memcpy(input, malformed, strlen(malformed));
    Buffer in_buf(input, strlen(malformed));
    
    uint8_t output[256];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->decode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, ERR_FORMAT);
}

TEST(chunked_decode_malformed_invalid_hex) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Malformed: "5G\r\nHello\r\n0\r\n\r\n" (invalid hex char 'G')
    const char* malformed = "5G\r\nHello\r\n0\r\n\r\n";
    uint8_t input[64];
    memcpy(input, malformed, strlen(malformed));
    Buffer in_buf(input, strlen(malformed));
    
    uint8_t output[256];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->decode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, ERR_FORMAT);
}

TEST(chunked_decode_malformed_truncated) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Malformed: "5\r\nHel" (truncated data)
    const char* malformed = "5\r\nHel";
    uint8_t input[64];
    memcpy(input, malformed, strlen(malformed));
    Buffer in_buf(input, strlen(malformed));
    
    uint8_t output[256];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->decode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, ERR_FORMAT);
}

TEST(chunked_various_sizes) {
    Framer* f = ChunkedFramer::get_instance();
    
    // Test different payload sizes
    uint32_t sizes[] = {1, 10, 100, 255, 256, 512, 1000, 1024, 1025, 2000};
    
    for (uint32_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        uint32_t size = sizes[i];
        
        // Create payload with pattern
        uint8_t payload[2048];
        for (uint32_t j = 0; j < size; j++) {
            payload[j] = static_cast<uint8_t>((j * 3) & 0xFF);
        }
        Buffer pay_buf(payload, size);
        
        // Encode
        uint8_t encoded[4096];
        Buffer enc_buf(encoded, sizeof(encoded));
        TEST_ASSERT_EQ(f->encode(pay_buf, enc_buf), OK);
        
        // Decode
        uint8_t decoded[2048];
        Buffer dec_buf(decoded, sizeof(decoded));
        TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
        TEST_ASSERT_EQ(dec_buf.size, size);
        TEST_ASSERT_EQ(memcmp(decoded, payload, size), 0);
    }
}

#include "../test_runner.h"
#include "../../include/federated/framer/crc.h"
#include "../../include/federated/core/buffer.h"
#include "../../include/federated/core/endian.h"
#include <cstring>

using namespace federated::framer;
using namespace federated::core;

TEST(crc_get_instance) {
    Framer* f = CRCFramer::get_instance();
    TEST_ASSERT(f != nullptr);
    TEST_ASSERT(f->name != nullptr);
    TEST_ASSERT_STR_EQ(f->name, "crc");
}

TEST(crc_encode_decode_roundtrip) {
    Framer* f = CRCFramer::get_instance();
    
    const char* original = "CRC roundtrip test message";
    uint8_t orig_buf[64];
    memcpy(orig_buf, original, strlen(original));
    Buffer orig(orig_buf, strlen(original));
    
    // Encode
    uint8_t encoded[128];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(orig, enc_buf), OK);
    
    // Should be length (4) + payload + CRC (4)
    TEST_ASSERT_EQ(enc_buf.size, strlen(original) + 8);
    
    // Decode
    uint8_t decoded[128];
    Buffer dec_buf(decoded, sizeof(decoded));
    TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
    
    // Verify
    TEST_ASSERT_EQ(dec_buf.size, strlen(original));
    TEST_ASSERT_EQ(memcmp(decoded, original, strlen(original)), 0);
}

TEST(crc_corruption_detection) {
    Framer* f = CRCFramer::get_instance();
    
    const char* msg = "Test corruption detection";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    // Encode
    uint8_t encoded[128];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(in_buf, enc_buf), OK);
    
    // Corrupt the payload (flip a bit in the middle)
    encoded[10] ^= 0x01;
    
    // Try to decode - should fail with ERR_FORMAT due to CRC mismatch
    uint8_t decoded[128];
    Buffer dec_buf(decoded, sizeof(decoded));
    ErrorCode err = f->decode(enc_buf, dec_buf);
    TEST_ASSERT_EQ(err, ERR_FORMAT);
}

TEST(crc_empty_payload) {
    Framer* f = CRCFramer::get_instance();
    
    uint8_t input[1] = {0};
    Buffer in_buf(input, 0);  // Empty payload
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, 8);  // Length header (4) + CRC (4)
    
    // Verify length is 0
    uint32_t length;
    memcpy(&length, output, 4);
    length = federated::core::ntoh32(length);
    TEST_ASSERT_EQ(length, 0);
    
    // Should be able to decode empty payload
    uint8_t decoded[128];
    Buffer dec_buf(decoded, sizeof(decoded));
    TEST_ASSERT_EQ(f->decode(out_buf, dec_buf), OK);
    TEST_ASSERT_EQ(dec_buf.size, 0);
}

TEST(crc_various_payload_sizes) {
    Framer* f = CRCFramer::get_instance();
    
    // Test different payload sizes
    uint32_t sizes[] = {1, 10, 100, 255, 256, 512, 1000};
    
    for (uint32_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        uint32_t size = sizes[i];
        
        // Create payload with pattern (use stack for smaller sizes, check in chunks)
        uint8_t payload[1024];
        for (uint32_t j = 0; j < size; j++) {
            payload[j] = static_cast<uint8_t>(j & 0xFF);
        }
        Buffer pay_buf(payload, size);
        
        // Encode
        uint8_t encoded[2048];
        Buffer enc_buf(encoded, sizeof(encoded));
        TEST_ASSERT_EQ(f->encode(pay_buf, enc_buf), OK);
        TEST_ASSERT_EQ(enc_buf.size, size + 8);
        
        // Decode
        uint8_t decoded[1024];
        Buffer dec_buf(decoded, sizeof(decoded));
        TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
        TEST_ASSERT_EQ(dec_buf.size, size);
        TEST_ASSERT_EQ(memcmp(decoded, payload, size), 0);
    }
}

TEST(crc_corruption_in_crc_field) {
    Framer* f = CRCFramer::get_instance();
    
    const char* msg = "Test CRC field corruption";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    // Encode
    uint8_t encoded[128];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(in_buf, enc_buf), OK);
    
    // Corrupt the CRC field (last 4 bytes)
    encoded[enc_buf.size - 1] ^= 0xFF;
    
    // Try to decode - should fail with ERR_FORMAT
    uint8_t decoded[128];
    Buffer dec_buf(decoded, sizeof(decoded));
    ErrorCode err = f->decode(enc_buf, dec_buf);
    TEST_ASSERT_EQ(err, ERR_FORMAT);
}

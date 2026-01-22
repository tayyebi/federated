#include "../test_runner.h"
#include "../../include/federated/framer/length_prefix.h"
#include "../../include/federated/core/buffer.h"
#include "../../include/federated/core/endian.h"
#include <cstring>

using namespace federated::framer;
using namespace federated::core;

TEST(length_prefix_get_instance) {
    Framer* f = LengthPrefixFramer::get_instance();
    TEST_ASSERT(f != nullptr);
    TEST_ASSERT(f->name != nullptr);
    TEST_ASSERT_STR_EQ(f->name, "length_prefix");
}

TEST(length_prefix_encode) {
    Framer* f = LengthPrefixFramer::get_instance();
    
    const char* msg = "Length prefix test";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    
    // Should be length + 4 bytes for header
    TEST_ASSERT_EQ(out_buf.size, strlen(msg) + 4);
    
    // First 4 bytes should be length in network byte order
    uint32_t length;
    memcpy(&length, output, 4);
    length = federated::core::ntoh32(length);
    TEST_ASSERT_EQ(length, strlen(msg));
    
    // Remaining bytes should be payload
    TEST_ASSERT_EQ(memcmp(output + 4, msg, strlen(msg)), 0);
}

TEST(length_prefix_decode) {
    Framer* f = LengthPrefixFramer::get_instance();
    
    const char* msg = "Decode test";
    
    // Create framed input
    uint8_t input[128];
    uint32_t length = federated::core::hton32(strlen(msg));
    memcpy(input, &length, 4);
    memcpy(input + 4, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg) + 4);
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->decode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(output, msg, strlen(msg)), 0);
}

TEST(length_prefix_roundtrip) {
    Framer* f = LengthPrefixFramer::get_instance();
    
    const char* original = "Roundtrip with length prefix";
    uint8_t orig_buf[64];
    memcpy(orig_buf, original, strlen(original));
    Buffer orig(orig_buf, strlen(original));
    
    // Encode
    uint8_t encoded[128];
    Buffer enc_buf(encoded, sizeof(encoded));
    TEST_ASSERT_EQ(f->encode(orig, enc_buf), OK);
    
    // Decode
    uint8_t decoded[128];
    Buffer dec_buf(decoded, sizeof(decoded));
    TEST_ASSERT_EQ(f->decode(enc_buf, dec_buf), OK);
    
    // Verify
    TEST_ASSERT_EQ(dec_buf.size, strlen(original));
    TEST_ASSERT_EQ(memcmp(decoded, original, strlen(original)), 0);
}

TEST(length_prefix_empty_payload) {
    Framer* f = LengthPrefixFramer::get_instance();
    
    uint8_t input[1] = {0};
    Buffer in_buf(input, 0);  // Empty payload
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, 4);  // Just the length header
    
    // Verify length is 0
    uint32_t length;
    memcpy(&length, output, 4);
    length = federated::core::ntoh32(length);
    TEST_ASSERT_EQ(length, 0);
}

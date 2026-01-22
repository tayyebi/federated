#include "../test_runner.h"
#include "../../include/federated/framer/raw.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::framer;
using namespace federated::core;

TEST(raw_get_instance) {
    Framer* f = RawFramer::get_instance();
    TEST_ASSERT(f != nullptr);
    TEST_ASSERT(f->name != nullptr);
    TEST_ASSERT_STR_EQ(f->name, "raw");
}

TEST(raw_encode_passthrough) {
    Framer* f = RawFramer::get_instance();
    
    const char* msg = "Raw test data";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->encode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(output, msg, strlen(msg)), 0);
}

TEST(raw_decode_passthrough) {
    Framer* f = RawFramer::get_instance();
    
    const char* msg = "Raw decode test";
    uint8_t input[64];
    memcpy(input, msg, strlen(msg));
    Buffer in_buf(input, strlen(msg));
    
    uint8_t output[128];
    Buffer out_buf(output, sizeof(output));
    
    ErrorCode err = f->decode(in_buf, out_buf);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(out_buf.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(output, msg, strlen(msg)), 0);
}

TEST(raw_encode_decode_roundtrip) {
    Framer* f = RawFramer::get_instance();
    
    const char* original = "Roundtrip test";
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

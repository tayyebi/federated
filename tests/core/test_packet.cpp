#include "../test_runner.h"
#include "../../include/federated/core/packet.h"

using namespace federated::core;

TEST(packet_create_empty) {
    Packet pkt;
    TEST_ASSERT(pkt.payload.data == nullptr);
    TEST_ASSERT_EQ(pkt.payload.size, 0);
}

TEST(packet_create_with_buffer) {
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    Buffer buf(data, 8);
    Packet pkt(buf);
    
    TEST_ASSERT(pkt.payload.data == data);
    TEST_ASSERT_EQ(pkt.payload.size, 8);
    TEST_ASSERT_EQ(pkt.payload.data[0], 1);
    TEST_ASSERT_EQ(pkt.payload.data[7], 8);
}

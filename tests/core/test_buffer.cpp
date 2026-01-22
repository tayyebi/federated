#include "../test_runner.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::core;

TEST(buffer_create_empty) {
    Buffer buf;
    TEST_ASSERT(buf.data == nullptr);
    TEST_ASSERT_EQ(buf.size, 0);
}

TEST(buffer_create_with_data) {
    uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Buffer buf(data, 10);
    
    TEST_ASSERT(buf.data == data);
    TEST_ASSERT_EQ(buf.size, 10);
    TEST_ASSERT_EQ(buf.data[0], 1);
    TEST_ASSERT_EQ(buf.data[9], 10);
}

TEST(buffer_manual_assignment) {
    uint8_t data[5] = {10, 20, 30, 40, 50};
    Buffer buf;
    buf.data = data;
    buf.size = 5;
    
    TEST_ASSERT(buf.data != nullptr);
    TEST_ASSERT_EQ(buf.size, 5);
    TEST_ASSERT_EQ(buf.data[2], 30);
}

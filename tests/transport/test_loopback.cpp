#include "../test_runner.h"
#include "../../include/federated/transport/loopback.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::transport;
using namespace federated::core;

TEST(loopback_get_instance) {
    Transport* t = LoopbackTransport::get_instance();
    TEST_ASSERT(t != nullptr);
    TEST_ASSERT(t->name != nullptr);
    TEST_ASSERT_STR_EQ(t->name, "loopback");
}

TEST(loopback_open_close) {
    Transport* t = LoopbackTransport::get_instance();
    
    ErrorCode err = t->open();
    TEST_ASSERT_EQ(err, OK);
    
    err = t->close();
    TEST_ASSERT_EQ(err, OK);
}

TEST(loopback_available_when_open) {
    Transport* t = LoopbackTransport::get_instance();
    
    t->open();
    bool avail = t->available();
    TEST_ASSERT(avail);
    t->close();
}

TEST(loopback_send_and_receive) {
    Transport* t = LoopbackTransport::get_instance();
    t->open();
    
    // Send data
    const char* msg = "Hello, Federated!";
    uint8_t send_buf[128];
    memcpy(send_buf, msg, strlen(msg));
    Buffer send_buffer(send_buf, strlen(msg));
    
    ErrorCode err = t->send(send_buffer);
    TEST_ASSERT_EQ(err, OK);
    
    // Receive data
    uint8_t recv_buf[128] = {0};
    Buffer recv_buffer(recv_buf, sizeof(recv_buf));
    
    err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify received data matches sent data
    TEST_ASSERT_EQ(memcmp(send_buf, recv_buf, strlen(msg)), 0);
    
    t->close();
}

TEST(loopback_multiple_messages) {
    Transport* t = LoopbackTransport::get_instance();
    t->open();
    
    // Send first message
    const char* msg1 = "First";
    uint8_t buf1[64];
    memcpy(buf1, msg1, strlen(msg1));
    Buffer send1(buf1, strlen(msg1));
    TEST_ASSERT_EQ(t->send(send1), OK);
    
    // Send second message
    const char* msg2 = "Second";
    uint8_t buf2[64];
    memcpy(buf2, msg2, strlen(msg2));
    Buffer send2(buf2, strlen(msg2));
    TEST_ASSERT_EQ(t->send(send2), OK);
    
    // Receive first message
    uint8_t recv1[64] = {0};
    Buffer r1(recv1, sizeof(recv1));
    TEST_ASSERT_EQ(t->recv(r1), OK);
    TEST_ASSERT_EQ(memcmp(recv1, msg1, strlen(msg1)), 0);
    
    // Receive second message
    uint8_t recv2[64] = {0};
    Buffer r2(recv2, sizeof(recv2));
    TEST_ASSERT_EQ(t->recv(r2), OK);
    TEST_ASSERT_EQ(memcmp(recv2, msg2, strlen(msg2)), 0);
    
    t->close();
}

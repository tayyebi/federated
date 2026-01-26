#include "../test_runner.h"
#include "../../include/federated/transport/tcp.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::transport;
using namespace federated::core;

TEST(tcp_get_instance) {
    Transport* t = TCPTransport::get_instance();
    TEST_ASSERT(t != nullptr);
    TEST_ASSERT(t->name != nullptr);
    TEST_ASSERT_STR_EQ(t->name, "tcp");
}

TEST(tcp_open_close) {
    Transport* t = TCPTransport::get_instance();
    
    ErrorCode err = t->open();
    TEST_ASSERT_EQ(err, OK);
    
    err = t->close();
    TEST_ASSERT_EQ(err, OK);
}

TEST(tcp_available_when_open) {
    Transport* t = TCPTransport::get_instance();
    
    t->open();
    bool avail = t->available();
    TEST_ASSERT(avail);
    t->close();
}

TEST(tcp_send_and_receive) {
    Transport* t = TCPTransport::get_instance();
    t->open();
    
    // Send data
    const char* msg = "Hello, TCP!";
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
    TEST_ASSERT_EQ(recv_buffer.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(recv_buf, msg, strlen(msg)), 0);
    
    t->close();
}

TEST(tcp_multiple_messages) {
    Transport* t = TCPTransport::get_instance();
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
    // TCP is stream-based, might receive concatenated data
    TEST_ASSERT(r1.size > 0);
    
    t->close();
}

TEST(tcp_error_send_when_closed) {
    Transport* t = TCPTransport::get_instance();
    
    // Ensure transport is closed
    t->close();
    
    const char* msg = "Test";
    uint8_t buf[64];
    memcpy(buf, msg, strlen(msg));
    Buffer send_buffer(buf, strlen(msg));
    
    ErrorCode err = t->send(send_buffer);
    TEST_ASSERT_EQ(err, ERR_IO);
}

TEST(tcp_error_recv_when_closed) {
    Transport* t = TCPTransport::get_instance();
    
    // Ensure transport is closed
    t->close();
    
    uint8_t buf[64];
    Buffer recv_buffer(buf, sizeof(buf));
    
    ErrorCode err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, ERR_IO);
}

TEST(tcp_large_message) {
    Transport* t = TCPTransport::get_instance();
    t->open();
    
    // Send a larger message (but within limits)
    const size_t msg_size = 1024;
    uint8_t send_buf[msg_size];
    for (size_t i = 0; i < msg_size; i++) {
        send_buf[i] = (uint8_t)(i % 256);
    }
    Buffer send_buffer(send_buf, msg_size);
    
    ErrorCode err = t->send(send_buffer);
    TEST_ASSERT_EQ(err, OK);
    
    // Receive the data (might need multiple recv calls)
    uint8_t recv_buf[msg_size];
    Buffer recv_buffer(recv_buf, sizeof(recv_buf));
    
    err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(recv_buffer.size > 0);
    
    t->close();
}

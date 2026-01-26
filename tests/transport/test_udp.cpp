#include "../test_runner.h"
#include "../../include/federated/transport/udp.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>

using namespace federated::transport;
using namespace federated::core;

TEST(udp_get_instance) {
    Transport* t = UDPTransport::get_instance();
    TEST_ASSERT(t != nullptr);
    TEST_ASSERT(t->name != nullptr);
    TEST_ASSERT_STR_EQ(t->name, "udp");
}

TEST(udp_open_close) {
    Transport* t = UDPTransport::get_instance();
    
    ErrorCode err = t->open();
    TEST_ASSERT_EQ(err, OK);
    
    err = t->close();
    TEST_ASSERT_EQ(err, OK);
}

TEST(udp_available_when_open) {
    Transport* t = UDPTransport::get_instance();
    
    t->open();
    bool avail = t->available();
    TEST_ASSERT(avail);
    t->close();
}

TEST(udp_send_and_receive_datagram) {
    Transport* t = UDPTransport::get_instance();
    t->open();
    
    // Send datagram
    const char* msg = "Hello, UDP!";
    uint8_t send_buf[128];
    memcpy(send_buf, msg, strlen(msg));
    Buffer send_buffer(send_buf, strlen(msg));
    
    ErrorCode err = t->send(send_buffer);
    TEST_ASSERT_EQ(err, OK);
    
    // Receive datagram
    uint8_t recv_buf[128] = {0};
    Buffer recv_buffer(recv_buf, sizeof(recv_buf));
    
    err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify received data matches sent data
    TEST_ASSERT_EQ(recv_buffer.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(recv_buf, msg, strlen(msg)), 0);
    
    t->close();
}

TEST(udp_datagram_boundaries_preserved) {
    Transport* t = UDPTransport::get_instance();
    t->open();
    
    // Send first datagram
    const char* msg1 = "First datagram";
    uint8_t buf1[64];
    memcpy(buf1, msg1, strlen(msg1));
    Buffer send1(buf1, strlen(msg1));
    TEST_ASSERT_EQ(t->send(send1), OK);
    
    // Send second datagram
    const char* msg2 = "Second datagram";
    uint8_t buf2[64];
    memcpy(buf2, msg2, strlen(msg2));
    Buffer send2(buf2, strlen(msg2));
    TEST_ASSERT_EQ(t->send(send2), OK);
    
    // Receive first datagram - should receive complete first message
    uint8_t recv1[64] = {0};
    Buffer r1(recv1, sizeof(recv1));
    TEST_ASSERT_EQ(t->recv(r1), OK);
    TEST_ASSERT_EQ(r1.size, strlen(msg1));
    TEST_ASSERT_EQ(memcmp(recv1, msg1, strlen(msg1)), 0);
    
    // Receive second datagram - should receive complete second message
    uint8_t recv2[64] = {0};
    Buffer r2(recv2, sizeof(recv2));
    TEST_ASSERT_EQ(t->recv(r2), OK);
    TEST_ASSERT_EQ(r2.size, strlen(msg2));
    TEST_ASSERT_EQ(memcmp(recv2, msg2, strlen(msg2)), 0);
    
    t->close();
}

TEST(udp_multiple_datagrams) {
    Transport* t = UDPTransport::get_instance();
    t->open();
    
    // Send multiple datagrams
    const char* messages[] = {
        "Datagram A",
        "Datagram B",
        "Datagram C"
    };
    
    for (int i = 0; i < 3; i++) {
        uint8_t buf[64];
        memcpy(buf, messages[i], strlen(messages[i]));
        Buffer send_buf(buf, strlen(messages[i]));
        TEST_ASSERT_EQ(t->send(send_buf), OK);
    }
    
    // Receive all datagrams
    for (int i = 0; i < 3; i++) {
        uint8_t recv_buf[64] = {0};
        Buffer r(recv_buf, sizeof(recv_buf));
        TEST_ASSERT_EQ(t->recv(r), OK);
        TEST_ASSERT_EQ(r.size, strlen(messages[i]));
        TEST_ASSERT_EQ(memcmp(recv_buf, messages[i], strlen(messages[i])), 0);
    }
    
    t->close();
}

TEST(udp_error_send_when_closed) {
    Transport* t = UDPTransport::get_instance();
    
    // Ensure transport is closed
    t->close();
    
    const char* msg = "Test";
    uint8_t buf[64];
    memcpy(buf, msg, strlen(msg));
    Buffer send_buffer(buf, strlen(msg));
    
    ErrorCode err = t->send(send_buffer);
    TEST_ASSERT_EQ(err, ERR_IO);
}

TEST(udp_error_recv_when_closed) {
    Transport* t = UDPTransport::get_instance();
    
    // Ensure transport is closed
    t->close();
    
    uint8_t buf[64];
    Buffer recv_buffer(buf, sizeof(buf));
    
    ErrorCode err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, ERR_IO);
}

TEST(udp_recv_timeout_when_no_data) {
    Transport* t = UDPTransport::get_instance();
    t->open();
    
    // Try to receive without sending anything
    uint8_t buf[64];
    Buffer recv_buffer(buf, sizeof(buf));
    
    ErrorCode err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, ERR_TIMEOUT);
    
    t->close();
}

TEST(udp_various_datagram_sizes) {
    Transport* t = UDPTransport::get_instance();
    t->open();
    
    // Test small datagram
    const char* small = "Hi";
    uint8_t small_buf[64];
    memcpy(small_buf, small, strlen(small));
    Buffer send_small(small_buf, strlen(small));
    TEST_ASSERT_EQ(t->send(send_small), OK);
    
    // Test medium datagram (within standard MTU)
    const size_t medium_size = 512;
    uint8_t medium_buf[medium_size];
    for (size_t i = 0; i < medium_size; i++) {
        medium_buf[i] = (uint8_t)(i % 256);
    }
    Buffer send_medium(medium_buf, medium_size);
    TEST_ASSERT_EQ(t->send(send_medium), OK);
    
    // Test larger datagram (still within standard MTU)
    const size_t large_size = 1400;
    uint8_t large_buf[large_size];
    for (size_t i = 0; i < large_size; i++) {
        large_buf[i] = (uint8_t)((i * 7) % 256);
    }
    Buffer send_large(large_buf, large_size);
    TEST_ASSERT_EQ(t->send(send_large), OK);
    
    // Receive and verify small
    uint8_t recv_small[64] = {0};
    Buffer r_small(recv_small, sizeof(recv_small));
    TEST_ASSERT_EQ(t->recv(r_small), OK);
    TEST_ASSERT_EQ(r_small.size, strlen(small));
    TEST_ASSERT_EQ(memcmp(recv_small, small, strlen(small)), 0);
    
    // Receive and verify medium
    uint8_t recv_medium[medium_size] = {0};
    Buffer r_medium(recv_medium, sizeof(recv_medium));
    TEST_ASSERT_EQ(t->recv(r_medium), OK);
    TEST_ASSERT_EQ(r_medium.size, medium_size);
    TEST_ASSERT_EQ(memcmp(recv_medium, medium_buf, medium_size), 0);
    
    // Receive and verify large
    uint8_t recv_large[large_size] = {0};
    Buffer r_large(recv_large, sizeof(recv_large));
    TEST_ASSERT_EQ(t->recv(r_large), OK);
    TEST_ASSERT_EQ(r_large.size, large_size);
    TEST_ASSERT_EQ(memcmp(recv_large, large_buf, large_size), 0);
    
    t->close();
}

TEST(udp_reopening_transport) {
    Transport* t = UDPTransport::get_instance();
    
    // Open, send, receive, close
    t->open();
    
    const char* msg1 = "First session";
    uint8_t buf1[64];
    memcpy(buf1, msg1, strlen(msg1));
    Buffer send1(buf1, strlen(msg1));
    TEST_ASSERT_EQ(t->send(send1), OK);
    
    uint8_t recv1[64] = {0};
    Buffer r1(recv1, sizeof(recv1));
    TEST_ASSERT_EQ(t->recv(r1), OK);
    
    t->close();
    
    // Reopen and do it again
    t->open();
    
    const char* msg2 = "Second session";
    uint8_t buf2[64];
    memcpy(buf2, msg2, strlen(msg2));
    Buffer send2(buf2, strlen(msg2));
    TEST_ASSERT_EQ(t->send(send2), OK);
    
    uint8_t recv2[64] = {0};
    Buffer r2(recv2, sizeof(recv2));
    TEST_ASSERT_EQ(t->recv(r2), OK);
    TEST_ASSERT_EQ(r2.size, strlen(msg2));
    TEST_ASSERT_EQ(memcmp(recv2, msg2, strlen(msg2)), 0);
    
    t->close();
}

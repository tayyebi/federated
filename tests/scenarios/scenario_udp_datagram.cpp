/**
 * @file scenario_udp_datagram.cpp
 * @brief User Scenario Test: UDP Datagram Communication
 * 
 * Tests a realistic UDP datagram communication scenario:
 * 1. Open UDP transport (bind to localhost)
 * 2. Send multiple datagrams of different sizes
 * 3. Receive datagrams and verify boundaries are preserved
 * 4. Demonstrate that each datagram is received as a complete unit
 * 
 * This demonstrates UDP transport's key feature: message boundary preservation,
 * unlike TCP which is a byte stream protocol.
 */

#include "../test_runner.h"
#include "../../include/federated/transport/udp.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>

using namespace federated;

TEST(scenario_udp_simple_datagram_exchange) {
    printf("\n  === User Scenario: Simple UDP Datagram Exchange ===\n");
    
    // Get UDP transport
    transport::Transport* udp = transport::UDPTransport::get_instance();
    
    // Open transport (bind to localhost)
    TEST_ASSERT_EQ(udp->open(), core::OK);
    printf("  1. UDP transport opened (bound to localhost)\n");
    
    // Prepare datagram
    const char* message = "UDP Datagram Message";
    uint8_t send_buf[128];
    memcpy(send_buf, message, strlen(message));
    core::Buffer send_data(send_buf, strlen(message));
    
    // Send datagram
    printf("  2. Sending datagram: \"%s\" (%zu bytes)\n", message, strlen(message));
    TEST_ASSERT_EQ(udp->send(send_data), core::OK);
    printf("  3. Datagram sent successfully ✓\n");
    
    // Receive datagram
    uint8_t recv_buf[128] = {0};
    core::Buffer recv_data(recv_buf, sizeof(recv_buf));
    
    TEST_ASSERT_EQ(udp->recv(recv_data), core::OK);
    printf("  4. Datagram received: %zu bytes\n", recv_data.size);
    
    // Verify received data
    TEST_ASSERT_EQ(recv_data.size, strlen(message));
    TEST_ASSERT_EQ(memcmp(recv_buf, message, strlen(message)), 0);
    printf("  5. Message verified: \"%.*s\" ✓\n", (int)recv_data.size, recv_buf);
    
    // Close transport
    udp->close();
    printf("  6. UDP transport closed\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_udp_message_boundaries) {
    printf("\n  === User Scenario: UDP Message Boundary Preservation ===\n");
    
    transport::Transport* udp = transport::UDPTransport::get_instance();
    TEST_ASSERT_EQ(udp->open(), core::OK);
    printf("  1. UDP transport opened\n");
    
    // Send three distinct datagrams
    const char* msg1 = "FIRST";
    const char* msg2 = "SECOND_MESSAGE";
    const char* msg3 = "THIRD_MESSAGE_IS_LONGER";
    
    printf("  2. Sending three datagrams:\n");
    
    uint8_t buf1[64];
    memcpy(buf1, msg1, strlen(msg1));
    core::Buffer dgram1(buf1, strlen(msg1));
    TEST_ASSERT_EQ(udp->send(dgram1), core::OK);
    printf("     a) \"%s\" (%zu bytes)\n", msg1, strlen(msg1));
    
    uint8_t buf2[64];
    memcpy(buf2, msg2, strlen(msg2));
    core::Buffer dgram2(buf2, strlen(msg2));
    TEST_ASSERT_EQ(udp->send(dgram2), core::OK);
    printf("     b) \"%s\" (%zu bytes)\n", msg2, strlen(msg2));
    
    uint8_t buf3[64];
    memcpy(buf3, msg3, strlen(msg3));
    core::Buffer dgram3(buf3, strlen(msg3));
    TEST_ASSERT_EQ(udp->send(dgram3), core::OK);
    printf("     c) \"%s\" (%zu bytes)\n", msg3, strlen(msg3));
    
    printf("  3. All datagrams sent ✓\n");
    
    // Receive datagrams - each should preserve original boundaries
    printf("  4. Receiving datagrams:\n");
    
    uint8_t recv1[64] = {0};
    core::Buffer r1(recv1, sizeof(recv1));
    TEST_ASSERT_EQ(udp->recv(r1), core::OK);
    TEST_ASSERT_EQ(r1.size, strlen(msg1));
    TEST_ASSERT_EQ(memcmp(recv1, msg1, strlen(msg1)), 0);
    printf("     a) Received: \"%.*s\" (%zu bytes) ✓\n", (int)r1.size, recv1, r1.size);
    
    uint8_t recv2[64] = {0};
    core::Buffer r2(recv2, sizeof(recv2));
    TEST_ASSERT_EQ(udp->recv(r2), core::OK);
    TEST_ASSERT_EQ(r2.size, strlen(msg2));
    TEST_ASSERT_EQ(memcmp(recv2, msg2, strlen(msg2)), 0);
    printf("     b) Received: \"%.*s\" (%zu bytes) ✓\n", (int)r2.size, recv2, r2.size);
    
    uint8_t recv3[64] = {0};
    core::Buffer r3(recv3, sizeof(recv3));
    TEST_ASSERT_EQ(udp->recv(r3), core::OK);
    TEST_ASSERT_EQ(r3.size, strlen(msg3));
    TEST_ASSERT_EQ(memcmp(recv3, msg3, strlen(msg3)), 0);
    printf("     c) Received: \"%.*s\" (%zu bytes) ✓\n", (int)r3.size, recv3, r3.size);
    
    printf("  5. All datagrams received with boundaries preserved ✓\n");
    printf("  6. Key observation: Each recv() returns exactly one complete datagram\n");
    
    udp->close();
    printf("  7. UDP transport closed\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_udp_variable_size_datagrams) {
    printf("\n  === User Scenario: UDP Variable Size Datagrams ===\n");
    
    transport::Transport* udp = transport::UDPTransport::get_instance();
    TEST_ASSERT_EQ(udp->open(), core::OK);
    printf("  1. UDP transport opened\n");
    
    // Send datagrams of various sizes
    printf("  2. Sending datagrams of different sizes:\n");
    
    // Tiny datagram (1 byte)
    uint8_t tiny_buf[1] = {0x42};
    core::Buffer tiny_dgram(tiny_buf, 1);
    TEST_ASSERT_EQ(udp->send(tiny_dgram), core::OK);
    printf("     a) 1 byte datagram sent\n");
    
    // Small datagram (16 bytes)
    uint8_t small_buf[16];
    for (int i = 0; i < 16; i++) small_buf[i] = (uint8_t)i;
    core::Buffer small_dgram(small_buf, 16);
    TEST_ASSERT_EQ(udp->send(small_dgram), core::OK);
    printf("     b) 16 byte datagram sent\n");
    
    // Medium datagram (256 bytes)
    uint8_t medium_buf[256];
    for (int i = 0; i < 256; i++) medium_buf[i] = (uint8_t)i;
    core::Buffer medium_dgram(medium_buf, 256);
    TEST_ASSERT_EQ(udp->send(medium_dgram), core::OK);
    printf("     c) 256 byte datagram sent\n");
    
    // Large datagram (1024 bytes)
    uint8_t large_buf[1024];
    for (int i = 0; i < 1024; i++) large_buf[i] = (uint8_t)(i % 256);
    core::Buffer large_dgram(large_buf, 1024);
    TEST_ASSERT_EQ(udp->send(large_dgram), core::OK);
    printf("     d) 1024 byte datagram sent\n");
    
    printf("  3. All datagrams sent ✓\n");
    
    // Receive each datagram with exact size preservation
    printf("  4. Receiving and verifying sizes:\n");
    
    uint8_t recv_tiny[1024];
    core::Buffer r_tiny(recv_tiny, sizeof(recv_tiny));
    TEST_ASSERT_EQ(udp->recv(r_tiny), core::OK);
    TEST_ASSERT_EQ(r_tiny.size, 1);
    TEST_ASSERT_EQ(recv_tiny[0], 0x42);
    printf("     a) Received 1 byte datagram ✓\n");
    
    uint8_t recv_small[1024];
    core::Buffer r_small(recv_small, sizeof(recv_small));
    TEST_ASSERT_EQ(udp->recv(r_small), core::OK);
    TEST_ASSERT_EQ(r_small.size, 16);
    printf("     b) Received 16 byte datagram ✓\n");
    
    uint8_t recv_medium[1024];
    core::Buffer r_medium(recv_medium, sizeof(recv_medium));
    TEST_ASSERT_EQ(udp->recv(r_medium), core::OK);
    TEST_ASSERT_EQ(r_medium.size, 256);
    printf("     c) Received 256 byte datagram ✓\n");
    
    uint8_t recv_large[1024];
    core::Buffer r_large(recv_large, sizeof(recv_large));
    TEST_ASSERT_EQ(udp->recv(r_large), core::OK);
    TEST_ASSERT_EQ(r_large.size, 1024);
    printf("     d) Received 1024 byte datagram ✓\n");
    
    printf("  5. All datagrams received with correct sizes ✓\n");
    printf("  6. UDP preserves exact datagram size regardless of variation\n");
    
    udp->close();
    printf("  7. UDP transport closed\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_udp_timeout_handling) {
    printf("\n  === User Scenario: UDP Timeout Handling ===\n");
    
    transport::Transport* udp = transport::UDPTransport::get_instance();
    TEST_ASSERT_EQ(udp->open(), core::OK);
    printf("  1. UDP transport opened\n");
    
    // Try to receive without sending anything
    printf("  2. Attempting to receive without sending (should timeout)...\n");
    
    uint8_t buf[128];
    core::Buffer recv_data(buf, sizeof(buf));
    
    core::ErrorCode err = udp->recv(recv_data);
    TEST_ASSERT_EQ(err, core::ERR_TIMEOUT);
    printf("  3. Receive timed out as expected ✓\n");
    
    // Now send a datagram
    const char* msg = "After timeout";
    uint8_t send_buf[64];
    memcpy(send_buf, msg, strlen(msg));
    core::Buffer send_data(send_buf, strlen(msg));
    
    printf("  4. Sending datagram: \"%s\"\n", msg);
    TEST_ASSERT_EQ(udp->send(send_data), core::OK);
    
    // Receive should work now
    printf("  5. Receiving datagram...\n");
    uint8_t recv_buf[64] = {0};
    core::Buffer recv_msg(recv_buf, sizeof(recv_buf));
    TEST_ASSERT_EQ(udp->recv(recv_msg), core::OK);
    TEST_ASSERT_EQ(recv_msg.size, strlen(msg));
    TEST_ASSERT_EQ(memcmp(recv_buf, msg, strlen(msg)), 0);
    printf("  6. Datagram received successfully: \"%.*s\" ✓\n", 
           (int)recv_msg.size, recv_buf);
    
    udp->close();
    printf("  7. Transport handles timeouts gracefully ✓\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_udp_vs_tcp_comparison) {
    printf("\n  === User Scenario: UDP vs TCP Comparison ===\n");
    
    transport::Transport* udp = transport::UDPTransport::get_instance();
    TEST_ASSERT_EQ(udp->open(), core::OK);
    printf("  1. UDP transport opened\n");
    
    printf("  2. Demonstrating UDP's message-oriented nature:\n");
    
    // Send two messages back-to-back
    const char* msg1 = "Message1";
    const char* msg2 = "Message2";
    
    uint8_t buf1[32];
    memcpy(buf1, msg1, strlen(msg1));
    core::Buffer dgram1(buf1, strlen(msg1));
    
    uint8_t buf2[32];
    memcpy(buf2, msg2, strlen(msg2));
    core::Buffer dgram2(buf2, strlen(msg2));
    
    TEST_ASSERT_EQ(udp->send(dgram1), core::OK);
    TEST_ASSERT_EQ(udp->send(dgram2), core::OK);
    printf("     - Sent two datagrams: \"%s\" and \"%s\"\n", msg1, msg2);
    
    // Receive them separately (UDP preserves boundaries)
    uint8_t recv1[32] = {0};
    core::Buffer r1(recv1, sizeof(recv1));
    TEST_ASSERT_EQ(udp->recv(r1), core::OK);
    
    uint8_t recv2[32] = {0};
    core::Buffer r2(recv2, sizeof(recv2));
    TEST_ASSERT_EQ(udp->recv(r2), core::OK);
    
    printf("     - Received as two separate datagrams ✓\n");
    printf("     - First:  \"%.*s\" (%zu bytes)\n", (int)r1.size, recv1, r1.size);
    printf("     - Second: \"%.*s\" (%zu bytes)\n", (int)r2.size, recv2, r2.size);
    
    // Verify they are not concatenated
    TEST_ASSERT_EQ(r1.size, strlen(msg1));
    TEST_ASSERT_EQ(r2.size, strlen(msg2));
    TEST_ASSERT_EQ(memcmp(recv1, msg1, strlen(msg1)), 0);
    TEST_ASSERT_EQ(memcmp(recv2, msg2, strlen(msg2)), 0);
    
    printf("  3. UDP Key Feature: Message boundaries are preserved\n");
    printf("     (Unlike TCP which would stream bytes together)\n");
    
    udp->close();
    printf("  4. UDP transport closed\n");
    printf("  === Scenario Complete ===\n\n");
}

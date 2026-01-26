/**
 * @file scenario_tcp_communication.cpp
 * @brief User Scenario Test: TCP Client-Server Communication
 * 
 * Tests a realistic TCP communication scenario:
 * 1. Open TCP transport (server listens on localhost)
 * 2. Client connects and sends a request
 * 3. Server receives request and sends response
 * 4. Client receives response
 * 5. Verify bidirectional communication
 * 
 * This demonstrates TCP transport working in a real client-server pattern.
 */

#include "../test_runner.h"
#include "../../include/federated/transport/tcp.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>

using namespace federated;

TEST(scenario_tcp_simple_exchange) {
    printf("\n  === User Scenario: Simple TCP Message Exchange ===\n");
    
    // Get TCP transport
    transport::Transport* tcp = transport::TCPTransport::get_instance();
    
    // Open transport (server starts listening on localhost)
    TEST_ASSERT_EQ(tcp->open(), core::OK);
    printf("  1. TCP transport opened (server listening on localhost)\n");
    
    // Prepare request message
    const char* request = "HELLO SERVER";
    uint8_t req_buf[128];
    memcpy(req_buf, request, strlen(request));
    core::Buffer req_data(req_buf, strlen(request));
    
    // Client sends request
    printf("  2. Client sending: \"%s\"\n", request);
    TEST_ASSERT_EQ(tcp->send(req_data), core::OK);
    printf("  3. Request sent successfully ✓\n");
    
    // Server receives request
    uint8_t recv_buf[128] = {0};
    core::Buffer recv_data(recv_buf, sizeof(recv_buf));
    
    TEST_ASSERT_EQ(tcp->recv(recv_data), core::OK);
    printf("  4. Server received %zu bytes\n", recv_data.size);
    
    // Verify received data
    TEST_ASSERT_EQ(recv_data.size, strlen(request));
    TEST_ASSERT_EQ(memcmp(recv_buf, request, strlen(request)), 0);
    printf("  5. Message verified: \"%.*s\" ✓\n", (int)recv_data.size, recv_buf);
    
    // Close transport
    tcp->close();
    printf("  6. TCP transport closed\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_tcp_bidirectional) {
    printf("\n  === User Scenario: TCP Bidirectional Communication ===\n");
    
    transport::Transport* tcp = transport::TCPTransport::get_instance();
    TEST_ASSERT_EQ(tcp->open(), core::OK);
    printf("  1. TCP server listening\n");
    
    // Client sends request
    const char* request = "REQUEST: Get status";
    uint8_t req_buf[128];
    memcpy(req_buf, request, strlen(request));
    core::Buffer req_msg(req_buf, strlen(request));
    
    printf("  2. Client → Server: \"%s\"\n", request);
    TEST_ASSERT_EQ(tcp->send(req_msg), core::OK);
    
    // Server receives request
    uint8_t recv_req_buf[128] = {0};
    core::Buffer recv_req(recv_req_buf, sizeof(recv_req_buf));
    TEST_ASSERT_EQ(tcp->recv(recv_req), core::OK);
    printf("  3. Server received: \"%.*s\"\n", (int)recv_req.size, recv_req_buf);
    
    // Server sends response
    const char* response = "RESPONSE: Status OK";
    uint8_t resp_buf[128];
    memcpy(resp_buf, response, strlen(response));
    core::Buffer resp_msg(resp_buf, strlen(response));
    
    printf("  4. Server → Client: \"%s\"\n", response);
    TEST_ASSERT_EQ(tcp->send(resp_msg), core::OK);
    
    // Client receives response
    uint8_t recv_resp_buf[128] = {0};
    core::Buffer recv_resp(recv_resp_buf, sizeof(recv_resp_buf));
    TEST_ASSERT_EQ(tcp->recv(recv_resp), core::OK);
    printf("  5. Client received: \"%.*s\"\n", (int)recv_resp.size, recv_resp_buf);
    
    // Verify both messages
    TEST_ASSERT_EQ(memcmp(recv_req_buf, request, strlen(request)), 0);
    TEST_ASSERT_EQ(memcmp(recv_resp_buf, response, strlen(response)), 0);
    printf("  6. Bidirectional communication verified ✓\n");
    
    tcp->close();
    printf("  7. Connection closed\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_tcp_multiple_exchanges) {
    printf("\n  === User Scenario: TCP Multiple Message Exchanges ===\n");
    
    transport::Transport* tcp = transport::TCPTransport::get_instance();
    TEST_ASSERT_EQ(tcp->open(), core::OK);
    printf("  1. TCP connection established\n");
    
    // Multiple rounds of communication
    const char* messages[] = {
        "Message 1",
        "Message 2",
        "Message 3"
    };
    
    for (int i = 0; i < 3; i++) {
        // Send message
        uint8_t send_buf[64];
        memcpy(send_buf, messages[i], strlen(messages[i]));
        core::Buffer send_msg(send_buf, strlen(messages[i]));
        
        TEST_ASSERT_EQ(tcp->send(send_msg), core::OK);
        printf("  %d. Sent: \"%s\"\n", i * 2 + 2, messages[i]);
        
        // Receive message
        uint8_t recv_buf[64] = {0};
        core::Buffer recv_msg(recv_buf, sizeof(recv_buf));
        
        TEST_ASSERT_EQ(tcp->recv(recv_msg), core::OK);
        printf("  %d. Received: \"%.*s\" ✓\n", i * 2 + 3, (int)recv_msg.size, recv_buf);
    }
    
    tcp->close();
    printf("  8. All messages exchanged successfully\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_tcp_connection_lifecycle) {
    printf("\n  === User Scenario: TCP Connection Lifecycle ===\n");
    
    transport::Transport* tcp = transport::TCPTransport::get_instance();
    
    // Test connection lifecycle
    printf("  1. Opening TCP transport...\n");
    TEST_ASSERT_EQ(tcp->open(), core::OK);
    TEST_ASSERT(tcp->available());
    printf("  2. Transport is available ✓\n");
    
    // Send some data
    const char* data = "Lifecycle test";
    uint8_t buf[64];
    memcpy(buf, data, strlen(data));
    core::Buffer msg(buf, strlen(data));
    
    printf("  3. Sending data...\n");
    TEST_ASSERT_EQ(tcp->send(msg), core::OK);
    printf("  4. Data sent successfully ✓\n");
    
    // Receive data
    uint8_t recv_buf[64];
    core::Buffer recv_msg(recv_buf, sizeof(recv_buf));
    printf("  5. Receiving data...\n");
    TEST_ASSERT_EQ(tcp->recv(recv_msg), core::OK);
    printf("  6. Data received successfully ✓\n");
    
    // Close connection
    printf("  7. Closing TCP transport...\n");
    TEST_ASSERT_EQ(tcp->close(), core::OK);
    printf("  8. Transport closed ✓\n");
    
    // Verify not available after close
    TEST_ASSERT(!tcp->available());
    printf("  9. Transport no longer available ✓\n");
    
    printf("  === Scenario Complete ===\n\n");
}

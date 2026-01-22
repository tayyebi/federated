/**
 * @file scenario_file_transfer.cpp
 * @brief User Scenario Test: File Transfer with Framing
 * 
 * Tests a realistic file transfer scenario using the Federated stack:
 * 1. Create a file with known content
 * 2. Frame the content (length-prefix)
 * 3. Send via transport (loopback)
 * 4. Receive and decode
 * 5. Verify file integrity
 * 
 * This tests the integration of Transport + Framer layers
 * in a real-world use case.
 */

#include "../test_runner.h"
#include "../../include/federated/transport/loopback.h"
#include "../../include/federated/framer/length_prefix.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>

using namespace federated;

TEST(scenario_simple_file_transfer) {
    printf("\n  === User Scenario: Simple File Transfer ===\n");
    
    // Simulate file content
    const char* file_content = "This is a test file with some important data.\n"
                               "It has multiple lines.\n"
                               "We want to send it reliably.";
    
    // Get components
    transport::Transport* transport = transport::LoopbackTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    
    // Setup
    TEST_ASSERT_EQ(transport->open(), core::OK);
    printf("  1. Transport opened\n");
    
    // Sender side: Frame the file content
    uint8_t file_buf[512];
    memcpy(file_buf, file_content, strlen(file_content));
    core::Buffer file_data(file_buf, strlen(file_content));
    
    uint8_t framed_buf[512];
    core::Buffer framed(framed_buf, sizeof(framed_buf));
    
    TEST_ASSERT_EQ(framer->encode(file_data, framed), core::OK);
    printf("  2. File content framed (%zu bytes -> %zu bytes)\n", file_data.size, framed.size);
    
    // Send over transport
    TEST_ASSERT_EQ(transport->send(framed), core::OK);
    printf("  3. Framed data sent via transport\n");
    
    // Receiver side: Receive and decode
    uint8_t received_buf[512];
    core::Buffer received(received_buf, sizeof(received_buf));
    
    TEST_ASSERT_EQ(transport->recv(received), core::OK);
    printf("  4. Data received from transport (%zu bytes)\n", received.size);
    
    uint8_t decoded_buf[512];
    core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
    
    TEST_ASSERT_EQ(framer->decode(received, decoded), core::OK);
    printf("  5. Framed data decoded (%zu bytes)\n", decoded.size);
    
    // Verify file integrity
    TEST_ASSERT_EQ(decoded.size, strlen(file_content));
    TEST_ASSERT_EQ(memcmp(decoded.data, file_content, decoded.size), 0);
    printf("  6. File integrity verified ✓\n");
    
    // Cleanup
    transport->close();
    printf("  7. Transport closed\n");
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_multiple_file_chunks) {
    printf("\n  === User Scenario: Multiple File Chunks ===\n");
    
    // Simulate sending a file in multiple chunks
    const char* chunk1 = "First chunk of data";
    const char* chunk2 = "Second chunk of data";
    const char* chunk3 = "Third and final chunk";
    
    transport::Transport* transport = transport::LoopbackTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    
    TEST_ASSERT_EQ(transport->open(), core::OK);
    
    // Send all chunks
    const char* chunks[] = {chunk1, chunk2, chunk3};
    int chunk_count = 3;
    
    for (int i = 0; i < chunk_count; i++) {
        uint8_t chunk_buf[128];
        memcpy(chunk_buf, chunks[i], strlen(chunks[i]));
        core::Buffer chunk_data(chunk_buf, strlen(chunks[i]));
        
        uint8_t framed_buf[256];
        core::Buffer framed(framed_buf, sizeof(framed_buf));
        
        TEST_ASSERT_EQ(framer->encode(chunk_data, framed), core::OK);
        TEST_ASSERT_EQ(transport->send(framed), core::OK);
        printf("  Sent chunk %d (%zu bytes)\n", i + 1, chunk_data.size);
    }
    
    // Receive all chunks
    for (int i = 0; i < chunk_count; i++) {
        uint8_t received_buf[256];
        core::Buffer received(received_buf, sizeof(received_buf));
        
        TEST_ASSERT_EQ(transport->recv(received), core::OK);
        
        uint8_t decoded_buf[128];
        core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
        
        TEST_ASSERT_EQ(framer->decode(received, decoded), core::OK);
        
        // Verify chunk
        TEST_ASSERT_EQ(decoded.size, strlen(chunks[i]));
        TEST_ASSERT_EQ(memcmp(decoded.data, chunks[i], decoded.size), 0);
        printf("  Received and verified chunk %d ✓\n", i + 1);
    }
    
    transport->close();
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_bidirectional_communication) {
    printf("\n  === User Scenario: Bidirectional Communication ===\n");
    
    // Simulate a request-response pattern
    const char* request = "REQUEST: Get status";
    const char* response = "RESPONSE: Status OK";
    
    transport::Transport* transport = transport::LoopbackTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    
    TEST_ASSERT_EQ(transport->open(), core::OK);
    
    // Send request
    uint8_t req_buf[128];
    memcpy(req_buf, request, strlen(request));
    core::Buffer req_data(req_buf, strlen(request));
    
    uint8_t req_framed_buf[256];
    core::Buffer req_framed(req_framed_buf, sizeof(req_framed_buf));
    
    TEST_ASSERT_EQ(framer->encode(req_data, req_framed), core::OK);
    TEST_ASSERT_EQ(transport->send(req_framed), core::OK);
    printf("  Request sent: \"%s\"\n", request);
    
    // Receive request (simulate server side)
    uint8_t req_recv_buf[256];
    core::Buffer req_recv(req_recv_buf, sizeof(req_recv_buf));
    TEST_ASSERT_EQ(transport->recv(req_recv), core::OK);
    
    uint8_t req_decoded_buf[128];
    core::Buffer req_decoded(req_decoded_buf, sizeof(req_decoded_buf));
    TEST_ASSERT_EQ(framer->decode(req_recv, req_decoded), core::OK);
    
    // Verify request
    TEST_ASSERT_EQ(memcmp(req_decoded.data, request, strlen(request)), 0);
    printf("  Request received and verified ✓\n");
    
    // Send response
    uint8_t resp_buf[128];
    memcpy(resp_buf, response, strlen(response));
    core::Buffer resp_data(resp_buf, strlen(response));
    
    uint8_t resp_framed_buf[256];
    core::Buffer resp_framed(resp_framed_buf, sizeof(resp_framed_buf));
    
    TEST_ASSERT_EQ(framer->encode(resp_data, resp_framed), core::OK);
    TEST_ASSERT_EQ(transport->send(resp_framed), core::OK);
    printf("  Response sent: \"%s\"\n", response);
    
    // Receive response (client side)
    uint8_t resp_recv_buf[256];
    core::Buffer resp_recv(resp_recv_buf, sizeof(resp_recv_buf));
    TEST_ASSERT_EQ(transport->recv(resp_recv), core::OK);
    
    uint8_t resp_decoded_buf[128];
    core::Buffer resp_decoded(resp_decoded_buf, sizeof(resp_decoded_buf));
    TEST_ASSERT_EQ(framer->decode(resp_recv, resp_decoded), core::OK);
    
    // Verify response
    TEST_ASSERT_EQ(memcmp(resp_decoded.data, response, strlen(response)), 0);
    printf("  Response received and verified ✓\n");
    
    transport->close();
    printf("  === Scenario Complete ===\n\n");
}

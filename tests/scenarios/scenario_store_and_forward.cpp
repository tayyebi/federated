/**
 * @file scenario_store_and_forward.cpp
 * @brief User Scenario Test: Store-and-Forward Messaging
 * 
 * Tests offline messaging capability using file transport:
 * 1. Write message to file (sender offline)
 * 2. Close connection
 * 3. Open connection later (receiver comes online)
 * 4. Read message from file
 * 5. Verify message integrity
 * 
 * This tests the file transport's store-and-forward capability
 * for asynchronous communication.
 */

#include "../test_runner.h"
#include "../../include/federated/transport/file.h"
#include "../../include/federated/framer/length_prefix.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>

using namespace federated;

TEST(scenario_store_and_forward_basic) {
    printf("\n  === User Scenario: Store-and-Forward Messaging ===\n");
    
    const char* message = "Offline message: Please review the attached document.";
    const char* write_path = "/tmp/federated_outbox.msg";
    const char* read_path = "/tmp/federated_inbox.msg";
    
    transport::Transport* file_transport = transport::FileTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    
    // Phase 1: Sender writes message (offline mode)
    printf("  === Phase 1: Sender (Offline Mode) ===\n");
    
    transport::FileTransport::set_write_path(write_path);
    transport::FileTransport::set_read_path(read_path);  // Not used in this phase
    
    TEST_ASSERT_EQ(file_transport->open(), core::OK);
    printf("  1. File transport opened\n");
    
    // Create and frame message
    uint8_t msg_buf[256];
    memcpy(msg_buf, message, strlen(message));
    core::Buffer msg(msg_buf, strlen(message));
    
    uint8_t framed_buf[512];
    core::Buffer framed(framed_buf, sizeof(framed_buf));
    
    TEST_ASSERT_EQ(framer->encode(msg, framed), core::OK);
    printf("  2. Message framed (%zu bytes)\n", framed.size);
    
    // Write to file
    TEST_ASSERT_EQ(file_transport->send(framed), core::OK);
    printf("  3. Message written to file: %s ✓\n", write_path);
    
    file_transport->close();
    printf("  4. Connection closed (sender goes offline)\n");
    
    // Phase 2: Receiver reads message (comes online later)
    printf("  === Phase 2: Receiver (Comes Online) ===\n");
    
    // In a real scenario, the file would be transferred to the receiver's machine
    // For this test, we simulate it by swapping read/write paths
    transport::FileTransport::set_read_path(write_path);  // Read from sender's outbox
    transport::FileTransport::set_write_path("/tmp/federated_dummy.msg");  // Not used
    
    TEST_ASSERT_EQ(file_transport->open(), core::OK);
    printf("  5. Receiver opens connection\n");
    
    // Read from file
    uint8_t received_buf[512];
    core::Buffer received(received_buf, sizeof(received_buf));
    
    TEST_ASSERT_EQ(file_transport->recv(received), core::OK);
    printf("  6. Message read from file (%zu bytes)\n", received.size);
    
    // Decode message
    uint8_t decoded_buf[256];
    core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
    
    TEST_ASSERT_EQ(framer->decode(received, decoded), core::OK);
    printf("  7. Message decoded (%zu bytes)\n", decoded.size);
    
    // Verify message
    TEST_ASSERT_EQ(decoded.size, strlen(message));
    TEST_ASSERT_EQ(memcmp(decoded.data, message, decoded.size), 0);
    
    decoded_buf[decoded.size] = '\0';
    printf("  8. Message received: \"%s\"\n", (char*)decoded_buf);
    printf("  9. Message integrity verified ✓\n");
    
    file_transport->close();
    
    // Cleanup
    remove(write_path);
    remove("/tmp/federated_dummy.msg");
    
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_message_queue) {
    printf("\n  === User Scenario: Message Queue via File ===\n");
    
    const char* messages[] = {
        "Queued message 1",
        "Queued message 2",
        "Queued message 3"
    };
    int message_count = 3;
    
    const char* queue_file = "/tmp/federated_queue_%d.msg";
    
    transport::Transport* file_transport = transport::FileTransport::get_instance();
    framer::Framer* framer = framer::LengthPrefixFramer::get_instance();
    
    // Send multiple messages to separate files (message queue)
    printf("  === Queuing Messages ===\n");
    
    for (int i = 0; i < message_count; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), queue_file, i);
        
        transport::FileTransport::set_write_path(filepath);
        transport::FileTransport::set_read_path("/tmp/dummy");
        
        TEST_ASSERT_EQ(file_transport->open(), core::OK);
        
        // Frame and send
        uint8_t msg_buf[128];
        memcpy(msg_buf, messages[i], strlen(messages[i]));
        core::Buffer msg(msg_buf, strlen(messages[i]));
        
        uint8_t framed_buf[256];
        core::Buffer framed(framed_buf, sizeof(framed_buf));
        
        TEST_ASSERT_EQ(framer->encode(msg, framed), core::OK);
        TEST_ASSERT_EQ(file_transport->send(framed), core::OK);
        
        file_transport->close();
        printf("  Queued message %d: \"%s\"\n", i + 1, messages[i]);
    }
    
    // Receive messages from queue
    printf("  === Processing Queue ===\n");
    
    for (int i = 0; i < message_count; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), queue_file, i);
        
        transport::FileTransport::set_read_path(filepath);
        transport::FileTransport::set_write_path("/tmp/dummy");
        
        TEST_ASSERT_EQ(file_transport->open(), core::OK);
        
        // Receive and decode
        uint8_t received_buf[256];
        core::Buffer received(received_buf, sizeof(received_buf));
        
        TEST_ASSERT_EQ(file_transport->recv(received), core::OK);
        
        uint8_t decoded_buf[128];
        core::Buffer decoded(decoded_buf, sizeof(decoded_buf));
        
        TEST_ASSERT_EQ(framer->decode(received, decoded), core::OK);
        
        // Verify
        TEST_ASSERT_EQ(decoded.size, strlen(messages[i]));
        TEST_ASSERT_EQ(memcmp(decoded.data, messages[i], decoded.size), 0);
        
        file_transport->close();
        
        // Delete processed message
        remove(filepath);
        printf("  Processed message %d ✓\n", i + 1);
    }
    
    printf("  === Scenario Complete ===\n\n");
}

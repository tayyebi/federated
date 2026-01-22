#include "../test_runner.h"
#include "../../include/federated/transport/file.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using namespace federated::transport;
using namespace federated::core;

TEST(file_get_instance) {
    Transport* t = FileTransport::get_instance();
    TEST_ASSERT(t != nullptr);
    TEST_ASSERT(t->name != nullptr);
    TEST_ASSERT_STR_EQ(t->name, "file");
}

TEST(file_send_and_receive) {
    Transport* t = FileTransport::get_instance();
    
    // Create temp files
    char write_path[] = "/tmp/fed_write_XXXXXX";
    char read_path[] = "/tmp/fed_read_XXXXXX";
    
    int write_fd = mkstemp(write_path);
    int read_fd = mkstemp(read_path);
    TEST_ASSERT(write_fd >= 0);
    TEST_ASSERT(read_fd >= 0);
    close(write_fd);
    close(read_fd);
    
    // Configure transport
    FileTransport::set_write_path(write_path);
    FileTransport::set_read_path(read_path);
    
    TEST_ASSERT_EQ(t->open(), OK);
    
    // Write data to the "read" file (what we'll receive from)
    const char* msg = "File transport test";
    FILE* f = fopen(read_path, "w");
    TEST_ASSERT(f != nullptr);
    fwrite(msg, 1, strlen(msg), f);
    fclose(f);
    
    // Receive data
    uint8_t recv_buf[128] = {0};
    Buffer recv_buffer(recv_buf, sizeof(recv_buf));
    ErrorCode err = t->recv(recv_buffer);
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(recv_buffer.size > 0);
    TEST_ASSERT_EQ(memcmp(recv_buf, msg, strlen(msg)), 0);
    
    // Send data
    const char* send_msg = "Sending via file";
    uint8_t send_buf[128];
    memcpy(send_buf, send_msg, strlen(send_msg));
    Buffer send_buffer(send_buf, strlen(send_msg));
    err = t->send(send_buffer);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify file was written
    f = fopen(write_path, "r");
    TEST_ASSERT(f != nullptr);
    char verify_buf[128] = {0};
    size_t n = fread(verify_buf, 1, sizeof(verify_buf), f);
    fclose(f);
    TEST_ASSERT(n == strlen(send_msg));
    TEST_ASSERT_EQ(memcmp(verify_buf, send_msg, strlen(send_msg)), 0);
    
    t->close();
    
    // Cleanup
    unlink(write_path);
    unlink(read_path);
}

TEST(file_available) {
    Transport* t = FileTransport::get_instance();
    
    char write_path[] = "/tmp/fed_write2_XXXXXX";
    char read_path[] = "/tmp/fed_read2_XXXXXX";
    
    int write_fd = mkstemp(write_path);
    int read_fd = mkstemp(read_path);
    close(write_fd);
    close(read_fd);
    
    FileTransport::set_write_path(write_path);
    FileTransport::set_read_path(read_path);
    
    t->open();
    bool avail = t->available();
    TEST_ASSERT(avail);
    t->close();
    
    unlink(write_path);
    unlink(read_path);
}

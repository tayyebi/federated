#include "../test_runner.h"
#include "../../include/federated/core/registry.h"
#include "../../include/federated/transport/transport.h"

using namespace federated::core;
using namespace federated::transport;

// Mock transport for testing
static ErrorCode mock_open() { return OK; }
static ErrorCode mock_close() { return OK; }
static ErrorCode mock_send(const Buffer&) { return OK; }
static ErrorCode mock_recv(Buffer&) { return OK; }
static bool mock_available() { return true; }

static Transport mock_transport = {
    "mock",
    mock_open,
    mock_close,
    mock_send,
    mock_recv,
    mock_available
};

TEST(registry_initialization) {
    Registry reg;
    TEST_ASSERT_EQ(reg.transport_count, 0);
    TEST_ASSERT_EQ(reg.framer_count, 0);
    TEST_ASSERT_EQ(reg.crypto_count, 0);
    TEST_ASSERT_EQ(reg.service_count, 0);
    TEST_ASSERT_EQ(reg.tool_count, 0);
}

TEST(registry_register_transport) {
    Registry reg;
    bool result = reg.register_transport(&mock_transport);
    TEST_ASSERT(result);
    TEST_ASSERT_EQ(reg.transport_count, 1);
    TEST_ASSERT(reg.transports[0] == &mock_transport);
}

TEST(registry_find_transport) {
    Registry reg;
    reg.register_transport(&mock_transport);
    
    Transport* found = reg.find_transport("mock");
    TEST_ASSERT(found != nullptr);
    TEST_ASSERT(found == &mock_transport);
    
    Transport* not_found = reg.find_transport("nonexistent");
    TEST_ASSERT(not_found == nullptr);
}

#include "../../include/federated/transport/loopback.h"
#include <cstring>
#include <queue>

namespace federated {
namespace transport {

// Simple in-memory queue for loopback
constexpr size_t MAX_MESSAGE_SIZE = 4096;

struct Message {
    uint8_t data[MAX_MESSAGE_SIZE];
    size_t size;
};

static std::queue<Message> g_loopback_queue;
static bool g_loopback_open = false;

// Implementation functions
static core::ErrorCode loopback_open() {
    g_loopback_open = true;
    // Clear any existing messages
    while (!g_loopback_queue.empty()) {
        g_loopback_queue.pop();
    }
    return core::OK;
}

static core::ErrorCode loopback_close() {
    g_loopback_open = false;
    // Clear queue on close
    while (!g_loopback_queue.empty()) {
        g_loopback_queue.pop();
    }
    return core::OK;
}

static core::ErrorCode loopback_send(const core::Buffer& data) {
    if (!g_loopback_open) {
        return core::ERR_IO;
    }
    
    if (data.size > MAX_MESSAGE_SIZE) {
        return core::ERR_FORMAT;
    }
    
    Message msg;
    memcpy(msg.data, data.data, data.size);
    msg.size = data.size;
    
    g_loopback_queue.push(msg);
    
    return core::OK;
}

static core::ErrorCode loopback_recv(core::Buffer& data) {
    if (!g_loopback_open) {
        return core::ERR_IO;
    }
    
    if (g_loopback_queue.empty()) {
        return core::ERR_TIMEOUT;
    }
    
    Message msg = g_loopback_queue.front();
    g_loopback_queue.pop();
    
    if (msg.size > data.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(data.data, msg.data, msg.size);
    data.size = msg.size;
    
    return core::OK;
}

static bool loopback_available() {
    return g_loopback_open;
}

// Static transport instance
static Transport g_loopback_transport = {
    "loopback",
    loopback_open,
    loopback_close,
    loopback_send,
    loopback_recv,
    loopback_available
};

Transport* LoopbackTransport::get_instance() {
    return &g_loopback_transport;
}

} // namespace transport
} // namespace federated

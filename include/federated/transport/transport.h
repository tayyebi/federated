#ifndef FEDERATED_TRANSPORT_TRANSPORT_H
#define FEDERATED_TRANSPORT_TRANSPORT_H

#include "../core/buffer.h"
#include "../core/error.h"

namespace federated {
namespace transport {

/**
 * Transport - Generic transport interface
 * 
 * All transports must be independently testable and support failure simulation.
 * Hot-pluggable design.
 */
struct Transport {
    const char* name;
    
    // Lifecycle
    core::ErrorCode (*open)();
    core::ErrorCode (*close)();
    
    // I/O
    core::ErrorCode (*send)(const core::Buffer& data);
    core::ErrorCode (*recv)(core::Buffer& data);
    
    // Status
    bool (*available)();
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_TRANSPORT_H

#ifndef FEDERATED_TRANSPORT_LOOPBACK_H
#define FEDERATED_TRANSPORT_LOOPBACK_H

#include "transport.h"

namespace federated {
namespace transport {

/**
 * Loopback transport - in-memory transport for testing
 * 
 * Data sent is immediately available to receive.
 * No actual network I/O.
 */
struct LoopbackTransport {
    static Transport* get_instance();
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_LOOPBACK_H

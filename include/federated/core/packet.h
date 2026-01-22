#ifndef FEDERATED_CORE_PACKET_H
#define FEDERATED_CORE_PACKET_H

#include "buffer.h"

namespace federated {
namespace core {

/**
 * Packet - A network packet containing a payload
 */
struct Packet {
    Buffer payload;
    
    Packet() : payload() {}
    explicit Packet(const Buffer& buf) : payload(buf) {}
};

} // namespace core
} // namespace federated

#endif // FEDERATED_CORE_PACKET_H

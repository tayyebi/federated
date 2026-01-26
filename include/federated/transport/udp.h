#ifndef FEDERATED_TRANSPORT_UDP_H
#define FEDERATED_TRANSPORT_UDP_H

#include "transport.h"

namespace federated {
namespace transport {

/**
 * UDP transport - RFC 768 User Datagram Protocol
 * 
 * Implements unreliable, connectionless datagram communication over UDP/IP.
 * Preserves message boundaries unlike TCP's byte stream model.
 * 
 * Features:
 * - Cross-platform (POSIX sockets / Windows Winsock2)
 * - Connectionless (no explicit connect/accept handshake)
 * - Message boundaries preserved (each send/recv is one datagram)
 * - Non-blocking operations with timeouts
 * - Suitable for low-latency, loss-tolerant applications
 * 
 * Usage:
 * - Call open() to initialize (bind to localhost for server)
 * - Use send()/recv() for datagram exchange
 * - Each send() transmits one complete datagram
 * - Each recv() receives one complete datagram
 * - Call close() to cleanup
 * 
 * RFC 768: https://datatracker.ietf.org/doc/html/rfc768
 */
struct UDPTransport {
    static Transport* get_instance();
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_UDP_H

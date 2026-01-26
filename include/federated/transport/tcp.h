#ifndef FEDERATED_TRANSPORT_TCP_H
#define FEDERATED_TRANSPORT_TCP_H

#include "transport.h"

namespace federated {
namespace transport {

/**
 * TCP transport - RFC 793 Transmission Control Protocol
 * 
 * Implements reliable, connection-oriented byte stream communication over TCP/IP.
 * Supports both client and server modes using sockets.
 * 
 * Features:
 * - Cross-platform (POSIX sockets / Windows Winsock2)
 * - Connection-oriented (explicit connect/accept)
 * - Reliable delivery with flow control
 * - Non-blocking operations with timeouts
 * 
 * Usage:
 * - Call open() to initialize (server: bind/listen, client: ready to connect)
 * - Use send()/recv() for data exchange
 * - Call close() to cleanup
 * 
 * RFC 793: https://datatracker.ietf.org/doc/html/rfc793
 */
struct TCPTransport {
    static Transport* get_instance();
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_TCP_H

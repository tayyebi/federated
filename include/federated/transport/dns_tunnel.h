#ifndef FEDERATED_TRANSPORT_DNS_TUNNEL_H
#define FEDERATED_TRANSPORT_DNS_TUNNEL_H

#include "transport.h"

namespace federated {
namespace transport {

/**
 * DNS Tunnel Transport (Covert Channel)
 * 
 * RFC 1035 - Domain Names - Implementation and Specification
 * https://datatracker.ietf.org/doc/html/rfc1035
 * 
 * RFC 4648 - The Base16, Base32, and Base64 Data Encodings
 * https://datatracker.ietf.org/doc/html/rfc4648
 * 
 * Encodes data in DNS queries/responses for covert communication.
 * Useful in heavily firewalled/censored networks where only DNS is allowed.
 * 
 * Technique:
 * - Encode data in subdomain labels (base32 encoding per RFC 4648)
 * - Use TXT records for responses
 * - Limited bandwidth (~100-200 bytes per query)
 * - High latency but bypasses most firewalls
 * 
 * Implementation:
 * - Base32 encoding for DNS-safe characters
 * - DNS query construction following RFC 1035
 * - UDP socket for DNS queries (port 53)
 * - Configurable DNS server (defaults to 8.8.8.8)
 * - Rate limiting with configurable QPS
 * - Query timing jitter for anti-detection
 * 
 * Current Status: ENHANCED IMPLEMENTATION
 * - Functional send path with base32 encoding
 * - Basic DNS query construction
 * - Rate limiting with anti-detection jitter
 * - Configurable parameters
 * - Receive path simplified (full TXT parsing can be added)
 * - Works with standard DNS servers
 * 
 * Anti-Detection Features:
 * - Enforces maximum queries per second (configurable)
 * - Adds random jitter to query timing
 * - Prevents burst query patterns
 */

/**
 * Configuration for DNS Tunnel Transport
 * 
 * Allows customization of DNS server, rate limiting, and anti-detection features.
 */
struct DNSTunnelConfig {
    const char* dns_server;      // DNS server address (default: "8.8.8.8")
    uint16_t dns_port;           // DNS server port (default: 53)
    const char* base_domain;     // Base domain for tunneling (default: "tunnel.local")
    uint32_t max_qps;            // Max queries per second (default: 5)
    uint32_t min_jitter_ms;      // Min jitter in milliseconds (default: 100)
    uint32_t max_jitter_ms;      // Max jitter in milliseconds (default: 500)
    
    DNSTunnelConfig() 
        : dns_server("8.8.8.8")
        , dns_port(53)
        , base_domain("tunnel.local")
        , max_qps(5)
        , min_jitter_ms(100)
        , max_jitter_ms(500)
    {}
};

struct DNSTunnelTransport {
    static Transport* get_instance();
    
    // Configure DNS tunnel parameters (must be called before open())
    static void configure(const DNSTunnelConfig& config);
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_DNS_TUNNEL_H

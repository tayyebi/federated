#include "../../include/federated/transport/dns_tunnel.h"
#include "../../include/federated/core/error.h"
#include "../../include/federated/transport/udp.h"
#include <cstring>
#include <cstdio>

// Platform-specific socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

namespace federated {
namespace transport {

// DNS Tunnel configuration
constexpr const char* DNS_SERVER = "8.8.8.8";  // Google Public DNS
constexpr int DNS_PORT = 53;
constexpr const char* TUNNEL_DOMAIN = "tunnel.local";
constexpr size_t MAX_LABEL_LENGTH = 63;
constexpr size_t MAX_DOMAIN_LENGTH = 253;
constexpr size_t DNS_HEADER_SIZE = 12;
constexpr size_t MAX_DNS_MESSAGE = 512;

// DNS message state
struct DNSTunnelState {
    SOCKET socket_fd;
    struct sockaddr_in dns_server_addr;
    bool is_open;
    uint16_t transaction_id;
    
    DNSTunnelState() : socket_fd(INVALID_SOCKET), is_open(false), transaction_id(1) {
        memset(&dns_server_addr, 0, sizeof(dns_server_addr));
    }
};

static DNSTunnelState g_dns_tunnel_state;

// Base32 encoding table (RFC 4648)
static const char base32_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

// Encode data to base32
static size_t base32_encode(const uint8_t* input, size_t input_len, char* output, size_t output_size) {
    if (input_len == 0) return 0;
    
    size_t output_len = ((input_len * 8 + 4) / 5);  // Calculate output length
    if (output_len >= output_size) return 0;  // Not enough space
    
    size_t out_idx = 0;
    uint32_t buffer = 0;
    int bits = 0;
    
    for (size_t i = 0; i < input_len; i++) {
        buffer = (buffer << 8) | input[i];
        bits += 8;
        
        while (bits >= 5) {
            bits -= 5;
            output[out_idx++] = base32_alphabet[(buffer >> bits) & 0x1F];
        }
    }
    
    if (bits > 0) {
        buffer <<= (5 - bits);
        output[out_idx++] = base32_alphabet[buffer & 0x1F];
    }
    
    output[out_idx] = '\0';
    return out_idx;
}

// Decode base32 to data (for future receive path implementation)
static size_t base32_decode(const char* input, size_t input_len, uint8_t* output, size_t output_size) __attribute__((unused));
static size_t base32_decode(const char* input, size_t input_len, uint8_t* output, size_t output_size) {
    if (input_len == 0) return 0;
    
    size_t out_idx = 0;
    uint32_t buffer = 0;
    int bits = 0;
    
    for (size_t i = 0; i < input_len; i++) {
        char c = input[i];
        int val = -1;
        
        if (c >= 'A' && c <= 'Z') {
            val = c - 'A';
        } else if (c >= '2' && c <= '7') {
            val = 26 + (c - '2');
        } else if (c >= 'a' && c <= 'z') {
            val = c - 'a';  // Case insensitive
        }
        
        if (val == -1) continue;  // Skip invalid characters
        
        buffer = (buffer << 5) | val;
        bits += 5;
        
        if (bits >= 8) {
            bits -= 8;
            if (out_idx < output_size) {
                output[out_idx++] = (buffer >> bits) & 0xFF;
            }
        }
    }
    
    return out_idx;
}

// Build DNS query message
static size_t build_dns_query(const char* domain, uint16_t transaction_id, uint8_t* buffer, size_t buffer_size) {
    if (buffer_size < DNS_HEADER_SIZE + strlen(domain) + 6) return 0;
    
    size_t offset = 0;
    
    // DNS Header
    buffer[offset++] = (transaction_id >> 8) & 0xFF;  // Transaction ID (high byte)
    buffer[offset++] = transaction_id & 0xFF;          // Transaction ID (low byte)
    buffer[offset++] = 0x01;  // Flags: Standard query, recursion desired
    buffer[offset++] = 0x00;  // Flags
    buffer[offset++] = 0x00;  // Question count (high byte)
    buffer[offset++] = 0x01;  // Question count (low byte) = 1
    buffer[offset++] = 0x00;  // Answer count
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;  // Authority count
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;  // Additional count
    buffer[offset++] = 0x00;
    
    // Question section - encode domain name
    const char* label_start = domain;
    const char* p = domain;
    
    while (*p) {
        if (*p == '.' || *(p + 1) == '\0') {
            // End of label
            size_t label_len = p - label_start;
            if (*(p + 1) == '\0' && *p != '.') {
                label_len++;  // Include last character if not a dot
            }
            
            if (label_len > MAX_LABEL_LENGTH) return 0;
            
            buffer[offset++] = static_cast<uint8_t>(label_len);
            memcpy(buffer + offset, label_start, label_len);
            offset += label_len;
            
            if (*p == '.') {
                label_start = p + 1;
            }
        }
        p++;
    }
    
    buffer[offset++] = 0x00;  // End of domain name
    
    // Query type: TXT (16)
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x10;
    
    // Query class: IN (1)
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x01;
    
    return offset;
}

// DNS Tunnel implementation
static core::ErrorCode dns_tunnel_open() {
    if (g_dns_tunnel_state.is_open) {
        return core::OK;
    }
    
    // Platform-specific initialization
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return core::ERR_IO;
    }
#endif
    
    // Create UDP socket
    g_dns_tunnel_state.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_dns_tunnel_state.socket_fd == INVALID_SOCKET) {
#ifdef _WIN32
        WSACleanup();
#endif
        return core::ERR_IO;
    }
    
    // Configure DNS server address
    memset(&g_dns_tunnel_state.dns_server_addr, 0, sizeof(g_dns_tunnel_state.dns_server_addr));
    g_dns_tunnel_state.dns_server_addr.sin_family = AF_INET;
    g_dns_tunnel_state.dns_server_addr.sin_port = htons(DNS_PORT);
    
    // Use inet_addr instead of inet_pton for better compatibility
#ifdef _WIN32
    g_dns_tunnel_state.dns_server_addr.sin_addr.s_addr = inet_addr(DNS_SERVER);
#else
    if (inet_pton(AF_INET, DNS_SERVER, &g_dns_tunnel_state.dns_server_addr.sin_addr) <= 0) {
        close(g_dns_tunnel_state.socket_fd);
        g_dns_tunnel_state.socket_fd = INVALID_SOCKET;
        return core::ERR_IO;
    }
#endif
    
    g_dns_tunnel_state.is_open = true;
    g_dns_tunnel_state.transaction_id = 1;
    
    return core::OK;
}

static core::ErrorCode dns_tunnel_close() {
    if (!g_dns_tunnel_state.is_open) {
        return core::OK;
    }
    
    if (g_dns_tunnel_state.socket_fd != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(g_dns_tunnel_state.socket_fd);
        WSACleanup();
#else
        close(g_dns_tunnel_state.socket_fd);
#endif
        g_dns_tunnel_state.socket_fd = INVALID_SOCKET;
    }
    
    g_dns_tunnel_state.is_open = false;
    return core::OK;
}

static core::ErrorCode dns_tunnel_send(const core::Buffer& data) {
    if (!g_dns_tunnel_state.is_open) {
        return core::ERR_IO;
    }
    
    if (data.size == 0 || data.size > 200) {  // Limit to prevent excessive DNS queries
        return core::ERR_FORMAT;
    }
    
    // Encode data to base32
    char encoded[512];
    size_t encoded_len = base32_encode(data.data, data.size, encoded, sizeof(encoded));
    if (encoded_len == 0) {
        return core::ERR_FORMAT;
    }
    
    // Build domain name: <encoded_data>.<tunnel_domain>
    char domain[MAX_DOMAIN_LENGTH];
    int written = snprintf(domain, sizeof(domain), "%s.%s", encoded, TUNNEL_DOMAIN);
    if (written < 0 || static_cast<size_t>(written) >= sizeof(domain)) {
        return core::ERR_FORMAT;
    }
    
    // Build DNS query
    uint8_t query[MAX_DNS_MESSAGE];
    size_t query_len = build_dns_query(domain, g_dns_tunnel_state.transaction_id++, query, sizeof(query));
    if (query_len == 0) {
        return core::ERR_FORMAT;
    }
    
    // Send DNS query
    ssize_t sent = sendto(g_dns_tunnel_state.socket_fd, 
                         reinterpret_cast<const char*>(query), 
                         query_len, 
                         0,
                         reinterpret_cast<struct sockaddr*>(&g_dns_tunnel_state.dns_server_addr),
                         sizeof(g_dns_tunnel_state.dns_server_addr));
    
    if (sent < 0 || static_cast<size_t>(sent) != query_len) {
        return core::ERR_IO;
    }
    
    return core::OK;
}

static core::ErrorCode dns_tunnel_recv(core::Buffer& data) {
    if (!g_dns_tunnel_state.is_open) {
        return core::ERR_IO;
    }
    
    // Receive DNS response
    uint8_t response[MAX_DNS_MESSAGE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    ssize_t received = recvfrom(g_dns_tunnel_state.socket_fd,
                               reinterpret_cast<char*>(response),
                               sizeof(response),
                               0,
                               reinterpret_cast<struct sockaddr*>(&from_addr),
                               &from_len);
    
    if (received < 0) {
        return core::ERR_TIMEOUT;  // No data available
    }
    
    if (received < static_cast<ssize_t>(DNS_HEADER_SIZE)) {
        return core::ERR_FORMAT;  // Response too short
    }
    
    // For now, just return empty data as full DNS response parsing is complex
    // In a real implementation, we would parse TXT records and decode the response
    data.size = 0;
    
    return core::OK;
}

static bool dns_tunnel_available() {
    return g_dns_tunnel_state.is_open;
}

// Static transport instance
static Transport g_dns_tunnel_transport = {
    "dns_tunnel",
    dns_tunnel_open,
    dns_tunnel_close,
    dns_tunnel_send,
    dns_tunnel_recv,
    dns_tunnel_available
};

Transport* DNSTunnelTransport::get_instance() {
    return &g_dns_tunnel_transport;
}

} // namespace transport
} // namespace federated

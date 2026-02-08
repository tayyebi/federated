#include "../../include/federated/transport/dns_tunnel.h"
#include "../../include/federated/core/error.h"
#include "../../include/federated/transport/udp.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    typedef int socklen_t;
    
    // Windows sleep function
    static void sleep_ms(uint32_t ms) {
        Sleep(ms);
    }
    
    // Get time in milliseconds
    static uint64_t get_time_ms() {
        return GetTickCount64();
    }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/time.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
    
    // POSIX sleep function
    static void sleep_ms(uint32_t ms) {
        usleep(ms * 1000);
    }
    
    // Get time in milliseconds
    static uint64_t get_time_ms() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
    }
#endif

namespace federated {
namespace transport {

// Rate limiter for anti-detection
struct RateLimiter {
    uint64_t last_query_time_ms;  // Last query timestamp in milliseconds
    uint32_t max_qps;              // Maximum queries per second
    uint32_t min_jitter_ms;        // Minimum jitter delay
    uint32_t max_jitter_ms;        // Maximum jitter delay
    bool initialized;
    
    RateLimiter() 
        : last_query_time_ms(0)
        , max_qps(5)
        , min_jitter_ms(100)
        , max_jitter_ms(500)
        , initialized(false)
    {}
    
    // Check if we should delay before sending next query
    void enforce_rate_limit() {
        if (!initialized) {
            // Initialize random seed on first use
            srand(static_cast<unsigned int>(time(nullptr)));
            initialized = true;
        }
        
        uint64_t now = get_time_ms();
        uint64_t min_interval_ms = 1000 / max_qps;  // Minimum time between queries
        
        if (last_query_time_ms > 0) {
            uint64_t elapsed = now - last_query_time_ms;
            
            if (elapsed < min_interval_ms) {
                // Need to wait to respect rate limit
                uint32_t wait_time = static_cast<uint32_t>(min_interval_ms - elapsed);
                sleep_ms(wait_time);
                now = get_time_ms();
            }
        }
        
        // Add random jitter for anti-detection
        uint32_t jitter = min_jitter_ms;
        if (max_jitter_ms > min_jitter_ms) {
            jitter += rand() % (max_jitter_ms - min_jitter_ms);
        }
        sleep_ms(jitter);
        
        last_query_time_ms = get_time_ms();
    }
};

// DNS Tunnel configuration (global, can be set before open)
static DNSTunnelConfig g_config;

// DNS message state
struct DNSTunnelState {
    SOCKET socket_fd;
    struct sockaddr_in dns_server_addr;
    bool is_open;
    uint16_t transaction_id;
    RateLimiter rate_limiter;
    
    DNSTunnelState() : socket_fd(INVALID_SOCKET), is_open(false), transaction_id(1) {
        memset(&dns_server_addr, 0, sizeof(dns_server_addr));
    }
};

static DNSTunnelState g_dns_tunnel_state;

// DNS protocol constants
constexpr size_t MAX_LABEL_LENGTH = 63;
constexpr size_t MAX_DOMAIN_LENGTH = 253;
constexpr size_t DNS_HEADER_SIZE = 12;
constexpr size_t MAX_DNS_MESSAGE = 512;

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
    g_dns_tunnel_state.dns_server_addr.sin_port = htons(g_config.dns_port);
    
    // Use inet_addr instead of inet_pton for better compatibility
#ifdef _WIN32
    g_dns_tunnel_state.dns_server_addr.sin_addr.s_addr = inet_addr(g_config.dns_server);
#else
    if (inet_pton(AF_INET, g_config.dns_server, &g_dns_tunnel_state.dns_server_addr.sin_addr) <= 0) {
        close(g_dns_tunnel_state.socket_fd);
        g_dns_tunnel_state.socket_fd = INVALID_SOCKET;
        return core::ERR_IO;
    }
#endif
    
    // Configure rate limiter
    g_dns_tunnel_state.rate_limiter.max_qps = g_config.max_qps;
    g_dns_tunnel_state.rate_limiter.min_jitter_ms = g_config.min_jitter_ms;
    g_dns_tunnel_state.rate_limiter.max_jitter_ms = g_config.max_jitter_ms;
    
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
    
    // Enforce rate limiting with anti-detection jitter
    g_dns_tunnel_state.rate_limiter.enforce_rate_limit();
    
    // Encode data to base32
    char encoded[512];
    size_t encoded_len = base32_encode(data.data, data.size, encoded, sizeof(encoded));
    if (encoded_len == 0) {
        return core::ERR_FORMAT;
    }
    
    // Build domain name: <encoded_data>.<base_domain>
    char domain[MAX_DOMAIN_LENGTH];
    int written = snprintf(domain, sizeof(domain), "%s.%s", encoded, g_config.base_domain);
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

void DNSTunnelTransport::configure(const DNSTunnelConfig& config) {
    // Copy configuration (should be called before open())
    g_config = config;
}

} // namespace transport
} // namespace federated

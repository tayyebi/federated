#include "../../include/federated/transport/udp.h"
#include <cstring>
#include <queue>

// Platform-specific socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

namespace federated {
namespace transport {

// UDP datagram state
constexpr size_t IP_HEADER_SIZE = 20;   // IPv4 header size
constexpr size_t UDP_HEADER_SIZE = 8;   // UDP header size
constexpr size_t MAX_IP_DATAGRAM = 65535; // Maximum IP datagram size
constexpr size_t MAX_DATAGRAM_SIZE = MAX_IP_DATAGRAM - IP_HEADER_SIZE - UDP_HEADER_SIZE; // 65507 bytes
constexpr int DEFAULT_PORT = 0; // Ephemeral port
constexpr const char* LOCALHOST = "127.0.0.1";
constexpr int RECV_TIMEOUT_MS = 100;

struct Datagram {
    uint8_t data[MAX_DATAGRAM_SIZE];
    size_t size;
    struct sockaddr_in addr;
    socklen_t addr_len;
};

struct UDPState {
    SOCKET socket_fd;
    int local_port;
    bool is_open;
    struct sockaddr_in peer_addr;
    bool has_peer;
    std::queue<Datagram> recv_queue;
    
    UDPState() : socket_fd(INVALID_SOCKET),
                 local_port(0),
                 is_open(false),
                 has_peer(false) {
        memset(&peer_addr, 0, sizeof(peer_addr));
    }
};

static UDPState g_udp_state;

// Platform-specific initialization
static bool platform_init() {
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
#else
    return true; // POSIX doesn't need initialization
#endif
}

// Platform-specific cleanup
static void platform_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Close a socket
static void close_socket(SOCKET sock) {
    if (sock != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
    }
}

// Set receive timeout
static bool set_recv_timeout(SOCKET sock, int timeout_ms) {
#ifdef _WIN32
    DWORD timeout = timeout_ms;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, 
                     (const char*)&timeout, sizeof(timeout)) == 0;
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
#endif
}

// Implementation functions
static core::ErrorCode udp_open() {
    if (g_udp_state.is_open) {
        return core::OK;
    }
    
    if (!platform_init()) {
        return core::ERR_IO;
    }
    
    // Create UDP datagram socket
    g_udp_state.socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_udp_state.socket_fd == INVALID_SOCKET) {
        platform_cleanup();
        return core::ERR_IO;
    }
    
    // Allow address reuse
    int reuse = 1;
    setsockopt(g_udp_state.socket_fd, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&reuse, sizeof(reuse));
    
    // Bind to localhost with ephemeral port
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    local_addr.sin_port = htons(DEFAULT_PORT);
    
    if (bind(g_udp_state.socket_fd, (struct sockaddr*)&local_addr, 
             sizeof(local_addr)) == SOCKET_ERROR) {
        close_socket(g_udp_state.socket_fd);
        g_udp_state.socket_fd = INVALID_SOCKET;
        platform_cleanup();
        return core::ERR_IO;
    }
    
    // Get the actual port assigned
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(g_udp_state.socket_fd, (struct sockaddr*)&local_addr, 
                    &addr_len) == SOCKET_ERROR) {
        close_socket(g_udp_state.socket_fd);
        g_udp_state.socket_fd = INVALID_SOCKET;
        platform_cleanup();
        return core::ERR_IO;
    }
    g_udp_state.local_port = ntohs(local_addr.sin_port);
    
    // Set receive timeout
    set_recv_timeout(g_udp_state.socket_fd, RECV_TIMEOUT_MS);
    
    g_udp_state.is_open = true;
    return core::OK;
}

static core::ErrorCode udp_close() {
    if (!g_udp_state.is_open) {
        return core::OK;
    }
    
    close_socket(g_udp_state.socket_fd);
    g_udp_state.socket_fd = INVALID_SOCKET;
    
    // Clear queue
    while (!g_udp_state.recv_queue.empty()) {
        g_udp_state.recv_queue.pop();
    }
    
    platform_cleanup();
    
    g_udp_state.is_open = false;
    g_udp_state.has_peer = false;
    g_udp_state.local_port = 0;
    memset(&g_udp_state.peer_addr, 0, sizeof(g_udp_state.peer_addr));
    
    return core::OK;
}

static core::ErrorCode udp_send(const core::Buffer& data) {
    if (!g_udp_state.is_open) {
        return core::ERR_IO;
    }
    
    if (data.size > MAX_DATAGRAM_SIZE) {
        return core::ERR_FORMAT;
    }
    
    // Setup peer address if not already set (use our own port for loopback)
    if (!g_udp_state.has_peer) {
        memset(&g_udp_state.peer_addr, 0, sizeof(g_udp_state.peer_addr));
        g_udp_state.peer_addr.sin_family = AF_INET;
        g_udp_state.peer_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
        g_udp_state.peer_addr.sin_port = htons(g_udp_state.local_port);
        g_udp_state.has_peer = true;
    }
    
    // Send datagram (entire buffer sent as one datagram)
    int sent = sendto(g_udp_state.socket_fd,
                     (const char*)data.data,
                     data.size,
                     0,
                     (struct sockaddr*)&g_udp_state.peer_addr,
                     sizeof(g_udp_state.peer_addr));
    
    if (sent == SOCKET_ERROR) {
        return core::ERR_IO;
    }
    
    if ((size_t)sent != data.size) {
        return core::ERR_IO;
    }
    
    // For loopback testing, immediately try to receive the datagram
    // This simulates receiving the message we just sent
    Datagram dgram;
    dgram.addr_len = sizeof(dgram.addr);
    
    int received = recvfrom(g_udp_state.socket_fd,
                           (char*)dgram.data,
                           MAX_DATAGRAM_SIZE,
                           0,
                           (struct sockaddr*)&dgram.addr,
                           &dgram.addr_len);
    
    if (received > 0) {
        dgram.size = received;
        g_udp_state.recv_queue.push(dgram);
    }
    
    return core::OK;
}

static core::ErrorCode udp_recv(core::Buffer& data) {
    if (!g_udp_state.is_open) {
        return core::ERR_IO;
    }
    
    // Check if we have queued datagrams
    if (g_udp_state.recv_queue.empty()) {
        // Try to receive a datagram
        Datagram dgram;
        dgram.addr_len = sizeof(dgram.addr);
        
        int received = recvfrom(g_udp_state.socket_fd,
                               (char*)dgram.data,
                               MAX_DATAGRAM_SIZE,
                               0,
                               (struct sockaddr*)&dgram.addr,
                               &dgram.addr_len);
        
        if (received == SOCKET_ERROR) {
#ifdef _WIN32
            int err_code = WSAGetLastError();
            if (err_code == WSAETIMEDOUT || err_code == WSAEWOULDBLOCK) {
                return core::ERR_TIMEOUT;
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return core::ERR_TIMEOUT;
            }
#endif
            return core::ERR_IO;
        }
        
        if (received == 0) {
            return core::ERR_TIMEOUT;
        }
        
        dgram.size = received;
        g_udp_state.recv_queue.push(dgram);
    }
    
    // Get datagram from queue
    Datagram dgram = g_udp_state.recv_queue.front();
    g_udp_state.recv_queue.pop();
    
    if (dgram.size > data.size) {
        return core::ERR_FORMAT;
    }
    
    // Copy entire datagram (preserves message boundaries)
    memcpy(data.data, dgram.data, dgram.size);
    data.size = dgram.size;
    
    return core::OK;
}

static bool udp_available() {
    return g_udp_state.is_open;
}

// Static transport instance
static Transport g_udp_transport = {
    "udp",
    udp_open,
    udp_close,
    udp_send,
    udp_recv,
    udp_available
};

Transport* UDPTransport::get_instance() {
    return &g_udp_transport;
}

} // namespace transport
} // namespace federated

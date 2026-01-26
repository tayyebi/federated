#include "../../include/federated/transport/tcp.h"
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

// TCP connection state
constexpr size_t MAX_MESSAGE_SIZE = 4096;
constexpr int DEFAULT_PORT = 0; // Ephemeral port
constexpr const char* LOCALHOST = "127.0.0.1";
constexpr int LISTEN_BACKLOG = 5;
constexpr int RECV_TIMEOUT_MS = 100;

struct Message {
    uint8_t data[MAX_MESSAGE_SIZE];
    size_t size;
};

struct TCPState {
    SOCKET listen_socket;
    SOCKET client_socket;
    SOCKET server_socket;
    int server_port;
    bool is_open;
    bool connection_established;
    std::queue<Message> recv_queue;
    
    TCPState() : listen_socket(INVALID_SOCKET), 
                 client_socket(INVALID_SOCKET),
                 server_socket(INVALID_SOCKET),
                 server_port(0),
                 is_open(false),
                 connection_established(false) {}
};

static TCPState g_tcp_state;

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

// Set socket to non-blocking mode
static bool set_nonblocking(SOCKET sock) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
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
static core::ErrorCode tcp_open() {
    if (g_tcp_state.is_open) {
        return core::OK;
    }
    
    if (!platform_init()) {
        return core::ERR_IO;
    }
    
    // Create listening socket for server side
    g_tcp_state.listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_tcp_state.listen_socket == INVALID_SOCKET) {
        platform_cleanup();
        return core::ERR_IO;
    }
    
    // Allow address reuse
    int reuse = 1;
    setsockopt(g_tcp_state.listen_socket, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&reuse, sizeof(reuse));
    
    // Bind to localhost with ephemeral port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    server_addr.sin_port = htons(DEFAULT_PORT);
    
    if (bind(g_tcp_state.listen_socket, (struct sockaddr*)&server_addr, 
             sizeof(server_addr)) == SOCKET_ERROR) {
        close_socket(g_tcp_state.listen_socket);
        g_tcp_state.listen_socket = INVALID_SOCKET;
        platform_cleanup();
        return core::ERR_IO;
    }
    
    // Get the actual port assigned
    socklen_t addr_len = sizeof(server_addr);
    if (getsockname(g_tcp_state.listen_socket, (struct sockaddr*)&server_addr, 
                    &addr_len) == SOCKET_ERROR) {
        close_socket(g_tcp_state.listen_socket);
        g_tcp_state.listen_socket = INVALID_SOCKET;
        platform_cleanup();
        return core::ERR_IO;
    }
    g_tcp_state.server_port = ntohs(server_addr.sin_port);
    
    // Start listening
    if (listen(g_tcp_state.listen_socket, LISTEN_BACKLOG) == SOCKET_ERROR) {
        close_socket(g_tcp_state.listen_socket);
        g_tcp_state.listen_socket = INVALID_SOCKET;
        platform_cleanup();
        return core::ERR_IO;
    }
    
    g_tcp_state.is_open = true;
    return core::OK;
}

static core::ErrorCode tcp_close() {
    if (!g_tcp_state.is_open) {
        return core::OK;
    }
    
    close_socket(g_tcp_state.client_socket);
    g_tcp_state.client_socket = INVALID_SOCKET;
    
    close_socket(g_tcp_state.server_socket);
    g_tcp_state.server_socket = INVALID_SOCKET;
    
    close_socket(g_tcp_state.listen_socket);
    g_tcp_state.listen_socket = INVALID_SOCKET;
    
    // Clear queue
    while (!g_tcp_state.recv_queue.empty()) {
        g_tcp_state.recv_queue.pop();
    }
    
    platform_cleanup();
    
    g_tcp_state.is_open = false;
    g_tcp_state.connection_established = false;
    g_tcp_state.server_port = 0;
    
    return core::OK;
}

// Establish connection (client connects to server)
static core::ErrorCode ensure_connection() {
    if (g_tcp_state.connection_established) {
        return core::OK; // Already connected
    }
    
    if (!g_tcp_state.is_open || g_tcp_state.listen_socket == INVALID_SOCKET) {
        return core::ERR_IO;
    }
    
    // Create client socket
    g_tcp_state.client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_tcp_state.client_socket == INVALID_SOCKET) {
        return core::ERR_IO;
    }
    
    // Set client socket to non-blocking
    set_nonblocking(g_tcp_state.client_socket);
    
    // Connect to our server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    server_addr.sin_port = htons(g_tcp_state.server_port);
    
    int connect_result = connect(g_tcp_state.client_socket, 
                                 (struct sockaddr*)&server_addr, 
                                 sizeof(server_addr));
    
    // Non-blocking connect will return error, wait briefly
#ifdef _WIN32
    if (connect_result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
#else
    if (connect_result == SOCKET_ERROR && errno != EINPROGRESS) {
#endif
        close_socket(g_tcp_state.client_socket);
        g_tcp_state.client_socket = INVALID_SOCKET;
        return core::ERR_IO;
    }
    
    // Accept the connection on server side
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Set listen socket to non-blocking for accept
    set_nonblocking(g_tcp_state.listen_socket);
    
    // Try accepting with a brief retry loop
    int retry_count = 10;
    while (retry_count-- > 0) {
        g_tcp_state.server_socket = accept(g_tcp_state.listen_socket, 
                                          (struct sockaddr*)&client_addr, &client_len);
        if (g_tcp_state.server_socket != INVALID_SOCKET) {
            break;
        }
#ifdef _WIN32
        Sleep(10); // 10ms
#else
        usleep(10000); // 10ms
#endif
    }
    
    if (g_tcp_state.server_socket == INVALID_SOCKET) {
        close_socket(g_tcp_state.client_socket);
        g_tcp_state.client_socket = INVALID_SOCKET;
        return core::ERR_IO;
    }
    
    // Set receive timeouts
    set_recv_timeout(g_tcp_state.client_socket, RECV_TIMEOUT_MS);
    set_recv_timeout(g_tcp_state.server_socket, RECV_TIMEOUT_MS);
    
    g_tcp_state.connection_established = true;
    return core::OK;
}

static core::ErrorCode tcp_send(const core::Buffer& data) {
    if (!g_tcp_state.is_open) {
        return core::ERR_IO;
    }
    
    if (data.size > MAX_MESSAGE_SIZE) {
        return core::ERR_FORMAT;
    }
    
    // Ensure we have a connection
    core::ErrorCode err = ensure_connection();
    if (err != core::OK) {
        return err;
    }
    
    // Send data from client to server
    size_t total_sent = 0;
    while (total_sent < data.size) {
        int sent = send(g_tcp_state.client_socket,
                       (const char*)(data.data + total_sent),
                       data.size - total_sent, 0);
        
        if (sent == SOCKET_ERROR) {
#ifdef _WIN32
            int err_code = WSAGetLastError();
            if (err_code == WSAEWOULDBLOCK) {
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
#ifdef _WIN32
                Sleep(1);
#else
                usleep(1000);
#endif
                continue;
            }
            return core::ERR_IO;
        }
        
        if (sent > 0) {
            total_sent += sent;
        }
    }
    
    // Immediately receive on server side and queue it
    uint8_t temp_buf[MAX_MESSAGE_SIZE];
    size_t total_received = 0;
    
    // Give time for data to arrive
#ifdef _WIN32
    Sleep(10);
#else
    usleep(10000);
#endif
    
    while (total_received < data.size) {
        int received = recv(g_tcp_state.server_socket,
                           (char*)(temp_buf + total_received),
                           data.size - total_received, 0);
        
        if (received == SOCKET_ERROR) {
#ifdef _WIN32
            int err_code = WSAGetLastError();
            if (err_code == WSAEWOULDBLOCK || err_code == WSAETIMEDOUT) {
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
#ifdef _WIN32
                Sleep(1);
#else
                usleep(1000);
#endif
                continue;
            }
            return core::ERR_IO;
        }
        
        if (received == 0) {
            break;
        }
        
        total_received += received;
    }
    
    if (total_received > 0) {
        Message msg;
        memcpy(msg.data, temp_buf, total_received);
        msg.size = total_received;
        g_tcp_state.recv_queue.push(msg);
    }
    
    return core::OK;
}

static core::ErrorCode tcp_recv(core::Buffer& data) {
    if (!g_tcp_state.is_open) {
        return core::ERR_IO;
    }
    
    // Ensure we have a connection
    core::ErrorCode err = ensure_connection();
    if (err != core::OK) {
        return err;
    }
    
    // Check if we have queued data
    if (g_tcp_state.recv_queue.empty()) {
        return core::ERR_TIMEOUT;
    }
    
    Message msg = g_tcp_state.recv_queue.front();
    g_tcp_state.recv_queue.pop();
    
    if (msg.size > data.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(data.data, msg.data, msg.size);
    data.size = msg.size;
    
    return core::OK;
}

static bool tcp_available() {
    return g_tcp_state.is_open;
}

// Static transport instance
static Transport g_tcp_transport = {
    "tcp",
    tcp_open,
    tcp_close,
    tcp_send,
    tcp_recv,
    tcp_available
};

Transport* TCPTransport::get_instance() {
    return &g_tcp_transport;
}

} // namespace transport
} // namespace federated

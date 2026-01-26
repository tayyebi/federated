/**
 * HTTP/1.1 Static File Server Implementation
 * 
 * RFC 9110 - HTTP Semantics
 * https://datatracker.ietf.org/doc/html/rfc9110
 * 
 * RFC 9112 - HTTP/1.1
 * https://datatracker.ietf.org/doc/html/rfc9112
 * 
 * Implementation Details:
 * - Request parsing (RFC 9112 Section 3)
 * - Response building (RFC 9112 Section 4)
 * - GET method (RFC 9110 Section 9.3.1)
 * - HEAD method (RFC 9110 Section 9.3.2)
 * - Status codes (RFC 9110 Section 15)
 * - TCP-based server using existing transport layer
 */

#include "federated/service/http_server.h"
#include <cstring>
#include <cstdio>

namespace federated {
namespace service {

// Internal server state
struct HttpServerState {
    HttpServerConfig config;
    bool running;
    transport::Transport* tcp_transport;
    
    HttpServerState() : running(false), tcp_transport(nullptr) {
        config.port = 8080;
        config.allow_directory_listing = false;
        strcpy(config.document_root, "./www");
    }
};

static HttpServerState g_server_state;

// Get singleton instance
HttpServer* HttpServer::get_instance() {
    static HttpServer instance;
    return &instance;
}

// Initialize server with configuration
core::ErrorCode HttpServer::init(const HttpServerConfig& config) {
    if (g_server_state.running) {
        return core::ERR_INTERNAL;
    }
    
    // Copy configuration
    g_server_state.config = config;
    
    // Initialize HTTP subsystem
    return http_server_init(config);
}

// Start server (binds to port and listens)
core::ErrorCode HttpServer::start() {
    if (g_server_state.running) {
        return core::ERR_INTERNAL;
    }
    
    // Get TCP transport
    g_server_state.tcp_transport = transport::TCPTransport::get_instance();
    if (!g_server_state.tcp_transport) {
        return core::ERR_INTERNAL;
    }
    
    // Open TCP transport (starts listening)
    core::ErrorCode err = g_server_state.tcp_transport->open();
    if (err != core::OK) {
        return err;
    }
    
    g_server_state.running = true;
    return core::OK;
}

// Stop server
void HttpServer::stop() {
    if (g_server_state.running && g_server_state.tcp_transport) {
        g_server_state.tcp_transport->close();
        g_server_state.running = false;
    }
}

// Handle a single HTTP request
core::ErrorCode HttpServer::handle_request(const core::Buffer& request, core::Buffer& response) {
    // Parse request to find end of request line
    size_t request_line_end = 0;
    for (size_t i = 0; i < request.size - 1; i++) {
        if (request.data[i] == '\r' && request.data[i + 1] == '\n') {
            request_line_end = i + 2;
            break;
        }
    }
    
    if (request_line_end == 0) {
        return core::ERR_FORMAT;
    }
    
    // Parse request line
    HttpRequest req;
    core::Buffer request_line(request.data, request_line_end - 2);
    if (!parse_http_request_line(request_line, req)) {
        return core::ERR_FORMAT;
    }
    
    // Parse headers (rest of request)
    if (request_line_end < request.size) {
        core::Buffer headers_data(request.data + request_line_end, 
                                  request.size - request_line_end);
        parse_http_headers(headers_data, req);
    }
    
    // Handle request based on method
    HttpResponse resp;
    core::ErrorCode err = core::OK;
    
    if (strcmp(req.method, "GET") == 0) {
        err = http_handle_get(req, g_server_state.config, resp);
    } else if (strcmp(req.method, "HEAD") == 0) {
        err = http_handle_head(req, g_server_state.config, resp);
    } else {
        // 500 Internal Server Error for unsupported methods (RFC 9110 Section 15.6.1)
        resp.status_code = 500;
        strcpy(resp.status_text, "Internal Server Error");
        strcpy(resp.content_type, "text/html");
        
        static uint8_t error_body[] = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        resp.body = core::Buffer(error_body, sizeof(error_body) - 1);
        resp.content_length = resp.body.size;
    }
    
    if (err != core::OK) {
        return err;
    }
    
    // Build response
    return build_http_response(resp, response);
}

// Check if server is running
bool HttpServer::is_running() const {
    return g_server_state.running;
}

// Get current configuration
const HttpServerConfig& HttpServer::get_config() const {
    return g_server_state.config;
}

// Service interface implementation
static core::ErrorCode http_server_handle_request(const core::Buffer& input, core::Buffer& output) {
    HttpServer* server = HttpServer::get_instance();
    return server->handle_request(input, output);
}

// Global HTTP server service
Service http_server_service = {
    "http_server",
    http_server_handle_request
};

} // namespace service
} // namespace federated

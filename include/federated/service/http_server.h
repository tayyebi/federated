#ifndef FEDERATED_SERVICE_HTTP_SERVER_H
#define FEDERATED_SERVICE_HTTP_SERVER_H

/**
 * HTTP/1.1 Static File Server
 * 
 * RFC References:
 * - RFC 9110: HTTP Semantics
 *   https://datatracker.ietf.org/doc/html/rfc9110
 *   Defines HTTP methods, status codes, and headers (obsoletes RFC 7231)
 * 
 * - RFC 9112: HTTP/1.1
 *   https://datatracker.ietf.org/doc/html/rfc9112
 *   Defines HTTP/1.1 message format and connection management (obsoletes RFC 7230)
 * 
 * This implementation provides:
 * - HTTP/1.1 server for serving static files
 * - GET method support (Section 9.3.1 of RFC 9110)
 * - HEAD method support (Section 9.3.2 of RFC 9110)
 * - Status codes: 200 OK, 404 Not Found, 500 Internal Server Error (Section 15 of RFC 9110)
 * - Content-Type header based on file extension
 * - Content-Length header (Section 8.6 of RFC 9110)
 * - TCP-based server using existing transport layer
 * - Static singleton pattern for server instance
 */

#include "http.h"
#include "service.h"
#include "../transport/tcp.h"
#include "../core/buffer.h"
#include "../core/error.h"

namespace federated {
namespace service {

/**
 * HTTP Server - Singleton static file server
 * 
 * Provides HTTP/1.1 static file serving over TCP.
 * Uses the Service interface for request handling.
 */
struct HttpServer {
    // Get singleton instance
    static HttpServer* get_instance();
    
    // Initialize server with configuration
    core::ErrorCode init(const HttpServerConfig& config);
    
    // Start server (binds to port and listens)
    core::ErrorCode start();
    
    // Stop server
    void stop();
    
    // Handle a single HTTP request
    core::ErrorCode handle_request(const core::Buffer& request, core::Buffer& response);
    
    // Check if server is running
    bool is_running() const;
    
    // Get current configuration
    const HttpServerConfig& get_config() const;
};

// Global HTTP server service
extern Service http_server_service;

} // namespace service
} // namespace federated

#endif // FEDERATED_SERVICE_HTTP_SERVER_H

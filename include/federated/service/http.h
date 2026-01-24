#ifndef FEDERATED_SERVICE_HTTP_H
#define FEDERATED_SERVICE_HTTP_H

/**
 * HTTP/1.1 Static File Server Implementation
 * 
 * RFC References:
 * - RFC 7230: Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
 *   https://datatracker.ietf.org/doc/html/rfc7230
 *   Defines HTTP message format, connection management, and routing
 * 
 * - RFC 7231: Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
 *   https://datatracker.ietf.org/doc/html/rfc7231
 *   Defines HTTP methods (GET, HEAD, POST), status codes, and headers
 * 
 * - RFC 7232: Hypertext Transfer Protocol (HTTP/1.1): Conditional Requests
 *   https://datatracker.ietf.org/doc/html/rfc7232
 *   Defines If-Modified-Since, ETag, and conditional request handling
 * 
 * - RFC 7233: Hypertext Transfer Protocol (HTTP/1.1): Range Requests
 *   https://datatracker.ietf.org/doc/html/rfc7233
 *   Defines partial content delivery and byte-range requests
 * 
 * - RFC 7234: Hypertext Transfer Protocol (HTTP/1.1): Caching
 *   https://datatracker.ietf.org/doc/html/rfc7234
 *   Defines cache control headers and caching behavior
 * 
 * - RFC 7235: Hypertext Transfer Protocol (HTTP/1.1): Authentication
 *   https://datatracker.ietf.org/doc/html/rfc7235
 *   Defines WWW-Authenticate and Authorization headers
 * 
 * This implementation provides:
 * - Basic HTTP/1.1 server for serving static files
 * - GET method support (Section 4.3.1 of RFC 7231)
 * - HEAD method support (Section 4.3.2 of RFC 7231)
 * - Status codes: 200 OK, 404 Not Found, 500 Internal Server Error (Section 6 of RFC 7231)
 * - Content-Type detection based on file extension
 * - Content-Length header (Section 3.3.2 of RFC 7230)
 * - Directory index (index.html) support
 */

#include "../core/buffer.h"
#include "../core/error.h"

namespace federated {
namespace service {

/**
 * HTTP Request structure
 */
struct HttpRequest {
    char method[16];        // GET, HEAD, POST, etc.
    char path[512];         // Request path (e.g., /index.html)
    char version[16];       // HTTP/1.1
    char headers[2048];     // Raw headers
};

/**
 * HTTP Response structure
 */
struct HttpResponse {
    int  status_code;       // 200, 404, 500, etc.
    char status_text[64];   // OK, Not Found, etc.
    char content_type[128]; // text/html, image/png, etc.
    size_t content_length;  // Size of body
    core::Buffer body;      // Response body
};

/**
 * HTTP Server configuration
 */
struct HttpServerConfig {
    char document_root[512]; // Root directory for serving files
    int  port;              // Server port
    bool allow_directory_listing;  // Allow listing directory contents
};

// Parse HTTP request line
// RFC 7230 Section 3.1.1: Request Line
bool parse_http_request_line(const core::Buffer& line, HttpRequest& req);

// Parse HTTP headers
// RFC 7230 Section 3.2: Header Fields
core::ErrorCode parse_http_headers(const core::Buffer& data, HttpRequest& req);

// Build HTTP response
// RFC 7230 Section 3.1.2: Status Line
core::ErrorCode build_http_response(const HttpResponse& resp, 
                                   core::Buffer& output);

// Get MIME type from file extension
const char* get_mime_type(const char* filename);

// Handle HTTP GET request
// RFC 7231 Section 4.3.1: GET
core::ErrorCode http_handle_get(const HttpRequest& req,
                                const HttpServerConfig& config,
                                HttpResponse& resp);

// Handle HTTP HEAD request
// RFC 7231 Section 4.3.2: HEAD
core::ErrorCode http_handle_head(const HttpRequest& req,
                                 const HttpServerConfig& config,
                                 HttpResponse& resp);

// Read file from document root
core::ErrorCode http_read_file(const char* document_root,
                               const char* path,
                               core::Buffer& output);

// Initialize HTTP server
core::ErrorCode http_server_init(const HttpServerConfig& config);

// Cleanup HTTP server
void http_server_cleanup();

} // namespace service
} // namespace federated

#endif // FEDERATED_SERVICE_HTTP_H

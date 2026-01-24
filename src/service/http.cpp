/**
 * HTTP/1.1 Static File Server Implementation
 * 
 * RFC 7230 - Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
 * https://datatracker.ietf.org/doc/html/rfc7230
 * 
 * RFC 7231 - Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
 * https://datatracker.ietf.org/doc/html/rfc7231
 * 
 * Implementation Details:
 * - Request parsing (RFC 7230 Section 3.1.1)
 * - Response building (RFC 7230 Section 3.1.2)
 * - GET method (RFC 7231 Section 4.3.1)
 * - HEAD method (RFC 7231 Section 4.3.2)
 * - Status codes (RFC 7231 Section 6)
 */

#include "federated/service/http.h"
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

namespace federated {
namespace service {

// Parse HTTP request line
// RFC 7230 Section 3.1.1: request-line = method SP request-target SP HTTP-version CRLF
bool parse_http_request_line(const core::Buffer& line, HttpRequest& req) {
    if (line.size == 0) return false;
    
    size_t i = 0;
    size_t j = 0;
    
    // Parse method
    while (i < line.size && line.data[i] != ' ') {
        if (j < 15) req.method[j++] = line.data[i];
        i++;
    }
    req.method[j] = '\0';
    
    if (i >= line.size) return false;
    i++; // Skip space
    
    // Parse path
    j = 0;
    while (i < line.size && line.data[i] != ' ') {
        if (j < 511) req.path[j++] = line.data[i];
        i++;
    }
    req.path[j] = '\0';
    
    if (i >= line.size) return false;
    i++; // Skip space
    
    // Parse HTTP version
    j = 0;
    while (i < line.size && line.data[i] != '\r' && line.data[i] != '\n') {
        if (j < 15) req.version[j++] = line.data[i];
        i++;
    }
    req.version[j] = '\0';
    
    return true;
}

// Parse HTTP headers
// RFC 7230 Section 3.2: Header fields
core::ErrorCode parse_http_headers(const core::Buffer& data, HttpRequest& req) {
    // For now, just store raw headers
    if (data.size > sizeof(req.headers) - 1) {
        return core::ERR_FORMAT;
    }
    
    memcpy(req.headers, data.data, data.size);
    req.headers[data.size] = '\0';
    
    return core::OK;
}

// Get MIME type from file extension
const char* get_mime_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";
    
    // Common web types
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "text/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".xml") == 0) return "application/xml";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    
    // Images
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    
    // Fonts
    if (strcmp(ext, ".woff") == 0) return "font/woff";
    if (strcmp(ext, ".woff2") == 0) return "font/woff2";
    if (strcmp(ext, ".ttf") == 0) return "font/ttf";
    
    return "application/octet-stream";
}

// Build HTTP response
// RFC 7230 Section 3.1.2: status-line = HTTP-version SP status-code SP reason-phrase CRLF
core::ErrorCode build_http_response(const HttpResponse& resp, core::Buffer& output) {
    char header[2048];
    int written = 0;
    
    // Status line (RFC 7230 Section 3.1.2)
    written = snprintf(header, sizeof(header),
                      "HTTP/1.1 %d %s\r\n",
                      resp.status_code, resp.status_text);
    
    if (written < 0 || (size_t)written >= sizeof(header)) {
        return core::ERR_FORMAT;
    }
    
    // Content-Type header (RFC 7231 Section 3.1.1.5)
    written += snprintf(header + written, sizeof(header) - written,
                       "Content-Type: %s\r\n", resp.content_type);
    
    // Content-Length header (RFC 7230 Section 3.3.2)
    written += snprintf(header + written, sizeof(header) - written,
                       "Content-Length: %zu\r\n", resp.content_length);
    
    // Connection header
    written += snprintf(header + written, sizeof(header) - written,
                       "Connection: close\r\n");
    
    // End of headers
    written += snprintf(header + written, sizeof(header) - written, "\r\n");
    
    // Copy header to output
    if ((size_t)written + resp.body.size > output.size) {
        return core::ERR_FORMAT;
    }
    
    memcpy(output.data, header, written);
    
    // Copy body if present
    if (resp.body.data && resp.body.size > 0) {
        memcpy(output.data + written, resp.body.data, resp.body.size);
    }
    
    output.size = written + resp.body.size;
    
    return core::OK;
}

// Read file from document root
core::ErrorCode http_read_file(const char* document_root,
                              const char* path,
                              core::Buffer& output) {
    char full_path[1024];
    
    // Build full path
    snprintf(full_path, sizeof(full_path), "%s%s", document_root, path);
    
    // Security: prevent directory traversal
    if (strstr(path, "..") != nullptr) {
        return core::ERR_FORMAT;
    }
    
    // Check if path is a directory and look for index.html
    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        snprintf(full_path, sizeof(full_path), "%s%s/index.html", document_root, path);
    }
    
    // Try to open file
    FILE* f = fopen(full_path, "rb");
    if (!f) {
        return core::ERR_IO;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size < 0 || (size_t)size > output.size) {
        fclose(f);
        return core::ERR_FORMAT;
    }
    
    // Read file
    size_t read_size = fread(output.data, 1, size, f);
    fclose(f);
    
    if ((long)read_size != size) {
        return core::ERR_IO;
    }
    
    output.size = read_size;
    return core::OK;
}

// Handle HTTP GET request
// RFC 7231 Section 4.3.1: The GET method requests transfer of a current selected representation
core::ErrorCode http_handle_get(const HttpRequest& req,
                                const HttpServerConfig& config,
                                HttpResponse& resp) {
    // Allocate buffer for file content (up to 1MB)
    static uint8_t file_buffer[1024 * 1024];
    core::Buffer file_data(file_buffer, sizeof(file_buffer));
    
    core::ErrorCode err = http_read_file(config.document_root, req.path, file_data);
    
    if (err != core::OK) {
        // 404 Not Found (RFC 7231 Section 6.5.4)
        resp.status_code = 404;
        strcpy(resp.status_text, "Not Found");
        strcpy(resp.content_type, "text/html");
        
        static uint8_t not_found[] = "<html><body><h1>404 Not Found</h1></body></html>";
        resp.body = core::Buffer(not_found, sizeof(not_found) - 1);
        resp.content_length = resp.body.size;
        
        return core::OK;
    }
    
    // 200 OK (RFC 7231 Section 6.3.1)
    resp.status_code = 200;
    strcpy(resp.status_text, "OK");
    
    // Determine content type
    const char* mime = get_mime_type(req.path);
    strcpy(resp.content_type, mime);
    
    resp.body = file_data;
    resp.content_length = resp.body.size;
    
    return core::OK;
}

// Handle HTTP HEAD request
// RFC 7231 Section 4.3.2: The HEAD method is identical to GET except that the server MUST NOT send a message body
core::ErrorCode http_handle_head(const HttpRequest& req,
                                 const HttpServerConfig& config,
                                 HttpResponse& resp) {
    // HEAD is like GET but without body
    core::ErrorCode err = http_handle_get(req, config, resp);
    
    // Remove body for HEAD
    resp.body.data = nullptr;
    resp.body.size = 0;
    
    return err;
}

// Initialize HTTP server
core::ErrorCode http_server_init(const HttpServerConfig& /* config */) {
    // No initialization needed for static file serving
    return core::OK;
}

// Cleanup HTTP server
void http_server_cleanup() {
    // No cleanup needed
}

} // namespace service
} // namespace federated

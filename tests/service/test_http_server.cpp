#include "../test_runner.h"
#include "../../include/federated/service/http_server.h"
#include <cstring>
#include <cstdio>

using namespace federated::service;
using namespace federated::core;

// Test: HTTP server initialization
TEST(http_server_init) {
    HttpServer* server = HttpServer::get_instance();
    TEST_ASSERT(server != nullptr);
    
    HttpServerConfig config;
    strcpy(config.document_root, "/tmp/www");
    config.port = 8080;
    config.allow_directory_listing = false;
    
    ErrorCode err = server->init(config);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify configuration
    const HttpServerConfig& cfg = server->get_config();
    TEST_ASSERT_STR_EQ(cfg.document_root, "/tmp/www");
    TEST_ASSERT_EQ(cfg.port, 8080);
}

// Test: Parse HTTP request method
TEST(http_server_parse_method) {
    uint8_t request_data[] = "GET /index.html HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    
    // This will fail to find file, but should parse method correctly
    ErrorCode err = server->handle_request(request, response);
    TEST_ASSERT_EQ(err, OK);
    
    // Response should contain HTTP header
    TEST_ASSERT(strstr((char*)response.data, "HTTP/1.1") != nullptr);
}

// Test: Parse HTTP request path
TEST(http_server_parse_path) {
    uint8_t request_data[] = "GET /test/file.html HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    // Should parse and return 404 for non-existent file
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr((char*)response.data, "404") != nullptr);
}

// Test: Parse HTTP headers
TEST(http_server_parse_headers) {
    uint8_t request_data[] = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: TestClient/1.0\r\n"
        "Accept: text/html\r\n"
        "\r\n";
    
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    // Should parse successfully
    TEST_ASSERT_EQ(err, OK);
}

// Test: Generate HTTP response with status code
TEST(http_server_response_status_code) {
    uint8_t request_data[] = "GET /nonexistent.html HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    TEST_ASSERT_EQ(err, OK);
    
    // Should return 404 Not Found
    TEST_ASSERT(strstr((char*)response.data, "404") != nullptr);
    TEST_ASSERT(strstr((char*)response.data, "Not Found") != nullptr);
}

// Test: Generate HTTP response with headers
TEST(http_server_response_headers) {
    uint8_t request_data[] = "GET /test.html HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    TEST_ASSERT_EQ(err, OK);
    
    // Should have Content-Type header
    TEST_ASSERT(strstr((char*)response.data, "Content-Type:") != nullptr);
    
    // Should have Content-Length header
    TEST_ASSERT(strstr((char*)response.data, "Content-Length:") != nullptr);
}

// Test: Generate HTTP response with body
TEST(http_server_response_body) {
    uint8_t request_data[] = "GET /missing.html HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    TEST_ASSERT_EQ(err, OK);
    
    // Should have a body (404 page)
    TEST_ASSERT(strstr((char*)response.data, "\r\n\r\n") != nullptr);
    
    // Body should contain HTML
    TEST_ASSERT(strstr((char*)response.data, "<html>") != nullptr);
}

// Test: MIME type detection for HTML
TEST(http_server_mime_type_html) {
    const char* mime = get_mime_type("index.html");
    TEST_ASSERT_STR_EQ(mime, "text/html");
}

// Test: MIME type detection for plain text
TEST(http_server_mime_type_text) {
    const char* mime = get_mime_type("readme.txt");
    TEST_ASSERT_STR_EQ(mime, "text/plain");
}

// Test: MIME type detection for PNG image
TEST(http_server_mime_type_png) {
    const char* mime = get_mime_type("logo.png");
    TEST_ASSERT_STR_EQ(mime, "image/png");
}

// Test: MIME type detection for CSS
TEST(http_server_mime_type_css) {
    const char* mime = get_mime_type("style.css");
    TEST_ASSERT_STR_EQ(mime, "text/css");
}

// Test: MIME type detection for JavaScript
TEST(http_server_mime_type_js) {
    const char* mime = get_mime_type("app.js");
    TEST_ASSERT_STR_EQ(mime, "text/javascript");
}

// Test: Basic GET request handling
TEST(http_server_handle_get) {
    uint8_t request_data[] = "GET / HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(response.size > 0);
}

// Test: HEAD request handling
TEST(http_server_handle_head) {
    uint8_t request_data[] = "HEAD /index.html HTTP/1.1\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    TEST_ASSERT_EQ(err, OK);
    
    // HEAD should have headers but minimal/no body
    TEST_ASSERT(strstr((char*)response.data, "HTTP/1.1") != nullptr);
}

// Test: Invalid request format
TEST(http_server_invalid_request) {
    uint8_t request_data[] = "INVALID REQUEST\r\n\r\n";
    Buffer request(request_data, sizeof(request_data) - 1);
    
    uint8_t response_data[4096];
    Buffer response(response_data, sizeof(response_data));
    
    HttpServer* server = HttpServer::get_instance();
    ErrorCode err = server->handle_request(request, response);
    
    // Should return error for invalid format
    TEST_ASSERT_EQ(err, ERR_FORMAT);
}

// Test: Server running state
TEST(http_server_running_state) {
    HttpServer* server = HttpServer::get_instance();
    
    // Initially not running
    TEST_ASSERT(!server->is_running());
    
    // Note: We don't actually start the server in unit tests
    // as it would require binding to a port
}

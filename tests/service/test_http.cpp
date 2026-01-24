#include "../test_runner.h"
#include "../../include/federated/service/http.h"
#include <cstring>

using namespace federated::service;
using namespace federated::core;

// Test: Parse HTTP request line
TEST(http_parse_request_line) {
    uint8_t data[] = "GET /index.html HTTP/1.1\r\n";
    Buffer buf(data, sizeof(data) - 1);
    HttpRequest req;
    
    bool result = parse_http_request_line(buf, req);
    
    TEST_ASSERT(result);
    TEST_ASSERT_STR_EQ(req.method, "GET");
    TEST_ASSERT_STR_EQ(req.path, "/index.html");
    TEST_ASSERT_STR_EQ(req.version, "HTTP/1.1");
}

// Test: Parse HTTP request with root path
TEST(http_parse_request_root) {
    uint8_t data[] = "GET / HTTP/1.1\r\n";
    Buffer buf(data, sizeof(data) - 1);
    HttpRequest req;
    
    bool result = parse_http_request_line(buf, req);
    
    TEST_ASSERT(result);
    TEST_ASSERT_STR_EQ(req.method, "GET");
    TEST_ASSERT_STR_EQ(req.path, "/");
}

// Test: Parse HEAD request
TEST(http_parse_head_request) {
    uint8_t data[] = "HEAD /about.html HTTP/1.1\r\n";
    Buffer buf(data, sizeof(data) - 1);
    HttpRequest req;
    
    bool result = parse_http_request_line(buf, req);
    
    TEST_ASSERT(result);
    TEST_ASSERT_STR_EQ(req.method, "HEAD");
    TEST_ASSERT_STR_EQ(req.path, "/about.html");
}

// Test: Get MIME type
TEST(http_get_mime_type_html) {
    const char* mime = get_mime_type("index.html");
    TEST_ASSERT_STR_EQ(mime, "text/html");
}

TEST(http_get_mime_type_css) {
    const char* mime = get_mime_type("style.css");
    TEST_ASSERT_STR_EQ(mime, "text/css");
}

TEST(http_get_mime_type_js) {
    const char* mime = get_mime_type("script.js");
    TEST_ASSERT_STR_EQ(mime, "text/javascript");
}

TEST(http_get_mime_type_json) {
    const char* mime = get_mime_type("data.json");
    TEST_ASSERT_STR_EQ(mime, "application/json");
}

TEST(http_get_mime_type_png) {
    const char* mime = get_mime_type("image.png");
    TEST_ASSERT_STR_EQ(mime, "image/png");
}

TEST(http_get_mime_type_jpg) {
    const char* mime = get_mime_type("photo.jpg");
    TEST_ASSERT_STR_EQ(mime, "image/jpeg");
}

TEST(http_get_mime_type_unknown) {
    const char* mime = get_mime_type("file.unknown");
    TEST_ASSERT_STR_EQ(mime, "application/octet-stream");
}

// Test: Build HTTP response
TEST(http_build_response_200) {
    HttpResponse resp;
    resp.status_code = 200;
    strcpy(resp.status_text, "OK");
    strcpy(resp.content_type, "text/html");
    
    uint8_t body_data[] = "<html><body>Hello</body></html>";
    resp.body = Buffer(body_data, sizeof(body_data) - 1);
    resp.content_length = resp.body.size;
    
    uint8_t output_data[1024];
    Buffer output(output_data, sizeof(output_data));
    
    ErrorCode err = build_http_response(resp, output);
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr((char*)output.data, "HTTP/1.1 200 OK") != nullptr);
    TEST_ASSERT(strstr((char*)output.data, "Content-Type: text/html") != nullptr);
    TEST_ASSERT(strstr((char*)output.data, "Content-Length:") != nullptr);
}

// Test: Build 404 response
TEST(http_build_response_404) {
    HttpResponse resp;
    resp.status_code = 404;
    strcpy(resp.status_text, "Not Found");
    strcpy(resp.content_type, "text/html");
    
    uint8_t body_data[] = "<html><body>Not Found</body></html>";
    resp.body = Buffer(body_data, sizeof(body_data) - 1);
    resp.content_length = resp.body.size;
    
    uint8_t output_data[1024];
    Buffer output(output_data, sizeof(output_data));
    
    ErrorCode err = build_http_response(resp, output);
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr((char*)output.data, "HTTP/1.1 404 Not Found") != nullptr);
}

// Test: Handle GET for non-existent file
TEST(http_handle_get_not_found) {
    HttpRequest req;
    strcpy(req.method, "GET");
    strcpy(req.path, "/nonexistent.html");
    
    HttpServerConfig config;
    strcpy(config.document_root, "/tmp");
    config.allow_directory_listing = false;
    
    HttpResponse resp;
    
    ErrorCode err = http_handle_get(req, config, resp);
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(resp.status_code, 404);
}

// Test: Handle HEAD request
TEST(http_handle_head_request) {
    HttpRequest req;
    strcpy(req.method, "HEAD");
    strcpy(req.path, "/test.html");
    
    HttpServerConfig config;
    strcpy(config.document_root, "/tmp");
    config.allow_directory_listing = false;
    
    HttpResponse resp;
    
    ErrorCode err = http_handle_head(req, config, resp);
    
    // HEAD should return headers only, no body
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(resp.body.size == 0 || resp.status_code == 404);
}

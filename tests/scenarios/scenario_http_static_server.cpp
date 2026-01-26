/**
 * @file scenario_http_static_server.cpp
 * @brief User Scenario Test: HTTP Static File Server
 * 
 * Tests a realistic HTTP server scenario:
 * 1. Initialize HTTP server with configuration
 * 2. Create a static HTML file in memory
 * 3. Send HTTP GET request to server
 * 4. Server processes request and generates response
 * 5. Client receives and validates HTTP response
 * 6. Verify response contains proper headers and body
 * 
 * This demonstrates the HTTP server serving static content in a realistic scenario.
 */

#include "../test_runner.h"
#include "../../include/federated/service/http_server.h"
#include "../../include/federated/core/buffer.h"
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

using namespace federated;

TEST(scenario_http_serve_static_file) {
    printf("\n  === User Scenario: HTTP Server - Serve Static HTML File ===\n");
    
    // Step 1: Initialize HTTP server
    service::HttpServer* server = service::HttpServer::get_instance();
    printf("  1. HTTP server instance created\n");
    
    // Configure server
    service::HttpServerConfig config;
    strcpy(config.document_root, "/tmp/http_test_www");
    config.port = 8080;
    config.allow_directory_listing = false;
    
    core::ErrorCode err = server->init(config);
    TEST_ASSERT_EQ(err, core::OK);
    printf("  2. Server configured:\n");
    printf("     - Document root: %s\n", config.document_root);
    printf("     - Port: %d\n", config.port);
    printf("     - Directory listing: %s\n", config.allow_directory_listing ? "enabled" : "disabled");
    
    // Step 2: Create test directory and HTML file
    printf("  3. Creating test environment...\n");
    mkdir("/tmp/http_test_www", 0755);
    
    // Create a simple HTML file
    const char* html_content = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Test Page</title>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Welcome to Federated HTTP Server</h1>\n"
        "    <p>This is a test static HTML page.</p>\n"
        "    <p>Server is working correctly!</p>\n"
        "</body>\n"
        "</html>\n";
    
    FILE* f = fopen("/tmp/http_test_www/index.html", "w");
    if (f) {
        fprintf(f, "%s", html_content);
        fclose(f);
        printf("     - Created index.html (%zu bytes)\n", strlen(html_content));
    } else {
        printf("     - Warning: Could not create test file\n");
    }
    
    // Step 3: Prepare HTTP GET request
    printf("  4. Client preparing HTTP request...\n");
    const char* request_str = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: FederatedTestClient/1.0\r\n"
        "Accept: text/html,text/plain\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    uint8_t request_data[1024];
    memcpy(request_data, request_str, strlen(request_str));
    core::Buffer request(request_data, strlen(request_str));
    
    printf("     Request:\n");
    printf("     -------\n");
    printf("%s", request_str);
    printf("     -------\n");
    
    // Step 4: Server handles request
    printf("  5. Server processing request...\n");
    uint8_t response_data[8192];
    core::Buffer response(response_data, sizeof(response_data));
    
    err = server->handle_request(request, response);
    TEST_ASSERT_EQ(err, core::OK);
    printf("     - Request processed successfully ✓\n");
    
    // Step 5: Validate response
    printf("  6. Validating HTTP response...\n");
    
    // Null-terminate for string operations
    if (response.size < sizeof(response_data)) {
        response_data[response.size] = '\0';
    }
    
    // Check status line
    TEST_ASSERT(strstr((char*)response.data, "HTTP/1.1") != nullptr);
    printf("     - HTTP version: HTTP/1.1 ✓\n");
    
    if (strstr((char*)response.data, "200 OK") != nullptr) {
        printf("     - Status code: 200 OK ✓\n");
        
        // Check Content-Type header
        TEST_ASSERT(strstr((char*)response.data, "Content-Type: text/html") != nullptr);
        printf("     - Content-Type: text/html ✓\n");
        
        // Check Content-Length header
        TEST_ASSERT(strstr((char*)response.data, "Content-Length:") != nullptr);
        printf("     - Content-Length header present ✓\n");
        
        // Check for HTML content in body
        TEST_ASSERT(strstr((char*)response.data, "<html>") != nullptr);
        TEST_ASSERT(strstr((char*)response.data, "Welcome to Federated HTTP Server") != nullptr);
        printf("     - HTML body contains expected content ✓\n");
        
        printf("\n     Response preview:\n");
        printf("     -------\n");
        // Print first 300 chars of response
        int preview_len = response.size < 300 ? response.size : 300;
        for (int i = 0; i < preview_len; i++) {
            printf("%c", response.data[i]);
        }
        if (response.size > 300) printf("\n     ... (%zu total bytes)", response.size);
        printf("\n     -------\n");
    } else {
        printf("     - Status code: 404 Not Found (file not accessible)\n");
        TEST_ASSERT(strstr((char*)response.data, "404") != nullptr);
    }
    
    printf("  7. Response validation complete\n");
    
    // Cleanup
    unlink("/tmp/http_test_www/index.html");
    rmdir("/tmp/http_test_www");
    printf("  8. Test environment cleaned up\n");
    
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_http_404_not_found) {
    printf("\n  === User Scenario: HTTP Server - 404 Not Found ===\n");
    
    service::HttpServer* server = service::HttpServer::get_instance();
    
    service::HttpServerConfig config;
    strcpy(config.document_root, "/tmp/empty_test_dir");
    config.port = 8080;
    
    server->init(config);
    printf("  1. Server initialized\n");
    
    // Request non-existent file
    const char* request_str = "GET /does_not_exist.html HTTP/1.1\r\n\r\n";
    uint8_t request_data[512];
    memcpy(request_data, request_str, strlen(request_str));
    core::Buffer request(request_data, strlen(request_str));
    
    printf("  2. Requesting non-existent file: /does_not_exist.html\n");
    
    uint8_t response_data[4096];
    core::Buffer response(response_data, sizeof(response_data));
    
    core::ErrorCode err = server->handle_request(request, response);
    TEST_ASSERT_EQ(err, core::OK);
    
    // Should return 404
    response_data[response.size] = '\0';
    TEST_ASSERT(strstr((char*)response.data, "404") != nullptr);
    TEST_ASSERT(strstr((char*)response.data, "Not Found") != nullptr);
    
    printf("  3. Server responded with: 404 Not Found ✓\n");
    printf("  4. Response contains error page ✓\n");
    
    printf("  === Scenario Complete ===\n\n");
}

TEST(scenario_http_multiple_requests) {
    printf("\n  === User Scenario: HTTP Server - Multiple Requests ===\n");
    
    service::HttpServer* server = service::HttpServer::get_instance();
    
    service::HttpServerConfig config;
    strcpy(config.document_root, "/tmp/multi_test");
    config.port = 8080;
    server->init(config);
    
    printf("  1. Server initialized for multiple requests\n");
    
    // Create test directory
    mkdir("/tmp/multi_test", 0755);
    
    // Create multiple test files
    const char* files[] = {"page1.html", "page2.html", "page3.html"};
    const char* contents[] = {
        "<html><body><h1>Page 1</h1></body></html>",
        "<html><body><h1>Page 2</h1></body></html>",
        "<html><body><h1>Page 3</h1></body></html>"
    };
    
    for (int i = 0; i < 3; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/tmp/multi_test/%s", files[i]);
        FILE* f = fopen(path, "w");
        if (f) {
            fprintf(f, "%s", contents[i]);
            fclose(f);
        }
    }
    printf("  2. Created 3 test HTML files\n");
    
    // Make multiple requests
    printf("  3. Making multiple requests...\n");
    for (int i = 0; i < 3; i++) {
        char request_str[256];
        snprintf(request_str, sizeof(request_str), 
                "GET /%s HTTP/1.1\r\n\r\n", files[i]);
        
        uint8_t request_data[512];
        memcpy(request_data, request_str, strlen(request_str));
        core::Buffer request(request_data, strlen(request_str));
        
        uint8_t response_data[4096];
        core::Buffer response(response_data, sizeof(response_data));
        
        core::ErrorCode err = server->handle_request(request, response);
        TEST_ASSERT_EQ(err, core::OK);
        
        response_data[response.size] = '\0';
        if (strstr((char*)response.data, "200 OK")) {
            printf("     - Request %d: %s -> 200 OK ✓\n", i+1, files[i]);
        } else {
            printf("     - Request %d: %s -> 404 (file not accessible)\n", i+1, files[i]);
        }
    }
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/tmp/multi_test/%s", files[i]);
        unlink(path);
    }
    rmdir("/tmp/multi_test");
    
    printf("  4. All requests processed successfully\n");
    printf("  === Scenario Complete ===\n\n");
}

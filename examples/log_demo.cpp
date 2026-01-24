#include "federated/tool/log.h"

using namespace federated::tool::log;

int main() {
    // Initialize logger
    LogConfig config;
    config.level = LOG_TRACE;
    config.use_color = true;
    config.driver = DRIVER_STDOUT;
    log_init(config);
    
    // Demonstrate all log levels
    LOG_HEADER("Federated Logging System Demo");
    
    LOG_ERROR("This is an error message - displayed in RED");
    LOG_WARN("This is a warning message - displayed in YELLOW");
    LOG_INFO("This is an info message - displayed in GRAY");
    LOG_DEBUG("This is a debug message - displayed in GRAY");
    LOG_TRACE("This is a trace message - displayed in GRAY");
    
    LOG_HEADER("Service Configuration");
    LOG_INFO("SMTP Port: 2525");
    LOG_INFO("IMAP Port: 1143");
    LOG_WARN("Using default configuration");
    
    LOG_HEADER("Status Check");
    LOG_INFO("All services operational");
    LOG_ERROR("Failed to connect to database");
    
    // Cleanup
    log_cleanup();
    
    return 0;
}

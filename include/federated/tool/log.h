#ifndef FEDERATED_TOOL_LOG_H
#define FEDERATED_TOOL_LOG_H

#include "../core/error.h"
#include <cstddef>
#include <cstdio>

namespace federated {
namespace tool {
namespace log {

/**
 * Log levels
 */
enum LogLevel {
    LOG_ERROR = 0,
    LOG_WARN = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3,
    LOG_TRACE = 4
};

/**
 * Log driver types
 */
enum LogDriver {
    DRIVER_STDOUT,
    DRIVER_FILE,
    DRIVER_BOTH
};

/**
 * ANSI color codes
 */
namespace color {
    const char* const RESET = "\033[0m";
    const char* const RED = "\033[31m";
    const char* const YELLOW = "\033[33m";
    const char* const GRAY = "\033[90m";
    const char* const BLACK = "\033[30m";
    const char* const BG_LIGHT_GRAY = "\033[47m";
    const char* const BOLD = "\033[1m";
}

/**
 * Logger configuration
 */
struct LogConfig {
    LogLevel    level;
    LogDriver   driver;
    bool        use_color;
    const char* log_file_path;
    FILE*       log_file_handle;
    
    LogConfig() : level(LOG_INFO), driver(DRIVER_STDOUT), 
                  use_color(true), log_file_path(nullptr),
                  log_file_handle(nullptr) {}
};

// Initialize logger
core::ErrorCode log_init(const LogConfig& config);

// Cleanup logger
void log_cleanup();

// Set log level
void log_set_level(LogLevel level);

// Set color mode
void log_set_color(bool enabled);

// Get terminal width
int get_terminal_width();

// Log messages
void log_error(const char* format, ...);
void log_warn(const char* format, ...);
void log_info(const char* format, ...);
void log_debug(const char* format, ...);
void log_trace(const char* format, ...);

// Log header (full-width with background)
void log_header(const char* text);

// Log with specific level
void log_message(LogLevel level, const char* format, ...);

// Helper macros for convenience
#define LOG_ERROR(...) federated::tool::log::log_error(__VA_ARGS__)
#define LOG_WARN(...)  federated::tool::log::log_warn(__VA_ARGS__)
#define LOG_INFO(...)  federated::tool::log::log_info(__VA_ARGS__)
#define LOG_DEBUG(...) federated::tool::log::log_debug(__VA_ARGS__)
#define LOG_TRACE(...) federated::tool::log::log_trace(__VA_ARGS__)
#define LOG_HEADER(...) federated::tool::log::log_header(__VA_ARGS__)

} // namespace log
} // namespace tool
} // namespace federated

#endif // FEDERATED_TOOL_LOG_H

#include "federated/tool/log.h"
#include <cstdarg>
#include <cstring>
#include <ctime>

// Platform-specific includes for terminal width detection
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

namespace federated {
namespace tool {
namespace log {

// Global logger state
static LogConfig g_config;
static bool g_initialized = false;

core::ErrorCode log_init(const LogConfig& config) {
    g_config = config;
    
    // Open log file if needed
    if (g_config.log_file_path && 
        (g_config.driver == DRIVER_FILE || g_config.driver == DRIVER_BOTH)) {
        g_config.log_file_handle = fopen(g_config.log_file_path, "a");
        if (!g_config.log_file_handle) {
            fprintf(stderr, "Failed to open log file: %s\n", g_config.log_file_path);
            return core::ERR_IO;
        }
    }
    
    g_initialized = true;
    return core::OK;
}

void log_cleanup() {
    if (g_config.log_file_handle) {
        fclose(g_config.log_file_handle);
        g_config.log_file_handle = nullptr;
    }
    g_initialized = false;
}

void log_set_level(LogLevel level) {
    g_config.level = level;
}

void log_set_color(bool enabled) {
    g_config.use_color = enabled;
}

int get_terminal_width() {
#ifdef _WIN32
    // Windows implementation using console API
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80; // Default width
#else
    // POSIX implementation using ioctl
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; // Default width
#endif
}

// Get current timestamp
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Get level name
static const char* get_level_name(LogLevel level) {
    switch (level) {
        case LOG_ERROR: return "ERROR";
        case LOG_WARN:  return "WARN ";
        case LOG_INFO:  return "INFO ";
        case LOG_DEBUG: return "DEBUG";
        case LOG_TRACE: return "TRACE";
        default:        return "UNKN ";
    }
}

// Get level color
static const char* get_level_color(LogLevel level) {
    if (!g_config.use_color) return "";
    
    switch (level) {
        case LOG_ERROR: return color::RED;
        case LOG_WARN:  return color::YELLOW;
        case LOG_INFO:  return color::GRAY;
        case LOG_DEBUG: return color::GRAY;
        case LOG_TRACE: return color::GRAY;
        default:        return "";
    }
}

// Log to stdout
static void log_to_stdout(LogLevel level, const char* message) {
    if (g_config.driver == DRIVER_FILE) return;
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    const char* level_color = get_level_color(level);
    const char* reset = g_config.use_color ? color::RESET : "";
    const char* level_name = get_level_name(level);
    
    fprintf(stdout, "%s[%s] [%s]%s %s\n", 
            level_color, timestamp, level_name, reset, message);
    fflush(stdout);
}

// Log to file
static void log_to_file(LogLevel level, const char* message) {
    if (!g_config.log_file_handle) return;
    if (g_config.driver == DRIVER_STDOUT) return;
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    const char* level_name = get_level_name(level);
    
    fprintf(g_config.log_file_handle, "[%s] [%s] %s\n", 
            timestamp, level_name, message);
    fflush(g_config.log_file_handle);
}

void log_message(LogLevel level, const char* format, ...) {
    if (!g_initialized) {
        // Auto-initialize with defaults
        LogConfig default_config;
        log_init(default_config);
    }
    
    // Check log level
    if (level > g_config.level) return;
    
    // Format message
    char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Log to configured drivers
    log_to_stdout(level, message);
    log_to_file(level, message);
}

void log_error(const char* format, ...) {
    if (!g_initialized) {
        LogConfig default_config;
        log_init(default_config);
    }
    
    if (LOG_ERROR > g_config.level) return;
    
    char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    log_to_stdout(LOG_ERROR, message);
    log_to_file(LOG_ERROR, message);
}

void log_warn(const char* format, ...) {
    if (!g_initialized) {
        LogConfig default_config;
        log_init(default_config);
    }
    
    if (LOG_WARN > g_config.level) return;
    
    char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    log_to_stdout(LOG_WARN, message);
    log_to_file(LOG_WARN, message);
}

void log_info(const char* format, ...) {
    if (!g_initialized) {
        LogConfig default_config;
        log_init(default_config);
    }
    
    if (LOG_INFO > g_config.level) return;
    
    char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    log_to_stdout(LOG_INFO, message);
    log_to_file(LOG_INFO, message);
}

void log_debug(const char* format, ...) {
    if (!g_initialized) {
        LogConfig default_config;
        log_init(default_config);
    }
    
    if (LOG_DEBUG > g_config.level) return;
    
    char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    log_to_stdout(LOG_DEBUG, message);
    log_to_file(LOG_DEBUG, message);
}

void log_trace(const char* format, ...) {
    if (!g_initialized) {
        LogConfig default_config;
        log_init(default_config);
    }
    
    if (LOG_TRACE > g_config.level) return;
    
    char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    log_to_stdout(LOG_TRACE, message);
    log_to_file(LOG_TRACE, message);
}

void log_header(const char* text) {
    if (!g_initialized) {
        LogConfig default_config;
        log_init(default_config);
    }
    
    if (g_config.driver == DRIVER_FILE) {
        // For file, just write the header text
        if (g_config.log_file_handle) {
            fprintf(g_config.log_file_handle, "=== %s ===\n", text);
            fflush(g_config.log_file_handle);
        }
        return;
    }
    
    int width = get_terminal_width();
    int text_len = strlen(text);
    
    // Build header line with bounds checking
    char header[512];
    const size_t header_size = sizeof(header);
    size_t pos = 0;
    
    // Helper macro to safely append to header buffer
    #define SAFE_APPEND(str) do { \
        size_t len = strlen(str); \
        if (pos + len < header_size - 1) { \
            strcpy(header + pos, str); \
            pos += len; \
        } \
    } while(0)
    
    #define SAFE_APPEND_CHAR(c) do { \
        if (pos < header_size - 1) { \
            header[pos++] = c; \
        } \
    } while(0)
    
    // Background color
    if (g_config.use_color) {
        SAFE_APPEND(color::BG_LIGHT_GRAY);
        SAFE_APPEND(color::BLACK);
        SAFE_APPEND(color::BOLD);
    }
    
    // Calculate padding, ensuring we don't exceed buffer
    int padding = (width - text_len - 4) / 2; // 4 for "=  ="
    if (padding < 0) padding = 0;
    
    // Clamp width to prevent buffer overflow
    // Account for: padding + "= " + text + " =" + padding + ANSI codes (max ~50) + null
    int max_safe_width = header_size - 100; // Conservative limit
    if (width > max_safe_width) {
        width = max_safe_width;
        padding = (width - text_len - 4) / 2;
        if (padding < 0) padding = 0;
    }
    
    // Left padding
    for (int i = 0; i < padding && pos < header_size - 1; i++) {
        SAFE_APPEND_CHAR(' ');
    }
    
    // Text with borders
    SAFE_APPEND_CHAR('=');
    SAFE_APPEND_CHAR(' ');
    SAFE_APPEND(text);
    SAFE_APPEND_CHAR(' ');
    SAFE_APPEND_CHAR('=');
    
    // Right padding to fill width
    int current_len = padding + text_len + 4;
    for (int i = current_len; i < width && pos < header_size - 1; i++) {
        SAFE_APPEND_CHAR(' ');
    }
    
    // Reset color
    if (g_config.use_color) {
        SAFE_APPEND(color::RESET);
    }
    
    header[pos] = '\0';
    
    #undef SAFE_APPEND
    #undef SAFE_APPEND_CHAR
    
    fprintf(stdout, "%s\n", header);
    fflush(stdout);
}

} // namespace log
} // namespace tool
} // namespace federated

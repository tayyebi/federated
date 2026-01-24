#include "federated/tool/cli.h"
#include "federated/tool/log.h"
#include <cstring>

// Forward declaration
namespace federated {
namespace tool {
namespace cli {
void init_cli();
}
}
}

int main(int argc, char** argv) {
    using namespace federated::tool;
    
    // Initialize CLI commands and services
    cli::init_cli();
    
    // Initialize logging with defaults
    log::LogConfig log_config;
    log_config.level = log::LOG_INFO;
    log_config.use_color = true;
    log_config.driver = log::DRIVER_STDOUT;
    
    // Check for global log options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-color") == 0) {
            log_config.use_color = false;
        } else if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            const char* level = argv[i + 1];
            if (strcmp(level, "error") == 0) log_config.level = log::LOG_ERROR;
            else if (strcmp(level, "warn") == 0) log_config.level = log::LOG_WARN;
            else if (strcmp(level, "info") == 0) log_config.level = log::LOG_INFO;
            else if (strcmp(level, "debug") == 0) log_config.level = log::LOG_DEBUG;
            else if (strcmp(level, "trace") == 0) log_config.level = log::LOG_TRACE;
        } else if (strcmp(argv[i], "--log-file") == 0 && i + 1 < argc) {
            log_config.log_file_path = argv[i + 1];
            log_config.driver = log::DRIVER_BOTH;
        } else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            log_config.log_file_path = argv[i + 1];
            log_config.driver = log::DRIVER_BOTH;
        }
    }
    
    // Initialize logger
    log::log_init(log_config);
    
    // Run CLI
    int result = cli::cli_main(argc, argv);
    
    // Cleanup
    log::log_cleanup();
    
    return result;
}

#ifndef FEDERATED_TOOL_CLI_H
#define FEDERATED_TOOL_CLI_H

#include "../core/error.h"
#include <cstddef>

namespace federated {
namespace tool {
namespace cli {

// Maximum limits
static const size_t MAX_COMMANDS = 50;
static const size_t MAX_OPTIONS = 30;
static const size_t MAX_SERVICES = 20;
static const size_t MAX_ENV_VARS = 100;

/**
 * Option types for CLI parameters
 */
enum OptionType {
    OPT_STRING,
    OPT_INT,
    OPT_BOOL,
    OPT_FLAG
};

/**
 * CLI Option definition - single source of truth
 */
struct Option {
    const char* name;           // Long name (e.g., "port")
    char        short_name;     // Short name (e.g., 'p')
    OptionType  type;
    const char* description;
    const char* default_value;
    const char* env_var;        // Environment variable name (e.g., "FEDERATED_PORT")
    bool        required;
    
    Option() : name(nullptr), short_name('\0'), type(OPT_STRING),
               description(nullptr), default_value(nullptr),
               env_var(nullptr), required(false) {}
};

/**
 * Command definition - single source of truth
 */
struct Command {
    const char* name;
    const char* description;
    const char* usage;
    Option      options[MAX_OPTIONS];
    size_t      option_count;
    
    // Command handler
    core::ErrorCode (*handler)(int argc, char** argv);
    
    Command() : name(nullptr), description(nullptr), usage(nullptr),
                option_count(0), handler(nullptr) {}
};

/**
 * Service definition
 */
struct ServiceDef {
    const char* name;
    const char* description;
    Option      options[MAX_OPTIONS];
    size_t      option_count;
    
    // Service lifecycle callbacks
    core::ErrorCode (*start)(const char* config_file);
    core::ErrorCode (*stop)();
    core::ErrorCode (*status)();
    
    ServiceDef() : name(nullptr), description(nullptr),
                   option_count(0), start(nullptr), stop(nullptr), status(nullptr) {}
};

/**
 * CLI Registry - central registry for all commands and services
 */
struct CliRegistry {
    Command     commands[MAX_COMMANDS];
    size_t      command_count;
    ServiceDef  services[MAX_SERVICES];
    size_t      service_count;
    
    CliRegistry() : command_count(0), service_count(0) {}
};

// Global CLI registry
CliRegistry* get_cli_registry();

// Register a command
core::ErrorCode register_command(const Command& cmd);

// Register a service
core::ErrorCode register_service(const ServiceDef& service);

// Parse command line arguments
core::ErrorCode parse_args(int argc, char** argv, const Command& cmd);

// Get option value (checks: CLI args > env file > env var > default)
const char* get_option_value(const char* name);

// Load environment file (.env format)
core::ErrorCode load_env_file(const char* filename);

// Generate help text for command
void generate_help(const Command& cmd, char* output, size_t output_size);

// Generate bash completion script
void generate_bash_completion(char* output, size_t output_size);

// Generate zsh completion script
void generate_zsh_completion(char* output, size_t output_size);

// Main CLI entry point
int cli_main(int argc, char** argv);

} // namespace cli
} // namespace tool
} // namespace federated

#endif // FEDERATED_TOOL_CLI_H

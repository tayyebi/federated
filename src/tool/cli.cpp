#include "federated/tool/cli.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace federated {
namespace tool {
namespace cli {

// Global state
static CliRegistry g_registry;
static char g_option_values[MAX_OPTIONS][256];
static char g_option_names[MAX_OPTIONS][64];
static size_t g_option_value_count = 0;

// Environment variables storage
static char g_env_keys[MAX_ENV_VARS][64];
static char g_env_values[MAX_ENV_VARS][256];
static size_t g_env_count = 0;

CliRegistry* get_cli_registry() {
    return &g_registry;
}

core::ErrorCode register_command(const Command& cmd) {
    if (g_registry.command_count >= MAX_COMMANDS) {
        return core::ERR_INTERNAL;
    }
    g_registry.commands[g_registry.command_count++] = cmd;
    return core::OK;
}

core::ErrorCode register_service(const ServiceDef& service) {
    if (g_registry.service_count >= MAX_SERVICES) {
        return core::ERR_INTERNAL;
    }
    g_registry.services[g_registry.service_count++] = service;
    return core::OK;
}

// Helper: Set option value
static void set_option_value(const char* name, const char* value) {
    for (size_t i = 0; i < g_option_value_count; i++) {
        if (strcmp(g_option_names[i], name) == 0) {
            strncpy(g_option_values[i], value, 255);
            g_option_values[i][255] = '\0';
            return;
        }
    }
    
    if (g_option_value_count < MAX_OPTIONS) {
        strncpy(g_option_names[g_option_value_count], name, 63);
        g_option_names[g_option_value_count][63] = '\0';
        strncpy(g_option_values[g_option_value_count], value, 255);
        g_option_values[g_option_value_count][255] = '\0';
        g_option_value_count++;
    }
}

// Helper: Get stored option value
static const char* get_stored_option(const char* name) {
    for (size_t i = 0; i < g_option_value_count; i++) {
        if (strcmp(g_option_names[i], name) == 0) {
            return g_option_values[i];
        }
    }
    return nullptr;
}

// Helper: Get environment variable
static const char* get_env_var(const char* name) {
    // First check our loaded env vars
    for (size_t i = 0; i < g_env_count; i++) {
        if (strcmp(g_env_keys[i], name) == 0) {
            return g_env_values[i];
        }
    }
    // Then check system env
    return getenv(name);
}

core::ErrorCode parse_args(int argc, char** argv, const Command& cmd) {
    // Reset option values
    g_option_value_count = 0;
    
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        
        // Long option: --name=value or --name value
        if (arg[0] == '-' && arg[1] == '-') {
            const char* name = arg + 2;
            const char* eq = strchr(name, '=');
            
            char opt_name[64];
            const char* value;
            
            if (eq) {
                size_t len = eq - name;
                if (len > 63) len = 63;
                strncpy(opt_name, name, len);
                opt_name[len] = '\0';
                value = eq + 1;
            } else {
                strncpy(opt_name, name, 63);
                opt_name[63] = '\0';
                
                // Check if this is a flag
                bool is_flag = false;
                for (size_t j = 0; j < cmd.option_count; j++) {
                    if (strcmp(cmd.options[j].name, opt_name) == 0 &&
                        cmd.options[j].type == OPT_FLAG) {
                        is_flag = true;
                        break;
                    }
                }
                
                if (is_flag) {
                    value = "true";
                } else {
                    if (i + 1 >= argc) {
                        fprintf(stderr, "Error: Option --%s requires a value\n", opt_name);
                        return core::ERR_FORMAT;
                    }
                    value = argv[++i];
                }
            }
            
            set_option_value(opt_name, value);
        }
        // Short option: -p value or -p=value
        else if (arg[0] == '-' && arg[1] != '-') {
            char short_opt = arg[1];
            const char* value;
            
            if (arg[2] == '=') {
                value = arg + 3;
            } else if (arg[2] == '\0') {
                // Check if this is a flag
                bool is_flag = false;
                char opt_name[64] = "";
                for (size_t j = 0; j < cmd.option_count; j++) {
                    if (cmd.options[j].short_name == short_opt) {
                        strcpy(opt_name, cmd.options[j].name);
                        if (cmd.options[j].type == OPT_FLAG) {
                            is_flag = true;
                        }
                        break;
                    }
                }
                
                if (is_flag) {
                    value = "true";
                } else {
                    if (i + 1 >= argc) {
                        fprintf(stderr, "Error: Option -%c requires a value\n", short_opt);
                        return core::ERR_FORMAT;
                    }
                    value = argv[++i];
                }
            } else {
                value = arg + 2;
            }
            
            // Find option name from short name
            for (size_t j = 0; j < cmd.option_count; j++) {
                if (cmd.options[j].short_name == short_opt) {
                    set_option_value(cmd.options[j].name, value);
                    break;
                }
            }
        }
    }
    
    return core::OK;
}

const char* get_option_value(const char* name) {
    // Priority: CLI args > env file > env var > default
    
    // 1. Check CLI arguments
    const char* value = get_stored_option(name);
    if (value) return value;
    
    // 2. Check if option has env_var mapping
    CliRegistry* reg = get_cli_registry();
    for (size_t i = 0; i < reg->command_count; i++) {
        for (size_t j = 0; j < reg->commands[i].option_count; j++) {
            const Option& opt = reg->commands[i].options[j];
            if (strcmp(opt.name, name) == 0 && opt.env_var) {
                value = get_env_var(opt.env_var);
                if (value) return value;
                // Only return default if non-null; otherwise continue searching
                if (opt.default_value) return opt.default_value;
            }
        }
    }
    
    // 3. Check services
    for (size_t i = 0; i < reg->service_count; i++) {
        for (size_t j = 0; j < reg->services[i].option_count; j++) {
            const Option& opt = reg->services[i].options[j];
            if (strcmp(opt.name, name) == 0 && opt.env_var) {
                value = get_env_var(opt.env_var);
                if (value) return value;
                // Only return default if non-null; otherwise continue searching
                if (opt.default_value) return opt.default_value;
            }
        }
    }
    
    return nullptr;
}

core::ErrorCode load_env_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        return core::ERR_IO;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // Parse KEY=VALUE
        char* eq = strchr(line, '=');
        if (!eq) continue;
        
        *eq = '\0';
        const char* key = line;
        const char* value = eq + 1;
        
        // Trim whitespace
        while (*key == ' ' || *key == '\t') key++;
        while (*value == ' ' || *value == '\t') value++;
        
        // Remove trailing newline
        char* newline = strchr((char*)value, '\n');
        if (newline) *newline = '\0';
        
        // Remove quotes if present
        if (value[0] == '"' || value[0] == '\'') {
            value++;
            size_t len = strlen(value);
            if (len > 0 && (value[len-1] == '"' || value[len-1] == '\'')) {
                ((char*)value)[len-1] = '\0';
            }
        }
        
        // Store in our env vars
        if (g_env_count < MAX_ENV_VARS) {
            size_t key_len = strlen(key);
            if (key_len > 63) key_len = 63;
            memcpy(g_env_keys[g_env_count], key, key_len);
            g_env_keys[g_env_count][key_len] = '\0';
            
            size_t val_len = strlen(value);
            if (val_len > 255) val_len = 255;
            memcpy(g_env_values[g_env_count], value, val_len);
            g_env_values[g_env_count][val_len] = '\0';
            
            g_env_count++;
        }
    }
    
    fclose(f);
    return core::OK;
}

void generate_help(const Command& cmd, char* output, size_t output_size) {
    char* p = output;
    size_t remaining = output_size;
    
    // Helper macro to safely write and update pointers
    #define SAFE_SNPRINTF(...) do { \
        int n = snprintf(p, remaining, __VA_ARGS__); \
        if (n > 0 && (size_t)n < remaining) { \
            p += n; \
            remaining -= n; \
        } else { \
            remaining = 0; \
        } \
    } while(0)
    
    SAFE_SNPRINTF("Usage: %s\n\n", cmd.usage ? cmd.usage : cmd.name);
    
    if (cmd.description) {
        SAFE_SNPRINTF("%s\n\n", cmd.description);
    }
    
    if (cmd.option_count > 0) {
        SAFE_SNPRINTF("Options:\n");
        
        for (size_t i = 0; i < cmd.option_count && remaining > 0; i++) {
            const Option& opt = cmd.options[i];
            
            if (opt.short_name) {
                SAFE_SNPRINTF("  -%c, --%-20s", opt.short_name, opt.name);
            } else {
                SAFE_SNPRINTF("      --%-20s", opt.name);
            }
            
            if (opt.description) {
                SAFE_SNPRINTF(" %s", opt.description);
            }
            
            if (opt.default_value) {
                SAFE_SNPRINTF(" (default: %s)", opt.default_value);
            }
            
            if (opt.env_var) {
                SAFE_SNPRINTF(" [env: %s]", opt.env_var);
            }
            
            if (opt.required) {
                SAFE_SNPRINTF(" [required]");
            }
            
            SAFE_SNPRINTF("\n");
        }
    }
    
    #undef SAFE_SNPRINTF
}

void generate_bash_completion(char* output, size_t output_size) {
    char* p = output;
    size_t remaining = output_size;
    
    #define SAFE_SNPRINTF(...) do { \
        int n = snprintf(p, remaining, __VA_ARGS__); \
        if (n > 0 && (size_t)n < remaining) { \
            p += n; \
            remaining -= n; \
        } else { \
            remaining = 0; \
        } \
    } while(0)
    
    SAFE_SNPRINTF("# Bash completion for federated\n");
    SAFE_SNPRINTF("_federated_completions() {\n");
    SAFE_SNPRINTF("    local cur=\"${COMP_WORDS[COMP_CWORD]}\"\n");
    SAFE_SNPRINTF("    local cmd=\"${COMP_WORDS[1]}\"\n");
    SAFE_SNPRINTF("    \n");
    SAFE_SNPRINTF("    if [ $COMP_CWORD -eq 1 ]; then\n");
    SAFE_SNPRINTF("        COMPREPLY=($(compgen -W \"");
    
    CliRegistry* reg = get_cli_registry();
    for (size_t i = 0; i < reg->command_count && remaining > 0; i++) {
        SAFE_SNPRINTF("%s ", reg->commands[i].name);
    }
    
    SAFE_SNPRINTF("\" -- \"$cur\"))\n");
    SAFE_SNPRINTF("    else\n");
    SAFE_SNPRINTF("        COMPREPLY=($(compgen -W \"");
    SAFE_SNPRINTF("--help --version --config --env-file ");
    SAFE_SNPRINTF("\" -- \"$cur\"))\n");
    SAFE_SNPRINTF("    fi\n");
    SAFE_SNPRINTF("}\n\n");
    SAFE_SNPRINTF("complete -F _federated_completions federated\n");
    
    #undef SAFE_SNPRINTF
}

void generate_zsh_completion(char* output, size_t output_size) {
    char* p = output;
    size_t remaining = output_size;
    
    #define SAFE_SNPRINTF(...) do { \
        int n = snprintf(p, remaining, __VA_ARGS__); \
        if (n > 0 && (size_t)n < remaining) { \
            p += n; \
            remaining -= n; \
        } else { \
            remaining = 0; \
        } \
    } while(0)
    
    SAFE_SNPRINTF("#compdef federated\n\n");
    SAFE_SNPRINTF("_federated() {\n");
    SAFE_SNPRINTF("    local -a commands\n");
    SAFE_SNPRINTF("    commands=(\n");
    
    CliRegistry* reg = get_cli_registry();
    for (size_t i = 0; i < reg->command_count && remaining > 0; i++) {
        const Command& cmd = reg->commands[i];
        SAFE_SNPRINTF("        '%s:%s'\n", 
                      cmd.name, 
                      cmd.description ? cmd.description : "");
    }
    
    SAFE_SNPRINTF("    )\n");
    SAFE_SNPRINTF("    _describe 'command' commands\n");
    SAFE_SNPRINTF("}\n\n");
    SAFE_SNPRINTF("_federated\n");
    
    #undef SAFE_SNPRINTF
}

int cli_main(int argc, char** argv) {
    if (argc < 2) {
        printf("Federated - Universal Resilient Communication Toolkit\n\n");
        printf("Usage: federated <command> [options]\n\n");
        printf("Commands:\n");
        
        CliRegistry* reg = get_cli_registry();
        for (size_t i = 0; i < reg->command_count; i++) {
            printf("  %-15s %s\n", 
                   reg->commands[i].name,
                   reg->commands[i].description ? reg->commands[i].description : "");
        }
        
        printf("\nUse 'federated <command> --help' for more information\n");
        return 1;
    }
    
    const char* cmd_name = argv[1];
    
    // Special commands
    if (strcmp(cmd_name, "completion") == 0) {
        if (argc > 2 && strcmp(argv[2], "bash") == 0) {
            char output[4096];
            generate_bash_completion(output, sizeof(output));
            printf("%s", output);
            return 0;
        } else if (argc > 2 && strcmp(argv[2], "zsh") == 0) {
            char output[4096];
            generate_zsh_completion(output, sizeof(output));
            printf("%s", output);
            return 0;
        } else {
            printf("Usage: federated completion <bash|zsh>\n");
            return 1;
        }
    }
    
    // Find and execute command
    CliRegistry* reg = get_cli_registry();
    for (size_t i = 0; i < reg->command_count; i++) {
        if (strcmp(reg->commands[i].name, cmd_name) == 0) {
            const Command& cmd = reg->commands[i];
            
            // Check for --help
            for (int j = 2; j < argc; j++) {
                if (strcmp(argv[j], "--help") == 0 || strcmp(argv[j], "-h") == 0) {
                    char help[4096];
                    generate_help(cmd, help, sizeof(help));
                    printf("%s", help);
                    return 0;
                }
            }
            
            // Parse arguments
            core::ErrorCode err = parse_args(argc - 1, argv + 1, cmd);
            if (err != core::OK) {
                return 1;
            }
            
            // Execute command
            if (cmd.handler) {
                err = cmd.handler(argc - 1, argv + 1);
                return err == core::OK ? 0 : 1;
            }
            
            return 0;
        }
    }
    
    fprintf(stderr, "Error: Unknown command '%s'\n", cmd_name);
    fprintf(stderr, "Run 'federated --help' for usage\n");
    return 1;
}

} // namespace cli
} // namespace tool
} // namespace federated

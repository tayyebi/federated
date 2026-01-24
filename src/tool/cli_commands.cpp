#include "federated/tool/cli.h"
#include "federated/tool/log.h"
#include "federated/service/smtp.h"
#include "federated/service/imap.h"
#include "federated/service/mail.h"
#include <cstdio>
#include <cstring>

namespace federated {
namespace tool {
namespace cli {

// Service handlers
static core::ErrorCode smtp_service_start(const char* /* config_file */) {
    LOG_HEADER("Starting SMTP Service");
    
    const char* port = get_option_value("port");
    const char* host = get_option_value("host");
    
    LOG_INFO("Bind address: %s", host ? host : "0.0.0.0");
    LOG_INFO("Port: %s", port ? port : "25");
    
    // Initialize mail store
    service::MailStore* store = service::get_mail_store();
    core::ErrorCode err = store->init();
    if (err != core::OK) {
        LOG_ERROR("Failed to initialize mail store");
        return err;
    }
    
    LOG_INFO("SMTP service started successfully");
    return core::OK;
}

static core::ErrorCode smtp_service_stop() {
    LOG_INFO("Stopping SMTP service...");
    service::MailStore* store = service::get_mail_store();
    store->cleanup();
    LOG_INFO("SMTP service stopped");
    return core::OK;
}

static core::ErrorCode smtp_service_status() {
    LOG_INFO("SMTP service status: running");
    return core::OK;
}

static core::ErrorCode imap_service_start(const char* /* config_file */) {
    LOG_HEADER("Starting IMAP Service");
    
    const char* port = get_option_value("port");
    const char* host = get_option_value("host");
    
    LOG_INFO("Bind address: %s", host ? host : "0.0.0.0");
    LOG_INFO("Port: %s", port ? port : "143");
    
    // Initialize mail store
    service::MailStore* store = service::get_mail_store();
    core::ErrorCode err = store->init();
    if (err != core::OK) {
        LOG_ERROR("Failed to initialize mail store");
        return err;
    }
    
    LOG_INFO("IMAP service started successfully");
    return core::OK;
}

static core::ErrorCode imap_service_stop() {
    LOG_INFO("Stopping IMAP service...");
    service::MailStore* store = service::get_mail_store();
    store->cleanup();
    LOG_INFO("IMAP service stopped");
    return core::OK;
}

static core::ErrorCode imap_service_status() {
    LOG_INFO("IMAP service status: running");
    return core::OK;
}

// Command: service
static core::ErrorCode cmd_service_handler(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: federated service <start|stop|status|list> [service-name] [options]\n");
        return core::ERR_FORMAT;
    }
    
    const char* action = argv[1];
    
    if (strcmp(action, "list") == 0) {
        printf("Available services:\n");
        CliRegistry* reg = get_cli_registry();
        for (size_t i = 0; i < reg->service_count; i++) {
            const ServiceDef& svc = reg->services[i];
            printf("  %-15s %s\n", svc.name, svc.description ? svc.description : "");
        }
        return core::OK;
    }
    
    if (argc < 3) {
        fprintf(stderr, "Error: Service name required\n");
        return core::ERR_FORMAT;
    }
    
    const char* service_name = argv[2];
    
    // Load env file if specified
    const char* env_file = get_option_value("env-file");
    if (env_file) {
        core::ErrorCode err = load_env_file(env_file);
        if (err != core::OK) {
            fprintf(stderr, "Warning: Failed to load env file: %s\n", env_file);
        }
    }
    
    // Find service
    CliRegistry* reg = get_cli_registry();
    for (size_t i = 0; i < reg->service_count; i++) {
        if (strcmp(reg->services[i].name, service_name) == 0) {
            const ServiceDef& svc = reg->services[i];
            
            if (strcmp(action, "start") == 0) {
                const char* config = get_option_value("config");
                return svc.start ? svc.start(config) : core::OK;
            } else if (strcmp(action, "stop") == 0) {
                return svc.stop ? svc.stop() : core::OK;
            } else if (strcmp(action, "status") == 0) {
                return svc.status ? svc.status() : core::OK;
            } else {
                fprintf(stderr, "Error: Unknown action '%s'\n", action);
                return core::ERR_FORMAT;
            }
        }
    }
    
    fprintf(stderr, "Error: Service '%s' not found\n", service_name);
    return core::ERR_FORMAT;
}

// Command: version
static core::ErrorCode cmd_version_handler(int /* argc */, char** /* argv */) {
    printf("Federated v0.1.0\n");
    printf("Universal Resilient Communication Toolkit\n");
    return core::OK;
}

// Command: about
static core::ErrorCode cmd_about_handler(int /* argc */, char** /* argv */) {
    printf("Federated - Universal Resilient Communication Toolkit\n");
    printf("Version: 0.1.0\n\n");
    
    printf("Description:\n");
    printf("  A universal, resilient communication toolkit designed to operate\n");
    printf("  across any available medium - digital, analog, or human-assisted -\n");
    printf("  while remaining auditable, extensible, and survivable.\n\n");
    
    printf("Authors:\n");
    printf("  - GitHub Copilot (@copilot)\n");
    printf("  - Mostafa Tayyebi (@tayyebi)\n\n");
    
    printf("License:\n");
    printf("  MIT License\n\n");
    
    printf("Repository:\n");
    printf("  https://github.com/tayyebi/federated\n\n");
    
    printf("Documentation:\n");
    printf("  See README.md and docs/ directory for detailed information\n");
    
    return core::OK;
}

// Command: help
static core::ErrorCode cmd_help_handler(int /* argc */, char** /* argv */) {
    printf("Federated - Universal Resilient Communication Toolkit\n\n");
    printf("Usage: federated <command> [options]\n\n");
    printf("Commands:\n");
    
    CliRegistry* reg = get_cli_registry();
    for (size_t i = 0; i < reg->command_count; i++) {
        printf("  %-15s %s\n", 
               reg->commands[i].name,
               reg->commands[i].description ? reg->commands[i].description : "");
    }
    
    printf("\nEnvironment:\n");
    printf("  Configuration priority: CLI args > env file > env vars > defaults\n");
    printf("  Use --env-file <file> to load configuration from file\n");
    printf("  Use FEDERATED_* environment variables for configuration\n");
    
    printf("\nExamples:\n");
    printf("  federated service start smtp --port 2525\n");
    printf("  federated service start imap --port 1143 --env-file mail.env\n");
    printf("  FEDERATED_PORT=8080 federated service start smtp\n");
    printf("  federated completion bash > /etc/bash_completion.d/federated\n");
    
    return core::OK;
}

// Initialize CLI with commands and services
void init_cli() {
    // Register service command
    Command service_cmd;
    service_cmd.name = "service";
    service_cmd.description = "Manage services (start, stop, status, list)";
    service_cmd.usage = "federated service <action> [service-name] [options]";
    service_cmd.handler = cmd_service_handler;
    
    // Add common service options
    service_cmd.options[0].name = "host";
    service_cmd.options[0].short_name = 'h';
    service_cmd.options[0].type = OPT_STRING;
    service_cmd.options[0].description = "Bind host";
    service_cmd.options[0].default_value = "0.0.0.0";
    service_cmd.options[0].env_var = "FEDERATED_HOST";
    
    service_cmd.options[1].name = "port";
    service_cmd.options[1].short_name = 'p';
    service_cmd.options[1].type = OPT_INT;
    service_cmd.options[1].description = "Port number";
    service_cmd.options[1].env_var = "FEDERATED_PORT";
    
    service_cmd.options[2].name = "config";
    service_cmd.options[2].short_name = 'c';
    service_cmd.options[2].type = OPT_STRING;
    service_cmd.options[2].description = "Configuration file";
    service_cmd.options[2].env_var = "FEDERATED_CONFIG";
    
    service_cmd.options[3].name = "env-file";
    service_cmd.options[3].short_name = 'e';
    service_cmd.options[3].type = OPT_STRING;
    service_cmd.options[3].description = "Environment file (.env format)";
    
    service_cmd.options[4].name = "log-level";
    service_cmd.options[4].short_name = 'l';
    service_cmd.options[4].type = OPT_STRING;
    service_cmd.options[4].description = "Log level (error, warn, info, debug, trace)";
    service_cmd.options[4].default_value = "info";
    service_cmd.options[4].env_var = "FEDERATED_LOG_LEVEL";
    
    service_cmd.options[5].name = "log-file";
    service_cmd.options[5].short_name = 'L';
    service_cmd.options[5].type = OPT_STRING;
    service_cmd.options[5].description = "Log file path";
    service_cmd.options[5].env_var = "FEDERATED_LOG_FILE";
    
    service_cmd.options[6].name = "no-color";
    service_cmd.options[6].short_name = '\0';
    service_cmd.options[6].type = OPT_FLAG;
    service_cmd.options[6].description = "Disable colored output";
    
    service_cmd.option_count = 7;
    register_command(service_cmd);
    
    // Register version command
    Command version_cmd;
    version_cmd.name = "version";
    version_cmd.description = "Show version information";
    version_cmd.usage = "federated version";
    version_cmd.handler = cmd_version_handler;
    version_cmd.option_count = 0;
    register_command(version_cmd);
    
    // Register about command
    Command about_cmd;
    about_cmd.name = "about";
    about_cmd.description = "Show project information and authors";
    about_cmd.usage = "federated about";
    about_cmd.handler = cmd_about_handler;
    about_cmd.option_count = 0;
    register_command(about_cmd);
    
    // Register help command
    Command help_cmd;
    help_cmd.name = "help";
    help_cmd.description = "Show help information";
    help_cmd.usage = "federated help";
    help_cmd.handler = cmd_help_handler;
    help_cmd.option_count = 0;
    register_command(help_cmd);
    
    // Register SMTP service
    ServiceDef smtp_svc;
    smtp_svc.name = "smtp";
    smtp_svc.description = "SMTP mail server (RFC 5321)";
    smtp_svc.start = smtp_service_start;
    smtp_svc.stop = smtp_service_stop;
    smtp_svc.status = smtp_service_status;
    
    smtp_svc.options[0].name = "port";
    smtp_svc.options[0].short_name = 'p';
    smtp_svc.options[0].type = OPT_INT;
    smtp_svc.options[0].description = "SMTP port";
    smtp_svc.options[0].default_value = "25";
    smtp_svc.options[0].env_var = "FEDERATED_SMTP_PORT";
    
    smtp_svc.options[1].name = "host";
    smtp_svc.options[1].short_name = 'h';
    smtp_svc.options[1].type = OPT_STRING;
    smtp_svc.options[1].description = "Bind address";
    smtp_svc.options[1].default_value = "0.0.0.0";
    smtp_svc.options[1].env_var = "FEDERATED_SMTP_HOST";
    
    smtp_svc.option_count = 2;
    register_service(smtp_svc);
    
    // Register IMAP service
    ServiceDef imap_svc;
    imap_svc.name = "imap";
    imap_svc.description = "IMAP mail server (RFC 3501)";
    imap_svc.start = imap_service_start;
    imap_svc.stop = imap_service_stop;
    imap_svc.status = imap_service_status;
    
    imap_svc.options[0].name = "port";
    imap_svc.options[0].short_name = 'p';
    imap_svc.options[0].type = OPT_INT;
    imap_svc.options[0].description = "IMAP port";
    imap_svc.options[0].default_value = "143";
    imap_svc.options[0].env_var = "FEDERATED_IMAP_PORT";
    
    imap_svc.options[1].name = "host";
    imap_svc.options[1].short_name = 'h';
    imap_svc.options[1].type = OPT_STRING;
    imap_svc.options[1].description = "Bind address";
    imap_svc.options[1].default_value = "0.0.0.0";
    imap_svc.options[1].env_var = "FEDERATED_IMAP_HOST";
    
    imap_svc.option_count = 2;
    register_service(imap_svc);
}

} // namespace cli
} // namespace tool
} // namespace federated

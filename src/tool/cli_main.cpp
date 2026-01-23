#include "federated/tool/cli.h"

// Forward declaration
namespace federated {
namespace tool {
namespace cli {
void init_cli();
}
}
}

int main(int argc, char** argv) {
    // Initialize CLI commands and services
    federated::tool::cli::init_cli();
    
    // Run CLI
    return federated::tool::cli::cli_main(argc, argv);
}

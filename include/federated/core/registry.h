#ifndef FEDERATED_CORE_REGISTRY_H
#define FEDERATED_CORE_REGISTRY_H

namespace federated {

// Forward declarations
namespace transport { struct Transport; }
namespace framer { struct Framer; }
namespace crypto { struct Crypto; }
namespace service { struct Service; }
namespace tool { struct Tool; }

namespace core {

/**
 * Registry - The only global mutable state
 * 
 * Holds registered transports, framers, crypto modules, services, tools.
 * Everything is discovered explicitly.
 */
constexpr int MAX_TRANSPORTS = 32;
constexpr int MAX_FRAMERS = 16;
constexpr int MAX_CRYPTOS = 16;
constexpr int MAX_SERVICES = 32;
constexpr int MAX_TOOLS = 64;

struct Registry {
    transport::Transport* transports[MAX_TRANSPORTS];
    framer::Framer*       framers[MAX_FRAMERS];
    crypto::Crypto*       cryptos[MAX_CRYPTOS];
    service::Service*     services[MAX_SERVICES];
    tool::Tool*           tools[MAX_TOOLS];

    int transport_count;
    int framer_count;
    int crypto_count;
    int service_count;
    int tool_count;
    
    Registry();
    
    // Register components
    bool register_transport(transport::Transport* t);
    bool register_framer(framer::Framer* f);
    bool register_crypto(crypto::Crypto* c);
    bool register_service(service::Service* s);
    bool register_tool(tool::Tool* t);
    
    // Find components by name
    transport::Transport* find_transport(const char* name);
    framer::Framer*       find_framer(const char* name);
    crypto::Crypto*       find_crypto(const char* name);
    service::Service*     find_service(const char* name);
    tool::Tool*           find_tool(const char* name);
};

// Global registry instance
extern Registry global_registry;

} // namespace core
} // namespace federated

#endif // FEDERATED_CORE_REGISTRY_H

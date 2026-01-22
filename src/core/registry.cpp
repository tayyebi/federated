#include "../../include/federated/core/registry.h"
#include "../../include/federated/transport/transport.h"
#include "../../include/federated/framer/framer.h"
#include "../../include/federated/crypto/crypto.h"
#include "../../include/federated/service/service.h"
#include "../../include/federated/tool/tool.h"
#include <cstring>

namespace federated {
namespace core {

Registry global_registry;

Registry::Registry() 
    : transport_count(0), 
      framer_count(0), 
      crypto_count(0), 
      service_count(0), 
      tool_count(0) {
    // Initialize arrays to nullptr
    for (int i = 0; i < MAX_TRANSPORTS; i++) transports[i] = nullptr;
    for (int i = 0; i < MAX_FRAMERS; i++) framers[i] = nullptr;
    for (int i = 0; i < MAX_CRYPTOS; i++) cryptos[i] = nullptr;
    for (int i = 0; i < MAX_SERVICES; i++) services[i] = nullptr;
    for (int i = 0; i < MAX_TOOLS; i++) tools[i] = nullptr;
}

bool Registry::register_transport(transport::Transport* t) {
    if (transport_count >= MAX_TRANSPORTS) return false;
    transports[transport_count++] = t;
    return true;
}

bool Registry::register_framer(framer::Framer* f) {
    if (framer_count >= MAX_FRAMERS) return false;
    framers[framer_count++] = f;
    return true;
}

bool Registry::register_crypto(crypto::Crypto* c) {
    if (crypto_count >= MAX_CRYPTOS) return false;
    cryptos[crypto_count++] = c;
    return true;
}

bool Registry::register_service(service::Service* s) {
    if (service_count >= MAX_SERVICES) return false;
    services[service_count++] = s;
    return true;
}

bool Registry::register_tool(tool::Tool* t) {
    if (tool_count >= MAX_TOOLS) return false;
    tools[tool_count++] = t;
    return true;
}

transport::Transport* Registry::find_transport(const char* name) {
    for (int i = 0; i < transport_count; i++) {
        if (strcmp(transports[i]->name, name) == 0) {
            return transports[i];
        }
    }
    return nullptr;
}

framer::Framer* Registry::find_framer(const char* name) {
    for (int i = 0; i < framer_count; i++) {
        if (strcmp(framers[i]->name, name) == 0) {
            return framers[i];
        }
    }
    return nullptr;
}

crypto::Crypto* Registry::find_crypto(const char* name) {
    for (int i = 0; i < crypto_count; i++) {
        if (strcmp(cryptos[i]->name, name) == 0) {
            return cryptos[i];
        }
    }
    return nullptr;
}

service::Service* Registry::find_service(const char* name) {
    for (int i = 0; i < service_count; i++) {
        if (strcmp(services[i]->name, name) == 0) {
            return services[i];
        }
    }
    return nullptr;
}

tool::Tool* Registry::find_tool(const char* name) {
    for (int i = 0; i < tool_count; i++) {
        if (strcmp(tools[i]->name, name) == 0) {
            return tools[i];
        }
    }
    return nullptr;
}

} // namespace core
} // namespace federated

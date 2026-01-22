#ifndef FEDERATED_SERVICE_SERVICE_H
#define FEDERATED_SERVICE_SERVICE_H

#include "../core/buffer.h"
#include "../core/error.h"

namespace federated {
namespace service {

/**
 * Service - Application service interface
 * 
 * Services include: HTTP server, DNS cache, SMTP/IMAP, microblogging, file exchange
 * External peers do not need to be Federated nodes (web, email, DNS servers supported).
 */
struct Service {
    const char* name;
    
    // Handle service request
    core::ErrorCode (*handle_request)(const core::Buffer& input, core::Buffer& output);
};

} // namespace service
} // namespace federated

#endif // FEDERATED_SERVICE_SERVICE_H

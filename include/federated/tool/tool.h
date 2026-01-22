#ifndef FEDERATED_TOOL_TOOL_H
#define FEDERATED_TOOL_TOOL_H

#include "../core/buffer.h"
#include "../core/error.h"

namespace federated {
namespace tool {

/**
 * Tool - Diagnostic, monitoring, and utility tool interface
 * 
 * Built-in diagnostics: ping, arp, netstat, ifconfig, route, onion inspection
 * Feed aggregation: RSS 2.0, JSON Feed
 * Agent-less monitoring: SSH-based read-only commands
 * CLI / Alias: Scriptable, deterministic interface
 */
struct Tool {
    const char* name;
    
    // Run tool with input, produce output
    core::ErrorCode (*run)(const core::Buffer& input, core::Buffer& output);
};

} // namespace tool
} // namespace federated

#endif // FEDERATED_TOOL_TOOL_H

#ifndef FEDERATED_TRANSPORT_FILE_H
#define FEDERATED_TRANSPORT_FILE_H

#include "transport.h"

namespace federated {
namespace transport {

/**
 * File transport - read/write to files
 * 
 * Can be used for:
 * - Store and forward messaging
 * - Dead drops
 * - Shared filesystem communication
 */
struct FileTransport {
    static Transport* get_instance();
    static void set_read_path(const char* path);
    static void set_write_path(const char* path);
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_FILE_H

#ifndef FEDERATED_CORE_ERROR_H
#define FEDERATED_CORE_ERROR_H

namespace federated {
namespace core {

/**
 * ErrorCode - Explicit error codes, no exceptions
 */
enum ErrorCode {
    OK = 0,
    ERR_IO,
    ERR_FORMAT,
    ERR_TIMEOUT,
    ERR_CRYPTO,
    ERR_UNSUPPORTED,
    ERR_AUTH,
    ERR_INTERNAL
};

/**
 * Get human-readable error message
 */
const char* error_message(ErrorCode code);

} // namespace core
} // namespace federated

#endif // FEDERATED_CORE_ERROR_H

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
    ERR_INTERNAL,
    ERR_INVALID_ARG,
    ERR_OUT_OF_MEMORY,
    ERR_NOT_FOUND,
    ERR_NOT_INITIALIZED
};

/**
 * Get human-readable error message
 */
const char* error_message(ErrorCode code);

} // namespace core
} // namespace federated

#endif // FEDERATED_CORE_ERROR_H

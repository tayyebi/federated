#include "../../include/federated/core/error.h"

namespace federated {
namespace core {

const char* error_message(ErrorCode code) {
    switch (code) {
        case OK:                    return "Success";
        case ERR_IO:                return "I/O error";
        case ERR_FORMAT:            return "Format error";
        case ERR_TIMEOUT:           return "Timeout";
        case ERR_CRYPTO:            return "Cryptography error";
        case ERR_UNSUPPORTED:       return "Unsupported operation";
        case ERR_AUTH:              return "Authentication error";
        case ERR_INTERNAL:          return "Internal error";
        case ERR_INVALID_ARG:       return "Invalid argument";
        case ERR_OUT_OF_MEMORY:     return "Out of memory";
        case ERR_NOT_FOUND:         return "Not found";
        case ERR_NOT_INITIALIZED:   return "Not initialized";
        default:                    return "Unknown error";
    }
}

} // namespace core
} // namespace federated

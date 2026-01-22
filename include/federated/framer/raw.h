#ifndef FEDERATED_FRAMER_RAW_H
#define FEDERATED_FRAMER_RAW_H

#include "framer.h"

namespace federated {
namespace framer {

/**
 * Raw framer - no framing, pass-through
 * 
 * Useful for transports that handle framing themselves
 * or when no framing is needed.
 */
struct RawFramer {
    static Framer* get_instance();
};

} // namespace framer
} // namespace federated

#endif // FEDERATED_FRAMER_RAW_H

#ifndef FEDERATED_TRANSPORT_INFRARED_H
#define FEDERATED_TRANSPORT_INFRARED_H

#include "transport.h"

namespace federated {
namespace transport {

/**
 * Infrared (IrDA) Transport
 * 
 * IrDA Serial Infrared Specification
 * https://www.irda.org/
 * 
 * Provides line-of-sight infrared communication.
 * Typically used for device-to-device transfer.
 * 
 * Implementation:
 * - Serial port access for IrDA device
 * - Send/receive via serial protocol
 * - Low bandwidth (~115kbps typical)
 * - Very short range (< 1 meter)
 * 
 * Current Status: STUB IMPLEMENTATION
 * - TODO: Serial port communication for IrDA
 * - TODO: IrDA protocol framing
 * - TODO: Platform-specific serial port access
 */
struct InfraredTransport {
    static Transport* get_instance();
};

} // namespace transport
} // namespace federated

#endif // FEDERATED_TRANSPORT_INFRARED_H

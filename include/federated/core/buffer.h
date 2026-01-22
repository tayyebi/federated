#ifndef FEDERATED_CORE_BUFFER_H
#define FEDERATED_CORE_BUFFER_H

#include <cstddef>
#include <cstdint>

namespace federated {
namespace core {

/**
 * Buffer - Simple memory buffer with explicit ownership
 * 
 * No hidden allocation. Caller owns memory.
 */
struct Buffer {
    uint8_t* data;
    size_t   size;
    
    Buffer() : data(nullptr), size(0) {}
    Buffer(uint8_t* d, size_t s) : data(d), size(s) {}
};

} // namespace core
} // namespace federated

#endif // FEDERATED_CORE_BUFFER_H

# Platform Abstraction Layer Design

**Purpose:** Design guide for implementing cross-platform transport layers  
**Target Platforms:** Linux, macOS, Windows  
**Last Updated:** 2026-01-27

---

## Overview

The platform abstraction layer enables transport implementations (Bluetooth, WiFi Direct, Infrared) to work across different operating systems while maintaining the project's zero external dependency principle. This document defines the architecture, patterns, and guidelines for platform-specific code.

---

## Design Principles

### 1. Compile-Time Platform Detection
Use preprocessor macros for platform detection and conditional compilation:

```cpp
// Platform detection (standard macros)
#if defined(__linux__)
    #define FEDERATED_PLATFORM_LINUX
#elif defined(_WIN32) || defined(_WIN64)
    #define FEDERATED_PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define FEDERATED_PLATFORM_MACOS
#else
    #define FEDERATED_PLATFORM_UNKNOWN
#endif
```

### 2. Graceful Degradation
Unsupported platforms should compile successfully with stub implementations:

```cpp
#if defined(FEDERATED_PLATFORM_LINUX)
    // Full implementation
#elif defined(FEDERATED_PLATFORM_WINDOWS)
    // Full implementation
#else
    // Stub that returns ERR_UNSUPPORTED
#endif
```

### 3. Separate Compilation Units
Platform-specific implementations should be in separate files when complexity warrants:

```
src/transport/bluetooth.cpp           # Platform-agnostic interface
src/transport/bluetooth_linux.cpp     # Linux-specific implementation
src/transport/bluetooth_windows.cpp   # Windows-specific implementation
src/transport/bluetooth_macos.cpp     # macOS stub
```

### 4. Minimal Abstraction Overhead
- Avoid complex abstraction layers
- Use `#ifdef` guards within single files for simple cases
- Use function pointers for platform-specific operations
- No virtual functions or runtime polymorphism

---

## Platform-Specific APIs

### Linux Bluetooth (BlueZ)

**API Access:** System calls via socket API (no external libraries)

**Required Headers:**
```cpp
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>  // Kernel headers, not library
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
```

**Key Operations:**
- **HCI Socket:** Device management, scanning
  ```cpp
  int hci_socket = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  ```
  
- **RFCOMM Socket:** Data transfer
  ```cpp
  int rfcomm_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  ```

- **Address Structure:**
  ```cpp
  struct sockaddr_rc {
      sa_family_t rc_family;   // AF_BLUETOOTH
      bdaddr_t    rc_bdaddr;   // Bluetooth MAC address
      uint8_t     rc_channel;  // RFCOMM channel (1-30)
  };
  ```

**Example Implementation Pattern:**
```cpp
#if defined(FEDERATED_PLATFORM_LINUX)

static int g_bluetooth_socket = -1;

static core::ErrorCode bluetooth_open_linux() {
    g_bluetooth_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (g_bluetooth_socket < 0) {
        return core::ERR_IO;
    }
    
    struct sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    str2ba("XX:XX:XX:XX:XX:XX", &addr.rc_bdaddr);  // Parse from config
    addr.rc_channel = 1;
    
    if (connect(g_bluetooth_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(g_bluetooth_socket);
        g_bluetooth_socket = -1;
        return core::ERR_IO;
    }
    
    return core::OK;
}

#endif // FEDERATED_PLATFORM_LINUX
```

---

### Windows Bluetooth (Winsock Extensions)

**API Access:** Winsock2 with Bluetooth extensions

**Required Headers:**
```cpp
#include <winsock2.h>
#include <ws2bth.h>
#include <bluetoothapis.h>  // For device discovery
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bthprops.lib")
```

**Key Operations:**
- **Winsock Initialization:**
  ```cpp
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  ```

- **RFCOMM Socket:**
  ```cpp
  SOCKET rfcomm_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
  ```

- **Address Structure:**
  ```cpp
  SOCKADDR_BTH addr = {0};
  addr.addressFamily = AF_BTH;
  addr.btAddr = 0x123456789ABC;  // Bluetooth address as uint64
  addr.port = 1;  // RFCOMM channel
  ```

**Example Implementation Pattern:**
```cpp
#if defined(FEDERATED_PLATFORM_WINDOWS)

static SOCKET g_bluetooth_socket = INVALID_SOCKET;
static bool g_winsock_initialized = false;

static core::ErrorCode bluetooth_open_windows() {
    if (!g_winsock_initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return core::ERR_IO;
        }
        g_winsock_initialized = true;
    }
    
    g_bluetooth_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (g_bluetooth_socket == INVALID_SOCKET) {
        return core::ERR_IO;
    }
    
    SOCKADDR_BTH addr = {0};
    addr.addressFamily = AF_BTH;
    addr.btAddr = 0x123456789ABC;  // Parse from config
    addr.port = 1;
    
    if (connect(g_bluetooth_socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(g_bluetooth_socket);
        g_bluetooth_socket = INVALID_SOCKET;
        return core::ERR_IO;
    }
    
    return core::OK;
}

static core::ErrorCode bluetooth_close_windows() {
    if (g_bluetooth_socket != INVALID_SOCKET) {
        closesocket(g_bluetooth_socket);
        g_bluetooth_socket = INVALID_SOCKET;
    }
    return core::OK;
}

#endif // FEDERATED_PLATFORM_WINDOWS
```

---

### Linux WiFi Direct (wpa_supplicant)

**API Access:** D-Bus interface to wpa_supplicant daemon

**Required Headers:**
```cpp
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/wireless.h>
```

**D-Bus Communication:**
- Service: `fi.w1.wpa_supplicant1`
- Interface: `fi.w1.wpa_supplicant1.Interface.P2PDevice`
- Methods: `Find`, `Connect`, `Disconnect`

**Alternative: Control Interface:**
```cpp
// Direct socket to wpa_supplicant control interface
// /var/run/wpa_supplicant/p2p-dev-wlan0
struct sockaddr_un addr;
int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
// Send commands like "P2P_FIND", "P2P_CONNECT"
```

**Example Implementation Pattern:**
```cpp
#if defined(FEDERATED_PLATFORM_LINUX)

static int g_wpa_ctrl_socket = -1;

static core::ErrorCode wifi_direct_open_linux() {
    // Connect to wpa_supplicant control interface
    g_wpa_ctrl_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (g_wpa_ctrl_socket < 0) {
        return core::ERR_IO;
    }
    
    struct sockaddr_un local;
    local.sun_family = AF_UNIX;
    snprintf(local.sun_path, sizeof(local.sun_path), 
             "/tmp/wpa_ctrl_%d", getpid());
    
    if (bind(g_wpa_ctrl_socket, (struct sockaddr*)&local, sizeof(local)) < 0) {
        close(g_wpa_ctrl_socket);
        return core::ERR_IO;
    }
    
    // Send P2P_FIND command
    const char* cmd = "P2P_FIND";
    send(g_wpa_ctrl_socket, cmd, strlen(cmd), 0);
    
    return core::OK;
}

#endif // FEDERATED_PLATFORM_LINUX
```

---

### Windows WiFi Direct API

**API Access:** Windows Runtime (WinRT) WiFi Direct API

**Required Headers:**
```cpp
#include <windows.h>
#include <wrl/client.h>
#include <windows.devices.wifidirect.h>
```

**Note:** Requires C++/WinRT or COM, may conflict with zero-dependency goal.

**Alternative:** Use Winsock with manual WiFi Direct setup via netsh commands.

**Example Stub:**
```cpp
#if defined(FEDERATED_PLATFORM_WINDOWS)

static core::ErrorCode wifi_direct_open_windows() {
    // TODO: Windows WiFi Direct implementation
    // Options:
    // 1. Use WinRT API (requires COM, may conflict with zero-dep)
    // 2. Use netsh commands via system() (less elegant)
    // 3. Defer to future phase
    return core::ERR_UNSUPPORTED;
}

#endif // FEDERATED_PLATFORM_WINDOWS
```

---

### Serial Port Access (for Infrared)

**Linux (POSIX):**
```cpp
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

int open_serial_linux(const char* device) {
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;
    
    struct termios tty;
    tcgetattr(fd, &tty);
    
    cfsetospeed(&tty, B115200);  // IrDA typical speed
    cfsetispeed(&tty, B115200);
    
    tty.c_cflag |= (CLOCAL | CREAD);  // Enable receiver
    tty.c_cflag &= ~PARENB;  // No parity
    tty.c_cflag &= ~CSTOPB;  // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;      // 8 data bits
    
    tcsetattr(fd, TCSANOW, &tty);
    return fd;
}
```

**Windows:**
```cpp
#include <windows.h>

HANDLE open_serial_windows(const char* port) {
    HANDLE hSerial = CreateFile(port, GENERIC_READ | GENERIC_WRITE,
                                0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;
    
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    SetCommState(hSerial, &dcbSerialParams);
    return hSerial;
}
```

---

## Implementation Patterns

### Pattern 1: Single File with `#ifdef` Guards

**Use When:** Simple implementation, minimal code duplication

```cpp
// src/transport/bluetooth.cpp

#include "../../include/federated/transport/bluetooth.h"
#include "../../include/federated/core/error.h"

// Platform-specific headers
#if defined(FEDERATED_PLATFORM_LINUX)
    #include <bluetooth/bluetooth.h>
    #include <bluetooth/rfcomm.h>
#elif defined(FEDERATED_PLATFORM_WINDOWS)
    #include <winsock2.h>
    #include <ws2bth.h>
#endif

namespace federated {
namespace transport {

// Platform-specific global state
#if defined(FEDERATED_PLATFORM_LINUX)
    static int g_bt_socket = -1;
#elif defined(FEDERATED_PLATFORM_WINDOWS)
    static SOCKET g_bt_socket = INVALID_SOCKET;
#endif

// Open implementation
static core::ErrorCode bluetooth_open() {
#if defined(FEDERATED_PLATFORM_LINUX)
    // Linux implementation
    g_bt_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    return (g_bt_socket >= 0) ? core::OK : core::ERR_IO;
    
#elif defined(FEDERATED_PLATFORM_WINDOWS)
    // Windows implementation
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    g_bt_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    return (g_bt_socket != INVALID_SOCKET) ? core::OK : core::ERR_IO;
    
#else
    // Unsupported platform
    return core::ERR_UNSUPPORTED;
#endif
}

// Close implementation
static core::ErrorCode bluetooth_close() {
#if defined(FEDERATED_PLATFORM_LINUX)
    if (g_bt_socket >= 0) {
        close(g_bt_socket);
        g_bt_socket = -1;
    }
    return core::OK;
    
#elif defined(FEDERATED_PLATFORM_WINDOWS)
    if (g_bt_socket != INVALID_SOCKET) {
        closesocket(g_bt_socket);
        g_bt_socket = INVALID_SOCKET;
    }
    return core::OK;
    
#else
    return core::ERR_UNSUPPORTED;
#endif
}

// Transport structure
static Transport g_bluetooth_transport = {
    "bluetooth",
    bluetooth_open,
    bluetooth_close,
    bluetooth_send,
    bluetooth_recv,
    bluetooth_available
};

Transport* BluetoothTransport::get_instance() {
    return &g_bluetooth_transport;
}

} // namespace transport
} // namespace federated
```

---

### Pattern 2: Separate Files per Platform

**Use When:** Complex implementation, significant code per platform

**File Structure:**
```
src/transport/
    bluetooth.cpp           # Common interface and dispatch
    bluetooth_linux.cpp     # Linux implementation
    bluetooth_windows.cpp   # Windows implementation
    bluetooth_macos.cpp     # macOS stub
```

**CMakeLists.txt:**
```cmake
# Platform-specific source selection
if(UNIX AND NOT APPLE)
    set(BLUETOOTH_IMPL src/transport/bluetooth_linux.cpp)
elseif(WIN32)
    set(BLUETOOTH_IMPL src/transport/bluetooth_windows.cpp)
elseif(APPLE)
    set(BLUETOOTH_IMPL src/transport/bluetooth_macos.cpp)
endif()

add_library(transport
    src/transport/bluetooth.cpp
    ${BLUETOOTH_IMPL}
    # ... other sources
)
```

**bluetooth.cpp (dispatcher):**
```cpp
#include "../../include/federated/transport/bluetooth.h"

namespace federated {
namespace transport {

// Forward declarations (implemented in platform files)
extern core::ErrorCode bluetooth_open_impl();
extern core::ErrorCode bluetooth_close_impl();
extern core::ErrorCode bluetooth_send_impl(const core::Buffer& data);
extern core::ErrorCode bluetooth_recv_impl(core::Buffer& data);
extern bool bluetooth_available_impl();

// Wrapper functions
static core::ErrorCode bluetooth_open() {
    return bluetooth_open_impl();
}

static core::ErrorCode bluetooth_close() {
    return bluetooth_close_impl();
}

// ... other wrappers

static Transport g_bluetooth_transport = {
    "bluetooth",
    bluetooth_open,
    bluetooth_close,
    bluetooth_send,
    bluetooth_recv,
    bluetooth_available
};

Transport* BluetoothTransport::get_instance() {
    return &g_bluetooth_transport;
}

} // namespace transport
} // namespace federated
```

**bluetooth_linux.cpp:**
```cpp
#include "../../include/federated/core/error.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

namespace federated {
namespace transport {

static int g_bt_socket = -1;

core::ErrorCode bluetooth_open_impl() {
    g_bt_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    return (g_bt_socket >= 0) ? core::OK : core::ERR_IO;
}

// ... other implementations

} // namespace transport
} // namespace federated
```

---

## Testing Strategy

### Platform-Specific Tests

**Use Conditional Compilation:**
```cpp
// tests/transport/test_bluetooth.cpp

#include "../test_runner.h"
#include "../../include/federated/transport/bluetooth.h"

using namespace federated::transport;
using namespace federated::core;

#if defined(FEDERATED_PLATFORM_LINUX) || defined(FEDERATED_PLATFORM_WINDOWS)

// Full tests for supported platforms
TEST(bluetooth_open_close) {
    Transport* bt = BluetoothTransport::get_instance();
    
    // Note: May fail if no Bluetooth adapter present
    ErrorCode err = bt->open();
    if (err == OK) {
        TEST_ASSERT_EQ(bt->close(), OK);
    } else {
        // Acceptable failure if no hardware
        TEST_ASSERT_EQ(err, ERR_IO);
    }
}

TEST(bluetooth_send_recv) {
    // Requires paired device for full test
    // May be marked as manual test
}

#else

// Stub tests for unsupported platforms
TEST(bluetooth_unsupported_platform) {
    Transport* bt = BluetoothTransport::get_instance();
    TEST_ASSERT_EQ(bt->open(), ERR_UNSUPPORTED);
}

#endif
```

### Mock Implementations for Unit Tests

For tests that don't require real hardware:

```cpp
#ifdef FEDERATED_TEST_MODE
    // Mock Bluetooth socket
    static bool g_mock_bt_connected = false;
    
    core::ErrorCode bluetooth_open_impl() {
        g_mock_bt_connected = true;
        return core::OK;
    }
    
    core::ErrorCode bluetooth_send_impl(const core::Buffer& data) {
        return g_mock_bt_connected ? core::OK : core::ERR_IO;
    }
#endif
```

---

## CMake Integration

### Platform Detection

```cmake
# Detect platform
if(UNIX AND NOT APPLE)
    set(FEDERATED_PLATFORM_LINUX TRUE)
    message(STATUS "Platform: Linux")
elseif(WIN32)
    set(FEDERATED_PLATFORM_WINDOWS TRUE)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    set(FEDERATED_PLATFORM_MACOS TRUE)
    message(STATUS "Platform: macOS")
else()
    message(WARNING "Platform: Unknown")
endif()
```

### Conditional Source Inclusion

```cmake
set(TRANSPORT_SOURCES
    src/transport/loopback.cpp
    src/transport/file.cpp
    src/transport/tcp.cpp
    src/transport/udp.cpp
    src/transport/dns_tunnel.cpp
)

# Add platform-specific sources
if(FEDERATED_PLATFORM_LINUX)
    list(APPEND TRANSPORT_SOURCES
        src/transport/bluetooth_linux.cpp
        src/transport/wifi_direct_linux.cpp
    )
elseif(FEDERATED_PLATFORM_WINDOWS)
    list(APPEND TRANSPORT_SOURCES
        src/transport/bluetooth_windows.cpp
    )
endif()

# Always include base implementation
list(APPEND TRANSPORT_SOURCES src/transport/bluetooth.cpp)
```

### Platform-Specific Libraries

```cmake
if(FEDERATED_PLATFORM_WINDOWS)
    target_link_libraries(federated ws2_32 bthprops)
endif()

# Note: Linux Bluetooth uses kernel headers only, no linking needed
```

---

## Error Handling

### Platform-Specific Errors

Map platform errors to `core::ErrorCode`:

```cpp
core::ErrorCode map_platform_error(int platform_errno) {
#if defined(FEDERATED_PLATFORM_LINUX)
    switch (errno) {
        case ECONNREFUSED:  return core::ERR_IO;
        case ENODEV:        return core::ERR_UNSUPPORTED;
        case EACCES:        return core::ERR_IO;
        default:            return core::ERR_INTERNAL;
    }
#elif defined(FEDERATED_PLATFORM_WINDOWS)
    int err = WSAGetLastError();
    switch (err) {
        case WSAECONNREFUSED: return core::ERR_IO;
        case WSAENOTSOCK:     return core::ERR_INTERNAL;
        default:              return core::ERR_INTERNAL;
    }
#else
    return core::ERR_UNSUPPORTED;
#endif
}
```

---

## Documentation Requirements

### Header Comments

```cpp
/**
 * Bluetooth Transport (Platform-Dependent)
 * 
 * Platform Support:
 * - Linux: BlueZ stack via RFCOMM sockets
 * - Windows: Winsock2 Bluetooth extensions
 * - macOS: Not implemented (returns ERR_UNSUPPORTED)
 * 
 * Requirements:
 * - Linux: Bluetooth adapter, BlueZ installed
 * - Windows: Bluetooth adapter, Winsock2
 * 
 * Limitations:
 * - Requires device pairing before connection
 * - May require elevated privileges on some systems
 * - Throughput typically 100-300 KB/s (Bluetooth 2.x)
 */
```

### README.md Platform Requirements

Update README to document platform requirements:

```markdown
## Platform Requirements

### Bluetooth Transport
- **Linux:** BlueZ stack (kernel 2.4.6+), Bluetooth adapter
- **Windows:** Windows 7+ with Bluetooth support
- **macOS:** Not currently supported

### WiFi Direct Transport
- **Linux:** wpa_supplicant 2.0+, WiFi adapter with P2P support
- **Windows:** Windows 8+ with WiFi Direct capable adapter
- **macOS:** Not currently supported
```

---

## Best Practices

1. **Always Provide Stub Implementations**
   - Unsupported platforms should compile successfully
   - Return `ERR_UNSUPPORTED` from all operations
   - Document platform limitations clearly

2. **Minimize Platform-Specific Code**
   - Keep common logic in platform-agnostic files
   - Only platform-specific syscalls/APIs in `#ifdef` blocks
   - Share buffer management, error handling, etc.

3. **Test on All Platforms**
   - CI/CD should build on Linux, Windows, macOS
   - Mark platform-specific tests appropriately
   - Document manual testing requirements

4. **Document Platform Differences**
   - Note API differences in comments
   - Explain platform-specific behaviors
   - Provide examples for each platform

5. **Follow Zero-Dependency Principle**
   - Use only system headers and kernel interfaces
   - No external libraries (even if "header-only")
   - Prefer syscalls over high-level APIs when possible

---

## Example: Complete Bluetooth Implementation Outline

See `docs/PHASE4_ROADMAP.md` Task 2.1 for full Bluetooth implementation plan.

**Summary:**
1. Create platform detection headers
2. Implement Linux version (BlueZ RFCOMM)
3. Implement Windows version (Winsock2 Bluetooth)
4. Create macOS stub (ERR_UNSUPPORTED)
5. Add platform-specific tests
6. Document platform requirements
7. Update CMakeLists.txt for conditional compilation

---

## Conclusion

The platform abstraction layer enables cross-platform transport implementations while adhering to the project's core principles:
- Zero external dependencies
- Explicit error handling
- Flat architecture
- Test-first development

By following these patterns and guidelines, platform-specific transports can be implemented efficiently and maintainably.

---

**Next Steps:**
1. Review this design with the team
2. Implement first platform-specific transport (Bluetooth on Linux)
3. Validate patterns work in practice
4. Refine based on lessons learned

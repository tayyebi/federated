#include "../../include/federated/transport/file.h"
#include <cstdio>
#include <cstring>

namespace federated {
namespace transport {

static const char* g_read_path = nullptr;
static const char* g_write_path = nullptr;
static bool g_file_open = false;

// Implementation functions
static core::ErrorCode file_open() {
    if (!g_read_path || !g_write_path) {
        return core::ERR_IO;
    }
    g_file_open = true;
    return core::OK;
}

static core::ErrorCode file_close() {
    g_file_open = false;
    return core::OK;
}

static core::ErrorCode file_send(const core::Buffer& data) {
    if (!g_file_open || !g_write_path) {
        return core::ERR_IO;
    }
    
    FILE* f = fopen(g_write_path, "wb");
    if (!f) {
        return core::ERR_IO;
    }
    
    size_t written = fwrite(data.data, 1, data.size, f);
    fclose(f);
    
    if (written != data.size) {
        return core::ERR_IO;
    }
    
    return core::OK;
}

static core::ErrorCode file_recv(core::Buffer& data) {
    if (!g_file_open || !g_read_path) {
        return core::ERR_IO;
    }
    
    FILE* f = fopen(g_read_path, "rb");
    if (!f) {
        return core::ERR_IO;
    }
    
    size_t read = fread(data.data, 1, data.size, f);
    fclose(f);
    
    if (read == 0) {
        return core::ERR_IO;
    }
    
    data.size = read;
    return core::OK;
}

static bool file_available() {
    return g_file_open;
}

// Static transport instance
static Transport g_file_transport = {
    "file",
    file_open,
    file_close,
    file_send,
    file_recv,
    file_available
};

Transport* FileTransport::get_instance() {
    return &g_file_transport;
}

void FileTransport::set_read_path(const char* path) {
    g_read_path = path;
}

void FileTransport::set_write_path(const char* path) {
    g_write_path = path;
}

} // namespace transport
} // namespace federated

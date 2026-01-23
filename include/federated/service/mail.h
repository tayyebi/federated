#ifndef FEDERATED_SERVICE_MAIL_H
#define FEDERATED_SERVICE_MAIL_H

#include "../core/buffer.h"
#include "../core/error.h"
#include <cstdint>
#include <cstring>

namespace federated {
namespace service {

/**
 * MailMessage - Email message structure
 */
struct MailMessage {
    char     id[64];            // locally generated, stable
    char     from[256];
    char     to[256];
    char     subject[256];
    uint64_t timestamp;
    
    core::Buffer headers;       // raw RFC 5322 headers
    core::Buffer body;          // raw body
    
    MailMessage() : timestamp(0) {
        id[0] = '\0';
        from[0] = '\0';
        to[0] = '\0';
        subject[0] = '\0';
        headers = core::Buffer();
        body = core::Buffer();
    }
};

/**
 * Mailbox - Email mailbox structure
 */
struct Mailbox {
    char name[64];              // INBOX, Sent, Outbox, Queue
    
    Mailbox() {
        name[0] = '\0';
    }
    
    explicit Mailbox(const char* n) {
        strncpy(name, n, 63);
        name[63] = '\0';
    }
};

/**
 * MailStore - Interface for mail storage
 */
struct MailStore {
    // Append message to mailbox
    core::ErrorCode (*append)(const char* mailbox, const MailMessage& msg);
    
    // List messages in mailbox
    core::ErrorCode (*list)(const char* mailbox, MailMessage* messages, size_t* count);
    
    // Fetch specific message
    core::ErrorCode (*fetch)(const char* mailbox, const char* msg_id, MailMessage& msg);
    
    // Delete message
    core::ErrorCode (*remove)(const char* mailbox, const char* msg_id);
    
    // Initialize store
    core::ErrorCode (*init)();
    
    // Cleanup
    core::ErrorCode (*cleanup)();
};

// Get mail store instance
MailStore* get_mail_store();

} // namespace service
} // namespace federated

#endif // FEDERATED_SERVICE_MAIL_H

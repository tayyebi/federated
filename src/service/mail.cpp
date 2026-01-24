/**
 * Mail Storage Implementation
 * 
 * RFC 5322 - Internet Message Format
 * https://datatracker.ietf.org/doc/html/rfc5322
 * 
 * Provides in-memory storage for email messages following RFC 5322 format.
 * Supports standard mailboxes as defined by IMAP (RFC 3501):
 * - INBOX: Incoming messages
 * - SENT: Sent messages
 * - OUTBOX: Messages pending send
 * - QUEUE: Store-and-forward queue
 * 
 * Message IDs: Generated using timestamp + counter (Section 3.6.4)
 */

#include "federated/service/mail.h"
#include <cstring>
#include <cstdio>
#include <ctime>

namespace federated {
namespace service {

// Simple in-memory mail store (for testing, will add file persistence later)
static const size_t MAX_MESSAGES = 1000;
static const size_t MAX_MAILBOXES = 4;

struct StoredMessage {
    MailMessage msg;
    char mailbox[64];
    bool deleted;
    
    StoredMessage() : deleted(false) {
        mailbox[0] = '\0';
    }
};

static StoredMessage messages[MAX_MESSAGES];
static size_t message_count = 0;
static bool initialized = false;

// Generate unique message ID
static void generate_message_id(char* id, size_t size) {
    static uint64_t counter = 0;
    uint64_t timestamp = static_cast<uint64_t>(time(nullptr));
    // Use %llu for uint64_t to ensure portability across platforms
    snprintf(id, size, "%llu-%llu", 
             (unsigned long long)timestamp, 
             (unsigned long long)counter++);
}

static core::ErrorCode mail_store_init() {
    if (initialized) {
        return core::OK;
    }
    
    message_count = 0;
    for (size_t i = 0; i < MAX_MESSAGES; i++) {
        messages[i].deleted = false;
        messages[i].mailbox[0] = '\0';
        messages[i].msg.id[0] = '\0';
    }
    
    initialized = true;
    return core::OK;
}

static core::ErrorCode mail_store_cleanup() {
    initialized = false;
    message_count = 0;
    return core::OK;
}

static core::ErrorCode mail_store_append(const char* mailbox, const MailMessage& msg) {
    if (!initialized) {
        return core::ERR_INTERNAL;
    }
    
    if (message_count >= MAX_MESSAGES) {
        return core::ERR_INTERNAL;
    }
    
    StoredMessage& stored = messages[message_count];
    stored.msg = msg;
    stored.deleted = false;
    strncpy(stored.mailbox, mailbox, 63);
    stored.mailbox[63] = '\0';
    
    // Generate ID if not set
    if (stored.msg.id[0] == '\0') {
        generate_message_id(stored.msg.id, 64);
    }
    
    message_count++;
    return core::OK;
}

static core::ErrorCode mail_store_list(const char* mailbox, MailMessage* msgs, size_t* count) {
    if (!initialized) {
        return core::ERR_INTERNAL;
    }
    
    size_t found = 0;
    for (size_t i = 0; i < message_count && found < *count; i++) {
        if (!messages[i].deleted && 
            strcmp(messages[i].mailbox, mailbox) == 0) {
            msgs[found++] = messages[i].msg;
        }
    }
    
    *count = found;
    return core::OK;
}

static core::ErrorCode mail_store_fetch(const char* mailbox, const char* msg_id, MailMessage& msg) {
    if (!initialized) {
        return core::ERR_INTERNAL;
    }
    
    for (size_t i = 0; i < message_count; i++) {
        if (!messages[i].deleted &&
            strcmp(messages[i].mailbox, mailbox) == 0 &&
            strcmp(messages[i].msg.id, msg_id) == 0) {
            msg = messages[i].msg;
            return core::OK;
        }
    }
    
    return core::ERR_IO;
}

static core::ErrorCode mail_store_remove(const char* mailbox, const char* msg_id) {
    if (!initialized) {
        return core::ERR_INTERNAL;
    }
    
    for (size_t i = 0; i < message_count; i++) {
        if (!messages[i].deleted &&
            strcmp(messages[i].mailbox, mailbox) == 0 &&
            strcmp(messages[i].msg.id, msg_id) == 0) {
            messages[i].deleted = true;
            return core::OK;
        }
    }
    
    return core::ERR_IO;
}

static MailStore store_instance = {
    mail_store_append,
    mail_store_list,
    mail_store_fetch,
    mail_store_remove,
    mail_store_init,
    mail_store_cleanup
};

MailStore* get_mail_store() {
    return &store_instance;
}

} // namespace service
} // namespace federated

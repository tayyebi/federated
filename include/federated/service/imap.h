#ifndef FEDERATED_SERVICE_IMAP_H
#define FEDERATED_SERVICE_IMAP_H

/**
 * IMAP (Internet Message Access Protocol) Implementation
 * 
 * RFC References:
 * - RFC 3501: Internet Message Access Protocol - Version 4rev1
 *   https://datatracker.ietf.org/doc/html/rfc3501
 *   Defines the protocol for accessing and managing email messages
 * 
 * This implementation provides:
 * - IMAP4rev1 server functionality
 * - State machine: NOT AUTHENTICATED → AUTHENTICATED → SELECTED → LOGOUT
 *   (Section 3 of RFC 3501)
 * - Core commands:
 *   - CAPABILITY: Advertise server capabilities (Section 6.1.1)
 *   - LOGIN: Authenticate user (Section 6.2.3)
 *   - LIST: List available mailboxes (Section 6.3.8)
 *   - SELECT: Select a mailbox (Section 6.3.1)
 *   - FETCH: Retrieve message data (Section 6.4.5)
 *   - LOGOUT: Close connection (Section 6.1.3)
 * - Mailbox management: INBOX, SENT, OUTBOX, QUEUE
 */

#include "../core/buffer.h"
#include "../core/error.h"
#include "mail.h"

namespace federated {
namespace service {

/**
 * IMAP State Machine
 */
enum ImapState {
    IMAP_NOT_AUTH,
    IMAP_AUTH,
    IMAP_SELECTED,
    IMAP_LOGOUT
};

/**
 * IMAP Session
 */
struct ImapSession {
    ImapState state;
    char      username[64];
    char      selected_mailbox[64];
    
    ImapSession() : state(IMAP_NOT_AUTH) {
        username[0] = '\0';
        selected_mailbox[0] = '\0';
    }
};

/**
 * IMAP Command
 */
struct ImapCommand {
    char tag[16];
    char name[16];
    char args[512];
};

// Parse IMAP command
bool parse_imap_command(const core::Buffer& line, ImapCommand& cmd);

// IMAP Server - Process command
core::ErrorCode imap_server_process_command(ImapSession& session,
                                            const ImapCommand& cmd,
                                            char* response,
                                            size_t response_size);

// IMAP Server - Handle LOGIN
core::ErrorCode imap_handle_login(ImapSession& session,
                                  const char* username,
                                  const char* password,
                                  const char* tag,
                                  char* response,
                                  size_t response_size);

// IMAP Server - Handle LIST
core::ErrorCode imap_handle_list(ImapSession& session,
                                 const char* tag,
                                 char* response,
                                 size_t response_size);

// IMAP Server - Handle SELECT
core::ErrorCode imap_handle_select(ImapSession& session,
                                   const char* mailbox,
                                   const char* tag,
                                   char* response,
                                   size_t response_size);

// IMAP Server - Handle FETCH
core::ErrorCode imap_handle_fetch(ImapSession& session,
                                  const char* sequence,
                                  const char* items,
                                  const char* tag,
                                  char* response,
                                  size_t response_size);

} // namespace service
} // namespace federated

#endif // FEDERATED_SERVICE_IMAP_H

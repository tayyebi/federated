#ifndef FEDERATED_SERVICE_SMTP_H
#define FEDERATED_SERVICE_SMTP_H

/**
 * SMTP (Simple Mail Transfer Protocol) Implementation
 * 
 * RFC References:
 * - RFC 5321: Simple Mail Transfer Protocol (SMTP)
 *   https://datatracker.ietf.org/doc/html/rfc5321
 *   Defines the protocol for email transmission between mail servers
 * 
 * - RFC 5322: Internet Message Format
 *   https://datatracker.ietf.org/doc/html/rfc5322
 *   Defines the format of email messages (headers, body)
 * 
 * This implementation provides:
 * - SMTP server: Receives mail from clients (Section 3 of RFC 5321)
 * - SMTP client: Sends mail to servers (Section 4 of RFC 5321)
 * - State machine validation (Section 4.1.4 of RFC 5321)
 * - Command processing: HELO, EHLO, MAIL FROM, RCPT TO, DATA, RSET, QUIT
 * - Store-and-forward for offline operation
 */

#include "../core/buffer.h"
#include "../core/error.h"
#include "mail.h"

namespace federated {
namespace service {

/**
 * SMTP State Machine
 */
enum SmtpState {
    SMTP_INIT,
    SMTP_GREETED,
    SMTP_MAIL,
    SMTP_RCPT,
    SMTP_DATA,
    SMTP_RECV_BODY,
    SMTP_DONE,
    SMTP_ERROR
};

/**
 * SMTP Session
 */
struct SmtpSession {
    SmtpState state;
    char      mail_from[256];
    char      rcpt_to[256];
    core::Buffer body_buffer;
    size_t    body_size;
    
    SmtpSession() : state(SMTP_INIT), body_size(0) {
        mail_from[0] = '\0';
        rcpt_to[0] = '\0';
        body_buffer = core::Buffer();
    }
};

/**
 * SMTP Command Parser
 */
struct SmtpCommand {
    char cmd[16];
    char arg[512];
};

// Parse SMTP command
bool parse_smtp_command(const core::Buffer& line, SmtpCommand& cmd);

// SMTP Server - Process command
core::ErrorCode smtp_server_process_command(SmtpSession& session, 
                                            const SmtpCommand& cmd,
                                            char* response,
                                            size_t response_size);

// SMTP Client - Send message
core::ErrorCode smtp_client_send(const char* server,
                                int port,
                                const MailMessage& msg,
                                bool* sent);

// SMTP Client - Queue message for later (offline)
core::ErrorCode smtp_client_queue(const MailMessage& msg);

} // namespace service
} // namespace federated

#endif // FEDERATED_SERVICE_SMTP_H

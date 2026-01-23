#include "federated/service/smtp.h"
#include "federated/service/mail.h"
#include <cstring>
#include <cstdio>

namespace federated {
namespace service {

// Parse SMTP command
bool parse_smtp_command(const core::Buffer& line, SmtpCommand& cmd) {
    if (line.size == 0 || line.data == nullptr) {
        return false;
    }
    
    size_t i = 0;
    size_t j = 0;
    
    // Parse command
    while (i < line.size && line.data[i] != ' ' && line.data[i] != '\r' && line.data[i] != '\n') {
        if (j < 15) {
            cmd.cmd[j++] = line.data[i];
        }
        i++;
    }
    cmd.cmd[j] = '\0';
    
    // Skip space
    if (i < line.size && line.data[i] == ' ') {
        i++;
    }
    
    // Parse argument
    j = 0;
    while (i < line.size && line.data[i] != '\r' && line.data[i] != '\n') {
        if (j < 511) {
            cmd.arg[j++] = line.data[i];
        }
        i++;
    }
    cmd.arg[j] = '\0';
    
    return true;
}

// Extract email from angle brackets <email@example.com>
static void extract_email(const char* input, char* output, size_t output_size) {
    const char* start = strchr(input, '<');
    const char* end = strchr(input, '>');
    
    if (start && end && end > start) {
        start++; // Skip '<'
        size_t len = end - start;
        if (len >= output_size) {
            len = output_size - 1;
        }
        strncpy(output, start, len);
        output[len] = '\0';
    } else {
        // No angle brackets, copy as is
        strncpy(output, input, output_size - 1);
        output[output_size - 1] = '\0';
    }
}

// Process SMTP command
core::ErrorCode smtp_server_process_command(SmtpSession& session,
                                            const SmtpCommand& cmd,
                                            char* response,
                                            size_t response_size) {
    // Compare commands (case insensitive)
    auto str_eq = [](const char* a, const char* b) -> bool {
        while (*a && *b) {
            char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
            char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    };
    
    if (str_eq(cmd.cmd, "HELO") || str_eq(cmd.cmd, "EHLO")) {
        if (session.state != SMTP_INIT) {
            snprintf(response, response_size, "503 Bad sequence of commands\r\n");
            return core::OK;
        }
        session.state = SMTP_GREETED;
        snprintf(response, response_size, "250 Hello\r\n");
        return core::OK;
    }
    
    if (str_eq(cmd.cmd, "MAIL")) {
        if (session.state != SMTP_GREETED && session.state != SMTP_DONE) {
            snprintf(response, response_size, "503 Bad sequence of commands\r\n");
            return core::OK;
        }
        extract_email(cmd.arg, session.mail_from, 256);
        session.state = SMTP_MAIL;
        snprintf(response, response_size, "250 OK\r\n");
        return core::OK;
    }
    
    if (str_eq(cmd.cmd, "RCPT")) {
        if (session.state != SMTP_MAIL && session.state != SMTP_RCPT) {
            snprintf(response, response_size, "503 Bad sequence of commands\r\n");
            return core::OK;
        }
        extract_email(cmd.arg, session.rcpt_to, 256);
        session.state = SMTP_RCPT;
        snprintf(response, response_size, "250 OK\r\n");
        return core::OK;
    }
    
    if (str_eq(cmd.cmd, "DATA")) {
        if (session.state != SMTP_RCPT) {
            snprintf(response, response_size, "503 Bad sequence of commands\r\n");
            return core::OK;
        }
        session.state = SMTP_DATA;
        snprintf(response, response_size, "354 Start mail input; end with <CRLF>.<CRLF>\r\n");
        return core::OK;
    }
    
    if (str_eq(cmd.cmd, "QUIT")) {
        session.state = SMTP_DONE;
        snprintf(response, response_size, "221 Bye\r\n");
        return core::OK;
    }
    
    if (str_eq(cmd.cmd, "RSET")) {
        session.state = SMTP_GREETED;
        session.mail_from[0] = '\0';
        session.rcpt_to[0] = '\0';
        snprintf(response, response_size, "250 OK\r\n");
        return core::OK;
    }
    
    snprintf(response, response_size, "500 Command not recognized\r\n");
    return core::OK;
}

// SMTP Client - Send message (stub for now)
core::ErrorCode smtp_client_send(const char* /* server */,
                                int /* port */,
                                const MailMessage& msg,
                                bool* sent) {
    // For now, just queue it
    *sent = false;
    return smtp_client_queue(msg);
}

// SMTP Client - Queue message for offline delivery
core::ErrorCode smtp_client_queue(const MailMessage& msg) {
    MailStore* store = get_mail_store();
    return store->append("OUTBOX", msg);
}

} // namespace service
} // namespace federated

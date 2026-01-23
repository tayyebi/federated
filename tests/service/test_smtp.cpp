#include "../test_runner.h"
#include "../../include/federated/service/smtp.h"
#include "../../include/federated/service/mail.h"
#include <cstring>

using namespace federated::service;
using namespace federated::core;

// Test: Parse SMTP command
TEST(smtp_parse_command) {
    uint8_t data[] = "HELO example.com\r\n";
    Buffer buf(data, sizeof(data) - 1);
    
    SmtpCommand cmd;
    bool result = parse_smtp_command(buf, cmd);
    
    TEST_ASSERT(result == true);
    TEST_ASSERT_STR_EQ(cmd.cmd, "HELO");
    TEST_ASSERT_STR_EQ(cmd.arg, "example.com");
}

// Test: SMTP server receive message - HELO
TEST(smtp_server_helo) {
    SmtpSession session;
    SmtpCommand cmd;
    strcpy(cmd.cmd, "HELO");
    strcpy(cmd.arg, "client.example.com");
    
    char response[256];
    ErrorCode err = smtp_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(session.state == SMTP_GREETED);
    TEST_ASSERT(strstr(response, "250") != nullptr);
}

// Test: SMTP server state machine - correct sequence
TEST(smtp_server_correct_sequence) {
    MailStore* store = get_mail_store();
    store->init();
    
    SmtpSession session;
    SmtpCommand cmd;
    char response[256];
    
    // HELO
    strcpy(cmd.cmd, "HELO");
    strcpy(cmd.arg, "client.example.com");
    smtp_server_process_command(session, cmd, response, sizeof(response));
    TEST_ASSERT(session.state == SMTP_GREETED);
    
    // MAIL FROM
    strcpy(cmd.cmd, "MAIL");
    strcpy(cmd.arg, "FROM:<sender@example.com>");
    smtp_server_process_command(session, cmd, response, sizeof(response));
    TEST_ASSERT(session.state == SMTP_MAIL);
    TEST_ASSERT(strstr(response, "250") != nullptr);
    
    // RCPT TO
    strcpy(cmd.cmd, "RCPT");
    strcpy(cmd.arg, "TO:<receiver@example.com>");
    smtp_server_process_command(session, cmd, response, sizeof(response));
    TEST_ASSERT(session.state == SMTP_RCPT);
    TEST_ASSERT(strstr(response, "250") != nullptr);
    
    // DATA
    strcpy(cmd.cmd, "DATA");
    strcpy(cmd.arg, "");
    smtp_server_process_command(session, cmd, response, sizeof(response));
    TEST_ASSERT(session.state == SMTP_DATA);
    TEST_ASSERT(strstr(response, "354") != nullptr);
    
    store->cleanup();
}

// Test: SMTP server state machine - wrong sequence
TEST(smtp_server_wrong_sequence) {
    SmtpSession session;
    SmtpCommand cmd;
    char response[256];
    
    // Try DATA before HELO (wrong sequence)
    strcpy(cmd.cmd, "DATA");
    strcpy(cmd.arg, "");
    smtp_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT(strstr(response, "503") != nullptr || strstr(response, "500") != nullptr);
}

// Test: SMTP client queue message when offline
TEST(smtp_client_store_when_offline) {
    MailStore* store = get_mail_store();
    store->init();
    
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Offline message");
    msg.timestamp = 1234567890;
    
    ErrorCode err = smtp_client_queue(msg);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify message is in OUTBOX
    MailMessage messages[10];
    size_t count = 10;
    store->list("OUTBOX", messages, &count);
    
    TEST_ASSERT_EQ(count, 1);
    TEST_ASSERT_STR_EQ(messages[0].from, "sender@example.com");
    
    store->cleanup();
}

// Test: SMTP client send (offline mode)
TEST(smtp_client_send_basic) {
    MailStore* store = get_mail_store();
    store->init();
    
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test message");
    msg.timestamp = 1234567890;
    
    bool sent = false;
    ErrorCode err = smtp_client_send("smtp.example.com", 25, msg, &sent);
    
    TEST_ASSERT_EQ(err, OK);
    // In offline mode, message goes to OUTBOX
    
    MailMessage messages[10];
    size_t count = 10;
    store->list("OUTBOX", messages, &count);
    TEST_ASSERT_EQ(count, 1);
    
    store->cleanup();
}

// Test: SMTP RSET command
TEST(smtp_server_rset) {
    SmtpSession session;
    session.state = SMTP_MAIL;
    strcpy(session.mail_from, "sender@example.com");
    
    SmtpCommand cmd;
    strcpy(cmd.cmd, "RSET");
    strcpy(cmd.arg, "");
    
    char response[256];
    ErrorCode err = smtp_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(session.state == SMTP_GREETED);
    TEST_ASSERT(session.mail_from[0] == '\0');
}

// Test: SMTP QUIT command
TEST(smtp_server_quit) {
    SmtpSession session;
    session.state = SMTP_GREETED;
    
    SmtpCommand cmd;
    strcpy(cmd.cmd, "QUIT");
    strcpy(cmd.arg, "");
    
    char response[256];
    ErrorCode err = smtp_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(session.state == SMTP_DONE);
    TEST_ASSERT(strstr(response, "221") != nullptr);
}

#include "../test_runner.h"
#include "../../include/federated/service/imap.h"
#include "../../include/federated/service/mail.h"
#include <cstring>

using namespace federated::service;
using namespace federated::core;

// Test: Parse IMAP command
TEST(imap_parse_command) {
    uint8_t data[] = "A001 CAPABILITY\r\n";
    Buffer buf(data, sizeof(data) - 1);
    
    ImapCommand cmd;
    bool result = parse_imap_command(buf, cmd);
    
    TEST_ASSERT(result == true);
    TEST_ASSERT_STR_EQ(cmd.tag, "A001");
    TEST_ASSERT_STR_EQ(cmd.name, "CAPABILITY");
}

// Test: IMAP login
TEST(imap_login_plain) {
    ImapSession session;
    char response[512];
    
    ErrorCode err = imap_handle_login(session, "testuser", "password", "A001", response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(session.state == IMAP_AUTH);
    TEST_ASSERT(strstr(response, "OK") != nullptr);
    TEST_ASSERT(strstr(response, "A001") != nullptr);
}

// Test: IMAP list mailboxes
TEST(imap_list_mailboxes) {
    ImapSession session;
    session.state = IMAP_AUTH;
    
    char response[1024];
    ErrorCode err = imap_handle_list(session, "A002", response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr(response, "INBOX") != nullptr);
    TEST_ASSERT(strstr(response, "SENT") != nullptr);
    TEST_ASSERT(strstr(response, "OUTBOX") != nullptr);
    TEST_ASSERT(strstr(response, "QUEUE") != nullptr);
    TEST_ASSERT(strstr(response, "A002") != nullptr);
}

// Test: IMAP select mailbox
TEST(imap_select_mailbox) {
    MailStore* store = get_mail_store();
    store->init();
    
    // Add a message to INBOX
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test");
    store->append("INBOX", msg);
    
    ImapSession session;
    session.state = IMAP_AUTH;
    
    char response[1024];
    ErrorCode err = imap_handle_select(session, "INBOX", "A003", response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(session.state == IMAP_SELECTED);
    TEST_ASSERT_STR_EQ(session.selected_mailbox, "INBOX");
    TEST_ASSERT(strstr(response, "EXISTS") != nullptr);
    TEST_ASSERT(strstr(response, "A003") != nullptr);
    
    store->cleanup();
}

// Test: IMAP fetch messages
TEST(imap_fetch_inbox) {
    MailStore* store = get_mail_store();
    store->init();
    
    // Add messages to INBOX
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test");
    uint8_t body_data[] = "This is the email body.";
    msg.body = Buffer(body_data, sizeof(body_data) - 1);
    store->append("INBOX", msg);
    
    ImapSession session;
    session.state = IMAP_SELECTED;
    strcpy(session.selected_mailbox, "INBOX");
    
    char response[2048];
    ErrorCode err = imap_handle_fetch(session, "1", "BODY[]", "A004", response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr(response, "FETCH") != nullptr);
    TEST_ASSERT(strstr(response, "A004") != nullptr);
    
    store->cleanup();
}

// Test: IMAP delete message (mark as deleted + expunge)
TEST(imap_delete_message) {
    MailStore* store = get_mail_store();
    store->init();
    
    // Add a message
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "To Delete");
    store->append("INBOX", msg);
    
    // Get message ID
    MailMessage messages[10];
    size_t count = 10;
    store->list("INBOX", messages, &count);
    TEST_ASSERT_EQ(count, 1);
    
    // Delete the message
    ErrorCode err = store->remove("INBOX", messages[0].id);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify deletion
    count = 10;
    store->list("INBOX", messages, &count);
    TEST_ASSERT_EQ(count, 0);
    
    store->cleanup();
}

// Test: IMAP capability
TEST(imap_capability) {
    ImapSession session;
    ImapCommand cmd;
    strcpy(cmd.tag, "A001");
    strcpy(cmd.name, "CAPABILITY");
    strcpy(cmd.args, "");
    
    char response[512];
    ErrorCode err = imap_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr(response, "IMAP4rev1") != nullptr);
}

// Test: IMAP logout
TEST(imap_logout) {
    ImapSession session;
    session.state = IMAP_AUTH;
    
    ImapCommand cmd;
    strcpy(cmd.tag, "A999");
    strcpy(cmd.name, "LOGOUT");
    strcpy(cmd.args, "");
    
    char response[512];
    ErrorCode err = imap_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(session.state == IMAP_LOGOUT);
    TEST_ASSERT(strstr(response, "BYE") != nullptr);
}

// Test: IMAP command without authentication
TEST(imap_unauthenticated_command) {
    ImapSession session;
    session.state = IMAP_NOT_AUTH;
    
    ImapCommand cmd;
    strcpy(cmd.tag, "A002");
    strcpy(cmd.name, "LIST");
    strcpy(cmd.args, "\"\" \"*\"");
    
    char response[512];
    ErrorCode err = imap_server_process_command(session, cmd, response, sizeof(response));
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT(strstr(response, "BAD") != nullptr || strstr(response, "NO") != nullptr);
}
